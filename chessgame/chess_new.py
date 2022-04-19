from collections import deque

# TODO
# separate is_legal_move into functions per piece (wording TBD)(done)
# turn legal_moves into possible_moves
# initialize move matrix
# en-passant - legal_moves
# castling - legal_moves
# promotion - legal_moves
# init_pawn_targets needs to be completed (done)
# backspacing

# legal move are the moves that are allowed to me made and take into account the state of the board
# target squares are all squares that are in the "line of sight of a given piece"

import math

import numpy as np
from pygame.locals import *
import pygame
import sys

# globle varabuls
# the board it tels us what is on that square
board = np.zeros(64, dtype=int)
# e.g. pieces[1] is a list of locations of all white pawns
pieces = np.full((13, 8), -1, dtype=int)
# e.g. list of sets. target_squares[n]={p0, p1, p2, ...} means square n is targeted by pieces on squares p0, p1, ...
target_squares = []
# e.g. list of sets. legal_moves[n]={p0, p1, p2, ...} means the piece on square n can move to p0, p1, ...
legal_moves = []
move_list = deque()
screen = None
piece_img = []

wight_king_move = 0
black_king_move = 0
black_a_rook = 0
black_h_rook = 0
wight_a_rook = 0
wight_h_rook = 0

knight_offset = [17, 15, -6, 10, -17, -15, 6, -10]
bishop_offset = [9, -7, -9, 7]
rook_offset = [8, 1, -8, -1]
queen_offset = rook_offset + bishop_offset
king_offset = queen_offset


# get what file you are on given an index 0-63
def get_file(n):
    return n % 8


# get what rank you are on given an index 0-63
def get_rank(n):
    return n // 8


def get_rank_start(n):
    return n * 8


def get_rank_end(n):
    return (n + 1) * 8


def rank_dif(s, f):
    d = get_rank(f) - get_rank(s)
    return d


def file_dif(s, f):
    d = get_file(f) - get_file(s)
    return d


# turns a tuple n=(x, y) into an index from 0-63
def coords_to_num(n):
    return n[1] * 8 + n[0]


def init_pawn_targets(square):
    piece = board[square]
    urdl = board_edge(square)
    n = square
    u, r, d, l = urdl
    color = 1  # 1 if white, -1 if black
    if piece == 7:
        color = -1

    target_squares[n + 8 * color].add(square)
    # empty square in front
    if board[n + 8 * color] == 0 and get_rank(n) == (6, 1)[color == 1] and board[n + 16 * color] == 0:
        target_squares[n + 16 * color].add(square)
    if color == 1:
        if l > 0:
            target_squares[n + 7].add(square)
        if r > 0:
            target_squares[n + 9].add(square)
        # this is the black
    else:
        if l > 0:
            target_squares[n - 9].add(square)
        if r > 0:
            target_squares[n - 7].add(square)


def init_knight_targets(square):
    piece = board[square]
    urdl = board_edge(square)
    for i in range(4):
        n = square
        if urdl[i] < 2:
            continue
        if urdl[(i + 1) % 4] > 0:
            target_squares[n + knight_offset[i * 2]].add(square)
        if urdl[(i + 3) % 4] > 0:
            target_squares[n + knight_offset[i * 2 + 1]].add(square)


def init_bishop_targets(square):
    urdl = board_edge(square)
    u, r, d, l = urdl
    diag = [min(u, r), min(r, d), min(d, l), min(l, u)]
    for i in range(4):
        n = square
        for j in range(diag[i]):
            n += bishop_offset[i]
            target_squares[n].add(square)
            if board[n] > 0:
                break


def init_rook_targets(square):
    urdl = board_edge(square)
    for i in range(4):
        n = square
        for j in range(urdl[i]):
            n += rook_offset[i]
            target_squares[n].add(square)
            if board[n] > 0:
                break


def init_queen_targets(square):
    urdl = board_edge(square)
    u, r, d, l = urdl
    diag = [min(u, r), min(r, d), min(d, l), min(l, u)]
    all_dir = urdl + diag
    for i in range(8):
        n = square
        for j in range(all_dir[i]):
            n += queen_offset[i]
            target_squares[n].add(square)
            if board[n] > 0:
                break


def init_king_targets(square):
    urdl = board_edge(square)
    u, r, d, l = urdl
    diag = [min(u, r), min(r, d), min(d, l), min(l, u)]
    all_dir = urdl + diag
    for i in range(8):
        n = square
        if all_dir[i]:
            n += king_offset[i]
            target_squares[n].add(square)
            if board[n] > 0:
                continue


def apply_initial_targets(square):
    p = board[square]  # piece type
    if p == 1 or p == 7:
        init_pawn_targets(square)
    if p == 2 or p == 8:
        init_knight_targets(square)
    elif p == 3 or p == 9:
        init_bishop_targets(square)
    elif p == 4 or p == 10:
        init_rook_targets(square)
    elif p == 5 or p == 11:
        init_queen_targets(square)
    elif p == 6 or p == 12:
        init_king_targets(square)


# fill target squares at the start of the game


def populate_target_squares():
    global target_squares
    target_squares = [set() for i in range(64)]
    for i in range(1, 13):
        for j in range(8):
            piece_square = pieces[i][j]
            if piece_square < 0:
                continue
            apply_initial_targets(piece_square)
    # for i in range(64):
    #     print(chess_notation(i), end=", ")
    #     for j in target_squares[i]:
    #         print(chess_notation(j), end=" ")
    #     print()


# narrows down target_squares to only include legal_moves
def narrow_legal_moves():
    for i in range(64):
        for j in target_squares[i]:
            # i is end square
            # j is start square
            if board[j] == 0:
                continue
            w_start = (1 <= board[j] <= 6)
            w_end = (1 <= board[i] <= 6)
            # if w_start == w_end:
            #     legal_moves[i].remove(j)


def populate_legal_moves():
    global legal_moves
    legal_moves = [set() for i in range(64)]
    # setting legal_moves equal to target_squares
    for i in range(64):
        for j in target_squares[i]:
            legal_moves[i].add(j)
    narrow_legal_moves()


# white: P = 1, N = 2, B = 3, R = 4, Q = 5, K = 6
# black: P = 7, N = 8, B = 9, R = 10, Q = 11, K = 12
def init_board():
    global piece_img
    global target_squares
    # this is the board section
    # wight powns
    board[get_rank_start(1):get_rank_end(1)] = 1

    # black pawns
    board[get_rank_start(6):get_rank_end(6)] = 7
    # this wight back rank

    w = np.array([4, 2, 3, 5, 6, 3, 2, 4])
    board[get_rank_start(0):get_rank_end(0)] = w

    board[get_rank_start(7):get_rank_end(7)] = w + 6

    piece_img = [None] * 13

    files = ['', 'Images/WhitePawn.png', 'Images/WhiteKnight.png', 'Images/WhiteBishop.png',
             'Images/WhiteRook.png', 'Images/WhiteQueen.png', 'Images/WhiteKing.png',

             'Images/BlackPawn.png', 'Images/BlackKnight.png', 'Images/BlackBishop.png',
             'Images/BlackRook.png', 'Images/BlackQueen.png', 'Images/BlackKing.png']
    for j in range(1, 13):
        piece_img[j] = pygame.image.load(files[j])
        piece_img[j] = pygame.transform.scale(piece_img[j], (50, 50))
    # this is where we start pieces
    a = np.array(list(range(8, 16)))
    pieces[1] = a
    pieces[7] = a + 40
    a = np.array([[1, 6], [2, 5], [0, 7], [3, -1],
                  [4, -1]])  # knight on 1 and 6, bishop on 2 and 5, rook on 0 and 7, queen on 3, king on 4
    pieces[2:7, 0:2] = a  # white pieces
    pieces[8:13, 0:2] = a + 56  # black pieces

    populate_target_squares()
    populate_legal_moves()


def chess_notation(square):
    file_letter = ["a", "b", "c", "d", "e", "f", "g", "h"]
    file = file_letter[get_file(square)]
    rank = get_rank(square) + 1

    return file + str(rank)


def run_game():
    global screen
    mainClock = pygame.time.Clock()
    pygame.init()
    pygame.display.set_caption('Chess')
    screen = pygame.display.set_mode((400, 400), 0, 32)
    clicking = False
    init_board()
    draw_board()
    while True:
        for event in pygame.event.get():
            if event.type == QUIT or (event.type == KEYDOWN and event.key == K_ESCAPE):
                pygame.quit()
                sys.exit()
            if event.type == MOUSEBUTTONDOWN:
                if event.button == BUTTON_LEFT and not clicking:
                    clicking = True
                    press = pygame.mouse.get_pos()
                    press = math.floor(press[0] / 50), math.ceil(7 - press[1] / 50)
            if event.type == MOUSEBUTTONUP:
                if event.button == BUTTON_LEFT and clicking:
                    clicking = False
                    release = pygame.mouse.get_pos()
                    release = math.floor(release[0] / 50), math.ceil(7 - release[1] / 50)
                    start = coords_to_num(press)
                    end = coords_to_num(release)

                    # a move hase been made
                    if is_legal_move(start, end):
                        move_list.append((start, end))
                        print("legal")
                        # this move was a capter
                        if board[end] > 0:
                            captured_piece = np.where(pieces[board[end]] == end)
                            pieces[board[end]][captured_piece] = -2
                        # this is the piece that you moved
                        moved_piece = np.where(pieces[board[start]] == start)

                        pieces[board[start]][moved_piece] = end

                        board[end] = board[start]
                        board[start] = 0

                        draw_board()
                        populate_target_squares()
                        populate_legal_moves()
                    else:
                        print("illegal")
        pygame.display.update()
        mainClock.tick(100)


# redraws the squares affected by a move
def redraw_squares(start, end):
    pass


def pawn_move(start, end):
    piece = board[start]
    diff = end - start
    urdl = board_edge(start)
    n = start
    u, r, d, l = urdl
    color = 1  # 1 if white, -1 if black
    if piece == 7:
        color = -1
    # empty square in front
    if board[n + 8 * color] == 0:
        if diff == 8 * color:
            return True
        # 2 empty squares in front
        if get_rank(n) == (6, 1)[color == 1] and board[n + 16 * color] == 0:
            if diff == 16 * color:
                return True

        # check if the last move was a doblel pawn push

    if color == 1:
        if l > 0 and diff == 7:
            return True
        if r > 0 and diff == 9:
            return True
    else:
        if l > 0 and diff == -9:
            return True
        if r > 0 and diff == -7:
            return True
    return False


def knight_move(start, end):
    piece = board[start]
    diff = end - start
    urdl = board_edge(start)
    for i in range(4):
        n = start
        if urdl[i] < 2:
            continue
        if urdl[(i + 1) % 4] > 0:
            if diff == knight_offset[i * 2]:
                return True
        if urdl[(i + 3) % 4] > 0:
            if diff == knight_offset[i * 2 + 1]:
                return True
        continue
    return False


def bishop_move(start, end):
    piece = board[start]
    diff = end - start
    urdl = board_edge(start)
    u, r, d, l = urdl
    diag = [min(u, r), min(r, d), min(d, l), min(l, u)]
    for i in range(4):
        n = start
        for j in range(diag[i]):
            n += bishop_offset[i]
            if n == end:
                return True
            if board[n] > 0:
                break
    return False


def rook_move(start, end):
    global wight_a_rook, wight_h_rook, black_a_rook, black_h_rook
    piece = board[start]
    diff = end - start
    urdl = board_edge(start)
    for i in range(4):
        n = start
        for j in range(urdl[i]):
            n += rook_offset[i]
            if n == end:
                if piece == 4:
                    if start == 0:
                        wight_a_rook += 1
                    elif start == 7:
                        wight_h_rook += 1
                elif piece == 10:
                    if start == 56:
                        black_a_rook += 1
                    elif start == 63:
                        black_h_rook += 1
                return True
            if board[n] > 0:
                break
    return False


def queen_move(start, end):
    piece = board[start]
    diff = end - start
    urdl = board_edge(start)
    u, r, d, l = urdl
    diag = [min(u, r), min(r, d), min(d, l), min(l, u)]
    all_dir = urdl + diag
    for i in range(8):
        n = start
        for j in range(all_dir[i]):
            n += queen_offset[i]
            if n == end:
                return True
            if board[n] > 0:
                break
    return False


def king_move(start, end):
    global wight_king_move
    global black_king_move
    piece = board[start]
    diff = end - start
    urdl = board_edge(start)
    u, r, d, l = urdl
    diag = [min(u, r), min(r, d), min(d, l), min(l, u)]
    all_dir = urdl + diag
    for i in range(8):
        n = start
        if all_dir[i]:
            n += king_offset[i]
            if n == end:
                if piece == 6:
                    wight_king_move += 1
                else:
                    black_king_move += 1
                return True
            if board[n] > 0:
                continue
    return False


# returns all legal moves for a piece on a given square
def get_legal_moves(square):
    piece = board[square]
    # this function shouldn't be called on an empty square
    if piece == 0:
        raise Exception
    # white pawns
    elif piece == 1:
        pass
    # king
    elif piece == 6 or piece == 12:
        pass


# start and end are both integers from 0-63, representing a square on the board
# returns true if this is a legal move
def is_legal_move(start, end):
    piece = board[start]
    if start == end:
        return False
    if piece == 0:
        return False
    if start in legal_moves[end]:
        return True
    # w_start = (1 <= piece <= 6)
    # w_end = (1 <= board[end] <= 6)
    # if w_start == w_end:
    #     return False
    # if piece == 1 or piece == 7:
    #     return pawn_move(start, end)
    # elif piece == 2 or piece == 8:
    #     return knight_move(start, end)
    # elif piece == 3 or piece == 9:
    #     return bishop_move(start, end)
    # elif piece == 4 or piece == 10:
    #     return rook_move(start, end)
    # elif piece == 5 or piece == 11:
    #     return queen_move(start, end)
    # elif piece == 6 or piece == 12:
    #     return king_move(start, end)
    return False

# this updats the dicshonarys
def update_charts():
    pass


def draw_board():
    BLUE = (18, 201, 192)
    WHITE = (249, 255, 212)
    RED = (255, 0, 0)
    GREEN = (25, 166, 0, 150)
    GREY = (150, 150, 150, 150)
    YELLOW = (255, 255, 0)

    for i in range(64):
        if (get_file(i) + get_rank(i)) % 2 == 0:
            square_color = BLUE
        else:
            square_color = WHITE
        pygame.draw.rect(screen, square_color, (get_file(i) * 50, 350 - get_rank(i) * 50, 50, 50))
        p = board[i]
        if p > 0:
            screen.blit(pygame.transform.rotate(piece_img[p], 0), (get_file(i) * 50, 350 - get_rank(i) * 50))


# this function will retern fore numbers start u,r,d,l (CW) the # of squars to the edg of the board
def board_edge(square):
    up = 7 - get_rank(square)
    right = 7 - get_file(square)
    down = 7 - up
    left = 7 - right
    return [up, right, down, left]


def board_attack_update(square_start, square_end):
    pass


run_game()
