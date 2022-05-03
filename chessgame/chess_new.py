from collections import deque

# TODO

# reversing bits (done)
# capturing piece movement (done)
# regular piece movement
# en-passant - legal_moves
# castling - legal_moves
# promotion - legal_moves (done)
# undo moves
# make sure everything is good to go
# run game two-player
# run game engine

import math

import numpy as np
from pygame.locals import *
import pygame
import sys

move_list = deque()
screen = None
piece_img = []
bitboards = []

w_threats = np.int64(0)
b_threats = np.int64(0)

not_black_pieces = np.int64(0)
white_pieces = np.int64(0)

not_white_pieces = np.int64(0)
black_pieces = np.int64(0)
empty = np.int64(0)

file_a = np.int64(0)
file_h = np.int64(0)

rank_1 = np.int64(0)
rank_4 = np.int64(0)
rank_5 = np.int64(0)
rank_8 = np.int64(0)

square_a8 = np.int64(0)

mousePos = (0, 0)
press = (-1, -1)
release = (-1, -1)
start = -1
end = -1

BLUE = (18, 201, 192)
WHITE = (249, 255, 212)
RED = (255, 0, 0, 50)
GREEN = (25, 166, 0, 150)
GREY = (150, 150, 150, 50)
YELLOW = (255, 255, 0, 50)

white_moves = set()
black_moves = set()


# given a list of squares to place the pieces, returns a numpy 64-bit integer representing the bitboard
# follows numbering scheme
# e.g. generate_bitboard([0, 2]) = 0000 0000 ... 0000 0101
def generate_bitboard(squares):
    a = np.int64(0)
    for i in squares:
        a = np.bitwise_or(a, np.left_shift(np.int64(1), i))
    return a


def init_bitboards():
    global bitboards
    bitboards = [0 for i in range(13)]
    bitboards[1] = generate_bitboard(list(range(8, 16)))
    bitboards[2] = generate_bitboard([1, 6])
    bitboards[3] = generate_bitboard([2, 5])
    bitboards[4] = generate_bitboard([0, 7])
    bitboards[5] = generate_bitboard([4])
    bitboards[6] = generate_bitboard([3])

    diff = 8 * 5  # 5 ranks between white and black's pawn ranks
    bitboards[7] = np.left_shift(bitboards[1], diff)

    diff = 8 * 7  # 7 ranks between white and black's back ranks
    bitboards[8] = np.left_shift(bitboards[2], diff)
    bitboards[9] = np.left_shift(bitboards[3], diff)
    bitboards[10] = np.left_shift(bitboards[4], diff)
    bitboards[11] = np.left_shift(bitboards[5], diff)
    bitboards[12] = np.left_shift(bitboards[6], diff)


def init_masks():
    global file_a, file_h, rank_1, rank_4, rank_5, rank_8, square_a8
    square_a8 = np.left_shift(np.int64(1), 63)
    file_a = generate_bitboard([7, 15, 23, 31, 39, 47, 55, 63])
    file_b = r_shift(file_a, 1)
    file_c = r_shift(file_a, 2)
    file_d = r_shift(file_a, 3)
    file_h = generate_bitboard([0, 8, 16, 24, 32, 40, 48, 56])

    rank_1 = generate_bitboard([0, 1, 2, 3, 4, 5, 6, 7])
    rank_4 = generate_bitboard([24, 25, 26, 27, 28, 29, 30, 31])
    rank_5 = generate_bitboard([32, 33, 34, 35, 36, 37, 38, 39])
    rank_8 = generate_bitboard([56, 57, 58, 59, 60, 61, 62, 63])


# imagine starting at the top left (63), reading left to right, then going to rank below, counting down
# 0 is h1, 63 is a8
def print_numbering_scheme():
    print('Bitboard binary representation:')

    for i in range(63, -1, -1):
        e = '    '
        if i >= 10:
            e = '   '
        if i % 8 == 0:
            e = '\n'
        print(i, end=e)
    print()
    print('----------------')

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
    return n[1] * 8 + (7 - n[0])


def l_shift(x, n):
    return np.left_shift(x, n)


def r_shift(x, n):
    # if first bit is a 1
    if np.bitwise_and(x, square_a8):
        y = np.right_shift(x, 1)
        y = np.bitwise_and(y, np.bitwise_not(square_a8))
        y = np.right_shift(y, n - 1)
        return y
    return np.right_shift(x, n)


# returns the piece type on that square
# 0  - empty square
# 1  - wP
# 2  - wN
# 3  - wB
# 4  - wR
# 5  - wQ
# 6  - wK
# 7  - bP
# 8  - bN
# 9  - bB
# 10 - bR
# 11 - bQ
# 12 - bK
def get_piece(square):
    for b in range(1, len(bitboards)):
        if np.bitwise_and(np.left_shift(np.int64(1), square), bitboards[b]):
            return b
    return 0


def multi_or(args):
    a = args[0]
    for i in range(1, len(args)):
        a = np.bitwise_or(a, args[i])
    return a


def multi_and(args):
    a = args[0]
    for i in range(1, len(args)):
        a = np.bitwise_and(a, args[i])
    return a


def get_reverse_mask(num_ones):
    a = np.int64(0)
    base = np.int64(0)
    for i in range(0, num_ones):
        base = np.bitwise_or(base, np.left_shift(np.int64(1), i))
    for i in range(0, 32 // num_ones):
        a = np.bitwise_or(a, np.left_shift(base, i * (2 * num_ones)))
    return a


def reverse_chunk(x, m):
    mask = get_reverse_mask(m)
    return np.bitwise_or(np.right_shift(np.bitwise_and(x, np.bitwise_not(mask)), m),
                         np.left_shift(np.bitwise_and(x, mask), m))


def reverse_bits(x):
    r = reverse_chunk(x, 1)
    r = reverse_chunk(r, 2)
    r = reverse_chunk(r, 4)
    r = reverse_chunk(r, 8)
    r = reverse_chunk(r, 16)
    r = np.bitwise_or(np.right_shift(r, 32), np.left_shift(r, 32))
    return r


def possible_wP():
    wP = bitboards[1]
    moves = set()
    # capture right
    pawnMoves = multi_and([np.left_shift(wP, 7), black_pieces, np.bitwise_not(rank_8), np.bitwise_not(file_a)])
    for i in range(64):
        if np.bitwise_and(np.left_shift(np.int64(1), i), pawnMoves):
            moves.add((i - 7, i, 0))
    # capture left
    pawnMoves = multi_and([np.left_shift(wP, 9), black_pieces, np.bitwise_not(rank_8), np.bitwise_not(file_h)])
    for i in range(64):
        if np.bitwise_and(np.left_shift(np.int64(1), i), pawnMoves):
            moves.add((i - 9, i, 0))
    # one forward
    pawnMoves = multi_and([np.left_shift(wP, 8), empty, np.bitwise_not(rank_8)])
    for i in range(64):
        if np.bitwise_and(np.left_shift(np.int64(1), i), pawnMoves):
            moves.add((i - 8, i, 0))
    # two forward
    pawnMoves = multi_and([np.left_shift(wP, 16), empty, np.left_shift(empty, 8), rank_4])
    for i in range(64):
        if np.bitwise_and(np.left_shift(np.int64(1), i), pawnMoves):
            moves.add((i - 16, i, 0))
    # promotion by capture right
    pawnMoves = multi_and([np.left_shift(wP, 7), black_pieces, rank_8, np.bitwise_not(file_a)])
    for i in range(64):
        if np.bitwise_and(np.left_shift(np.int64(1), i), pawnMoves):
            for j in range(2, 6):
                moves.add((i - 7, i, j))
    # promotion by capture left
    pawnMoves = multi_and([np.left_shift(wP, 9), black_pieces, rank_8, np.bitwise_not(file_h)])
    for i in range(64):
        if np.bitwise_and(np.left_shift(np.int64(1), i), pawnMoves):
            for j in range(2, 6):
                moves.add((i - 9, i, j))
    # promotion by one forward
    pawnMoves = multi_and([np.left_shift(wP, 8), empty, rank_8])
    for i in range(64):
        if np.bitwise_and(np.left_shift(np.int64(1), i), pawnMoves):
            for j in range(2, 6):
                moves.add((i - 8, i, j))
    return moves


def possible_wN():
    wN = bitboards[2]
    moves = set()
    return moves


def possible_wB():
    wB = bitboards[3]
    moves = set()
    return moves


def possible_wR():
    wR = bitboards[4]
    moves = set()
    return moves


def possible_wQ():
    wQ = bitboards[5]
    moves = set()
    return moves


def possible_wK():
    wK = bitboards[6]
    moves = set()
    return moves


def possible_moves_white():
    global white_moves, not_white_pieces, black_pieces, empty
    b = bitboards
    not_white_pieces = np.bitwise_not(multi_or(b[1:7] + b[12]))
    black_pieces = multi_or(b[7:12])
    empty = np.bitwise_not(multi_or(b[1:13]))
    moves = [possible_wP(), possible_wN(), possible_wB(), possible_wR(), possible_wQ(), possible_wK()]
    white_moves = set().union(*moves)


def line_atk(o, m, s):
    print("this is o", np.binary_repr(o, 64))
    print("this is m", np.binary_repr(m, 64))
    print("this is s", np.binary_repr(s, 64))
    left = np.bitwise_xor(o, o-2*s)
    rev_o = reverse_bits(np.bitwise_and(o, m))
    rev_2s = reverse_bits(2*s)
    right = np.bitwise_xor(o, reverse_bits(reverse_bits(o)-reverse_bits(2*s)))
    without_m = np.bitwise_xor(left, right)
    return right
    return np.bitwise_and(without_m, m)


def possible_bP():
    bP = bitboards[7]
    moves = set()
    # capture left
    pawnMoves = multi_and([np.right_shift(bP, 7), white_pieces, np.bitwise_not(rank_1), np.bitwise_not(file_h)])
    for i in range(64):
        if np.bitwise_and(np.left_shift(np.int64(1), i), pawnMoves):
            moves.add((i + 7, i, 0))
    # capture right
    pawnMoves = multi_and([np.right_shift(bP, 9), white_pieces, np.bitwise_not(rank_1), np.bitwise_not(file_a)])
    for i in range(64):
        if np.bitwise_and(np.left_shift(np.int64(1), i), pawnMoves):
            moves.add((i + 9, i, 0))
    # one forward
    pawnMoves = multi_and([np.right_shift(bP, 8), empty, np.bitwise_not(rank_1)])
    for i in range(64):
        if np.bitwise_and(np.left_shift(np.int64(1), i), pawnMoves):
            moves.add((i + 8, i, 0))
    # two forward
    pawnMoves = multi_and([np.right_shift(bP, 16), empty, np.right_shift(empty, 8), rank_5])
    for i in range(64):
        if np.bitwise_and(np.left_shift(np.int64(1), i), pawnMoves):
            moves.add((i + 16, i, 0))
    # promotion by capture left
    pawnMoves = multi_and([np.right_shift(bP, 7), white_pieces, rank_1, np.bitwise_not(file_h)])
    for i in range(64):
        if np.bitwise_and(np.left_shift(np.int64(1), i), pawnMoves):
            for j in range(8, 12):
                moves.add((i + 7, i, j))
    # promotion by capture right
    pawnMoves = multi_and([np.right_shift(bP, 9), white_pieces, rank_1, np.bitwise_not(file_a)])
    for i in range(64):
        if np.bitwise_and(np.left_shift(np.int64(1), i), pawnMoves):
            for j in range(8, 12):
                moves.add((i + 9, i, j))
    # promotion one forward
    pawnMoves = multi_and([np.right_shift(bP, 8), empty, rank_1])
    for i in range(64):
        if np.bitwise_and(np.left_shift(np.int64(1), i), pawnMoves):
            for j in range(8, 12):
                moves.add((i + 8, i, j))
    return moves


def possible_bN():
    bN = bitboards[8]
    moves = set()
    return moves


def possible_bB():
    bB = bitboards[9]
    moves = set()
    return moves


def possible_bR():
    bR = bitboards[10]
    moves = set()
    return moves


def possible_bQ():
    bQ = bitboards[11]
    moves = set()
    return moves


def possible_bK():
    bK = bitboards[12]
    moves = set()
    return moves


def possible_moves_black():
    global black_moves, not_black_pieces, white_pieces, empty
    b = bitboards
    not_black_pieces = np.bitwise_not(multi_or(b[7:13] + b[6]))
    white_pieces = multi_or(b[1:6])
    empty = np.bitwise_not(multi_or(b[1:13]))
    moves = [possible_bP(), possible_bN(), possible_bB(), possible_bR(), possible_bQ(), possible_bK()]
    black_moves = set().union(*moves)


# white: P = 1, N = 2, B = 3, R = 4, Q = 5, K = 6
# black: P = 7, N = 8, B = 9, R = 10, Q = 11, K = 12
def init_board():
    global piece_img
    print_numbering_scheme()
    init_bitboards()
    init_masks()
    piece_img = [None] * 13

    files = ['', 'Images/WhitePawn.png', 'Images/WhiteKnight.png', 'Images/WhiteBishop.png',
             'Images/WhiteRook.png', 'Images/WhiteQueen.png', 'Images/WhiteKing.png',

             'Images/BlackPawn.png', 'Images/BlackKnight.png', 'Images/BlackBishop.png',
             'Images/BlackRook.png', 'Images/BlackQueen.png', 'Images/BlackKing.png']
    for j in range(1, 13):
        piece_img[j] = pygame.image.load(files[j])
        piece_img[j] = pygame.transform.scale(piece_img[j], (50, 50))


# start - square the move starts on
# end - square the move ends on
# promo - piece being promoted to
# 0 - no promotion
# 1-12 - promoting to that piece type
def is_legal_move(start, end, promo):
    return (start, end, promo) in white_moves or (start, end, promo) in black_moves


# sets the square to 0 in the corresponding piece's bitboard
def remove_piece(piece, square):
    b = bitboards[piece]
    remove_mask = np.bitwise_not(np.left_shift(np.int64(1), square))
    bitboards[piece] = np.bitwise_and(b, remove_mask)


# sets the square to 1 in the corresponding piece's bitboard
def add_piece(piece, square):
    b = bitboards[piece]
    add_mask = np.left_shift(np.int64(1), square)
    bitboards[piece] = np.bitwise_or(b, add_mask)


def apply_move(start, end, promo_num):
    moved_piece = get_piece(start)
    captured_piece = get_piece(end)
    remove_piece(moved_piece, start)
    if captured_piece:
        remove_piece(captured_piece, end)
    if promo_num:
        add_piece(promo_num, end)
    else:
        add_piece(moved_piece, end)
    print(start, end, promo_num)


def is_white_piece(piece):
    return 1 <= piece <= 6


def is_black_piece(piece):
    return 7 <= piece <= 12


def get_promo_num(is_white, key):
    if key == 'n':
        return (8, 2)[is_white]
    if key == 'b':
        return (9, 3)[is_white]
    if key == 'r':
        return (10, 4)[is_white]
    if key == 'q':
        return (11, 5)[is_white]
    return 0


def run_game():
    global screen, press, release, start, end, mousePos
    mainClock = pygame.time.Clock()
    pygame.init()
    pygame.display.set_caption('Chess')
    screen = pygame.display.set_mode((400, 400), 0, 32)
    clicking = False
    init_board()
    occupied = generate_bitboard([39, 38, 34, 32])
    slider = generate_bitboard([34])
    mask = rank_5
    final = line_atk(occupied, mask, slider)
    draw_board()
    draw_bitboard(final, YELLOW)
    possible_moves_white()
    draw_possible_moves(white_moves, RED)
    possible_moves_black()
    draw_possible_moves(black_moves, GREEN)
    press = (-1, -1)
    release = (-1, -1)
    start = -1
    end = -1
    promo_key = ''

    while True:
        mousePos = pygame.mouse.get_pos()
        for event in pygame.event.get():
            if event.type == QUIT or (event.type == KEYDOWN and event.key == K_ESCAPE):
                pygame.quit()
                sys.exit()
            if event.type == KEYDOWN:
                if event.key == K_n:
                    promo_key = 'n'
                elif event.key == K_b:
                    promo_key = 'b'
                elif event.key == K_r:
                    promo_key = 'r'
                elif event.key == K_q:
                    promo_key = 'q'
                else:
                    promo_key = ''
            if event.type == KEYUP:
                promo_key = ''
            if event.type == MOUSEBUTTONDOWN:
                if event.button == BUTTON_LEFT and not clicking:
                    clicking = True
                    press = mousePos
                    press = math.floor(press[0] / 50), math.ceil(7 - press[1] / 50)
                    start = coords_to_num(press)
            if event.type == MOUSEBUTTONUP:
                if event.button == BUTTON_LEFT and clicking:
                    clicking = False
                    release = mousePos
                    release = math.floor(release[0] / 50), math.ceil(7 - release[1] / 50)
                    end = coords_to_num(release)
                    piece = get_piece(start)
                    promo_num = get_promo_num(is_white_piece(piece), promo_key)
                    if is_legal_move(start, end, promo_num):
                        apply_move(start, end, promo_num)
                    press = (-1, -1)
                    release = (-1, -1)
                    start = -1
                    end = -1
                    draw_board()
                    draw_bitboard(final, YELLOW)
                    possible_moves_white()
                    possible_moves_black()
                    draw_possible_moves(white_moves, RED)
                    draw_possible_moves(black_moves, GREEN)
                    #
                    # # a move hase been made
                    # if is_legal_move(start, end):
                    #     apply_move(start, end)
                    #     draw_board()
                    # else:
                    #     print("illegal")
            if start > -1:
                draw_board()
                draw_bitboard(final, YELLOW)
                draw_possible_moves(white_moves, RED)
                draw_possible_moves(black_moves, GREEN)

        pygame.display.update()
        mainClock.tick(100)


def draw_board():
    for i in range(64):
        if (get_file(i) + get_rank(i)) % 2 == 0:
            square_color = BLUE
        else:
            square_color = WHITE
        pygame.draw.rect(screen, square_color, (350 - (i % 8) * 50, 350 - (i // 8) * 50, 50, 50))
        if i == start:
            continue
        for b in range(1, len(bitboards)):
            if np.bitwise_and(np.left_shift(np.int64(1), i), bitboards[b]):
                screen.blit(pygame.transform.rotate(piece_img[b], 0), (350 - (i % 8) * 50, 350 - (i // 8) * 50))
    if start > -1 and get_piece(start) > 0:
        screen.blit(pygame.transform.rotate(piece_img[get_piece(start)], 0), (mousePos[0] - 25, mousePos[1] - 25))


def draw_bitboard(bitboard, color):
    for i in range(64):
        if np.bitwise_and(np.left_shift(np.int64(1), i), bitboard):
            pygame.draw.circle(screen, color, (350 - (i % 8) * 50 + 25, 350 - (i // 8) * 50 + 25), 5)

def draw_possible_moves(moves, color):
    return
    # this code will not run
    for i in moves:
        pygame.draw.line(screen, color, (350 - (i[0] % 8) * 50 + 25, 350 - (i[0] // 8) * 50 + 25),
                         (350 - (i[1] % 8) * 50 + 25, 350 - (i[1] // 8) * 50 + 25))
        pygame.draw.circle(screen, color, (350 - (i[1] % 8) * 50 + 25, 350 - (i[1] // 8) * 50 + 25), 5)


run_game()
