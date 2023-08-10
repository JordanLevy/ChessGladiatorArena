# from ctypes import *
import math
import sys
import threading
import time
import subprocess

import pygame
from pygame.locals import *

#Pc2c4 pe7e6 Nb1c3 ng8f6 Pd2d4 bf8b4 Bc1d2 nb8c6 Pe2e4 bb4c3 Bd2c3 nf6e4 Bf1d3 ne4c3 Pb2c3 pd7d6 Ng1f3 ke8g8 Ke1g1 qd8f6
# Qd1c2 pd6d5 Bd3h7 kg8h8 Bh7d3 pd5c4 Bd3c4 pa7a6 Qc2e4 kh8g8 Bc4d3 qf6f5 Qe4c6 pb7c6 Bd3f5 pe6f5 Nf3e5 bc8e6 Ne5c6 pf5f4
# Nc6e7 kg8h8 Pf2f3 ra8e8 Pd4d5 re8e7 Pd4d5 re8e7 Pd5e6 re7e6

class Move:
    def __init__(self, start, end, move_id, capture, piece_id, eval):
        self.start = start
        self.end = end
        self.move_id = move_id
        self.capture = capture
        self.piece_id = piece_id
        self.eval = eval

    def __str__(self):
        return '(' + str(self.start) + ', ' + str(self.end) + ') ' + str(self.move_id)


clock_start = 0

screen = None
piece_img = []

mouse_xy = (0, 0)
press_xy = (-1, -1)
release_xy = (-1, -1)
press_square = -1
release_square = -1

BLUE = (18, 201, 192)
WHITE = (249, 255, 212)
RED = (255, 0, 0, 50)
GREEN = (25, 166, 0, 50)
GREY = (150, 150, 150, 50)
YELLOW = (255, 255, 0, 50)
GRAY_GREEN = (118, 176, 151, 50)

board = []
white_turn = True
move_list = []
position_list = []
move_count = 0
next_spec = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]

depth = 8

start_pos = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"

black_to_mate = 'r4k2/8/8/8/8/6R1/3QPPPP/6K1 w - - 0 1'
mate_in_1_3 = '2K5/4q3/5r2/8/8/8/5k2/8 w - - 0 1'
black_promo_mate = '8/8/8/8/8/8/PPP3kp/K7 w - - 0 1'
black_ep_mate = '7b/8/8/8/p1p5/P7/KP4k1/RB6 w - - 0 1'
best_move_castle = 'r3k3/pp4p1/2p3pp/7n/4P1q1/1QNP1Rb1/PP4BK/8 w q - 0 23'
white_bodies = 'rnbrkbnn/n7/8/8/8/8/PPPPPPPP/RNBQKBNR w KQ - 0 1'

EMPTY_SQUARE = 0

bP = 1
bN = 2
bB = 3
bR = 4
bQ = 5
bK = 6

wP = 9
wN = 10
wB = 11
wR = 12
wQ = 13
wK = 14

letter_to_piece = {'p': bP, 'n': bN, 'b': bB, 'r': bR, 'q': bQ, 'k': bK, 'P': wP, 'N': wN, 'B': wB, 'R': wR, 'Q': wQ,
                   'K': wK}
piece_to_letter = {bP: 'p', bN: 'n', bB: 'b', bR: 'r', bQ: 'q', bK: 'k', wP: 'P', wN: 'N', wB: 'B', wR: 'R', wQ: 'Q',
                   wK: 'K'}

white_promo = {'n': wN, 'b': wB, 'r': wR, 'q': wQ}
black_promo = {'n': bN, 'b': bB, 'r': bR, 'q': bQ}

kingside_wR = EMPTY_SQUARE
queenside_wR = EMPTY_SQUARE
kingside_bR = EMPTY_SQUARE
queenside_bR = EMPTY_SQUARE

kingside_wR_num_moves = 0
queenside_wR_num_moves = 0
kingside_bR_num_moves = 0
queenside_bR_num_moves = 0

wK_num_moves = 0
bK_num_moves = 0

show_spec = True
engine_enabled = True

path_to_exe = './ChessEngine/main.exe'


# get what file you are on given an index 0-63
def get_file(n):
    return n % 8


def get_file_letter(n):
    letter = chr(104 - get_file(n))
    return letter


# get what rank you are on given an index 0-63
def get_rank(n):
    return n // 8


# takes in a number and spits out the square in algabraic notation
def square_num_to_notation(n):
    all_notation = get_file_letter(n)
    all_notation += str(get_rank(n) + 1)
    return all_notation


# turns a tuple n=(x, y) into an index from 0-63
def coords_to_num(n):
    return n[1] * 8 + (7 - n[0])


def decode_notation(move):
    start_file = move[0]
    start_rank = int(move[1])
    end_file = move[2]
    end_rank = int(move[3])
    promo_num = 0

    start_file_num = ord(start_file) - 97
    end_file_num = ord(end_file) - 97

    start_rank -= 1
    end_rank -= 1

    start = coords_to_num((start_file_num, start_rank))
    end = coords_to_num((end_file_num, end_rank))

    if len(move) == 5:
        promo_num = get_promo_num(is_white_piece(start), move[4])

    return start, end, promo_num


def get_piece(square):
    return board[square]


def set_piece(square, piece_type):
    board[square] = piece_type << 4


def is_white_piece(piece_id):
    return (piece_id >> 7) == 1


def is_black_piece(piece_id):
    return (piece_id >> 7) == 0


def get_promo_num(is_white, key):
    if key == '':
        return 0
    if is_white:
        return white_promo[key]
    return black_promo[key]


def append_move(move):
    move_list.append(move)


def get_previous_move():
    if not move_list:
        return None
    return move_list[-1]


def init_board():
    global piece_img
    piece_img = [None] * 15

    files = ['',
             'Images/BlackPawn.png', 'Images/BlackKnight.png', 'Images/BlackBishop.png',
             'Images/BlackRook.png', 'Images/BlackQueen.png', 'Images/BlackKing.png',
             '', '',
             'Images/WhitePawn.png', 'Images/WhiteKnight.png', 'Images/WhiteBishop.png',
             'Images/WhiteRook.png', 'Images/WhiteQueen.png', 'Images/WhiteKing.png']
    for j in range(1, 15):
        if not files[j]:
            continue
        piece_img[j] = pygame.image.load(files[j])
        piece_img[j] = pygame.transform.scale(piece_img[j], (50, 50))


def get_type(piece_id):
    return piece_id >> 4


def get_spec(piece_id):
    return piece_id & 15


def board_to_fen():
    fen = ""
    spaces = 0

    for i in range(63, -1, -1):
        piece = get_type(board[i])
        if (piece not in piece_to_letter):
            spaces += 1
        else:
            if (spaces != 0):
                fen += str(spaces)
                spaces = 0
            fen += piece_to_letter[piece]
        if (i % 8 == 0):
            if (spaces != 0):
                fen += str(spaces)
                spaces = 0
            if i != 0:
                fen += "/"
    if white_turn:
        fen += " w"
    else:
        fen += " b"
    castling_rights = ""
    if wK_num_moves == 0:
        if kingside_wR_num_moves == 0:
            castling_rights += "K"
        if queenside_wR_num_moves == 0:
            castling_rights += "Q"
    if bK_num_moves == 0:
        if kingside_bR_num_moves == 0:
            castling_rights += "k"
        if queenside_bR_num_moves == 0:
            castling_rights += "q"
    fen += " " + castling_rights
    # this is for en passant right
    last_move = get_previous_move()
    if last_move.move_id == 16:
        # if the doble pawn puch is wight
        if is_white_piece(last_move.piece_id):
            ep_square = square_num_to_notation(last_move.end - 8)
        # black en square
        else:
            ep_square = square_num_to_notation(last_move.end + 8)
        fen += " " + ep_square
    return fen


def add_piece(square, piece_type):
    spec = next_spec[piece_type]
    board[square] = (piece_type << 4) | spec
    next_spec[piece_type] += 1
    return board[square]


def init_fen(fen):
    global board, white_turn, kingside_wR, queenside_wR, kingside_bR, queenside_bR, next_spec, wK_num_moves, bK_num_moves, kingside_wR_num_moves, queenside_wR_num_moves, kingside_bR_num_moves, queenside_bR_num_moves
    next_spec = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
    board = [EMPTY_SQUARE for i in range(64)]
    split = fen.split(' ')
    n = len(split)
    pieces = ''
    turn = ''
    castling_rights = ''
    en_passant = ''
    fifty_move_rule = ''
    turn_number = ''
    if n > 0:
        pieces = split[0]
    if n > 1:
        turn = split[1]
    if n > 2:
        castling_rights = split[2]
    if n > 3:
        en_passant = split[3]
    if n > 4:
        fifty_move_rule = split[4]
    if n > 5:
        turn_number = split[5]

    index = 0
    square = 63
    n = len(pieces)
    while index < n:
        char = pieces[index]
        if char == '/':
            index += 1
            continue
        elif char in letter_to_piece:
            p_type = letter_to_piece[char]
            p_ID = add_piece(square, letter_to_piece[char])
            if p_type == wR:
                if (square == 0):
                    kingside_wR = p_ID
                elif (square == 7):
                    queenside_wR = p_ID
            elif p_type == bR:
                if (square == 56):
                    kingside_bR = p_ID
                elif (square == 63):
                    queenside_bR = p_ID
            # set_piece(square, letter_to_piece[char])
            square -= 1
        else:
            square -= int(char)
        index += 1
    white_turn = (turn == 'w')

    if castling_rights:
        if 'K' in castling_rights:
            wK_num_moves = 0
            kingside_wR_num_moves = 0
        if 'Q' in castling_rights:
            wK_num_moves = 0
            queenside_wR_num_moves = 0
        if 'k' in castling_rights:
            bK_num_moves = 0
            kingside_bR_num_moves = 0
        if 'q' in castling_rights:
            bK_num_moves = 0
            queenside_bR_num_moves = 0


def draw_board():
    w_check = False
    b_check = False

    font = pygame.font.SysFont('Arial', 18, bold=True)
    prev_move = None
    if move_list:
        prev_move = move_list[-1]

    for i in range(64):
        piece_id = get_piece(i)
        piece_type = get_type(piece_id)
        piece_spec = get_spec(piece_id)
        # color the dark squares
        if (get_file(i) + get_rank(i)) % 2 == 1:
            square_color = BLUE
        # color the light squares
        else:
            square_color = WHITE
        # if a king is in check, color their square red
        if (w_check and piece_type == wK) or (b_check and piece_type == bK):
            square_color = RED
        if prev_move:
            if prev_move.start == i:
                square_color = YELLOW
            elif prev_move.end == i:
                square_color = YELLOW
        # draw the squares on the board
        pygame.draw.rect(screen, square_color, (350 - (i % 8) * 50, 350 - (i // 8) * 50, 50, 50))
        # if there is a piece on this square and it's not currently being held
        if piece_type != EMPTY_SQUARE and i != press_square:
            screen.blit(pygame.transform.rotate(piece_img[piece_type], 0), (350 - (i % 8) * 50, 350 - (i // 8) * 50))
            if show_spec:
                img = font.render(str(piece_spec), True, pygame.Color(WHITE), pygame.Color(GRAY_GREEN))
                screen.blit(img, (350 - (i % 8) * 50, 350 - (i // 8) * 50))
    # if there is a piece being held, draw it at the mouse position
    if press_square > -1 and get_type(get_piece(press_square)) > 0:
        piece_spec = get_spec(get_piece(press_square))
        screen.blit(pygame.transform.rotate(piece_img[get_type(get_piece(press_square))], 0),
                    (mouse_xy[0] - 25, mouse_xy[1] - 25))
        if show_spec:
            img = font.render(str(piece_spec), True, pygame.Color(WHITE), pygame.Color(GRAY_GREEN))
            screen.blit(img, (mouse_xy[0] - 25, mouse_xy[1] - 25))


def refresh_graphics():
    draw_board()


def print_move(m):
    letter = piece_to_letter[get_type(m.piece_id)]
    start = square_num_to_notation(m.start)
    end = square_num_to_notation(m.end)
    move = letter+start+end
    print(move, end=' ')


def print_move_list():
    for m in move_list:
        print_move(m)
    print()


def apply_move(start, end, promo_val):
    global board, wK_num_moves, bK_num_moves, kingside_wR_num_moves, queenside_wR_num_moves, kingside_bR_num_moves, queenside_bR_num_moves
    move_id = 0
    piece_id = board[start]
    piece_type = get_type(piece_id)
    capture = board[end]
    evaluation = 0
    board[end] = board[start]
    board[start] = EMPTY_SQUARE
    # check if pieces are moveing
    p_type = get_type(board[end])
    p_ID = board[end]
    if get_type(board[end]) == wK:
        wK_num_moves += 1
    if get_type(board[end]) == bK:
        bK_num_moves += 1
    if p_ID == kingside_wR:
        kingside_wR_num_moves += 1
    if p_ID == queenside_wR:
        queenside_wR_num_moves += 1
    if p_ID == kingside_bR:
        kingside_bR_num_moves += 1
    if p_ID == queenside_bR:
        queenside_bR_num_moves += 1
    prev_move = get_previous_move()
    # previous move was a double pawn push
    if prev_move and prev_move.move_id == 16:
        # capture en passant
        if (piece_type == wP or piece_type == bP) and (start - 1 == prev_move.end or start + 1 == prev_move.end):
            move_id = 17
            board[prev_move.end] = EMPTY_SQUARE
    # double pawn push
    if piece_type == wP and get_rank(start) == 1 and get_rank(end) == 3 and get_file(start) == get_file(end):
        move_id = 16
    elif piece_type == bP and get_rank(start) == 6 and get_rank(end) == 4 and get_file(start) == get_file(end):
        move_id = 16
    # casiling for white
    if piece_type == wK and start == 3:
        move_id = 18
        if end == 1:
            # this is moving the white rook for casaling king side
            board[2] = board[0]
            board[0] = EMPTY_SQUARE
        elif end == 5:
            # this is moving the rook casaling queen side
            board[4] = board[7]
            board[7] = EMPTY_SQUARE
    elif piece_type == bK and start == 59:
        move_id = 18
        if end == 57:
            # this is moving the black rook for casaling king side
            board[58] = board[56]
            board[56] = EMPTY_SQUARE
        elif end == 61:
            # this is moving the rook casaling queen side
            board[60] = board[63]
            board[63] = EMPTY_SQUARE
    # promotion for white pawn
    if piece_type == wP and get_rank(end) == 7:
        move_id = promo_val
        set_piece(end, promo_val)
    # promo for black pawn
    elif piece_type == bP and get_rank(end) == 0:
        move_id = promo_val
        set_piece(end, promo_val)
    new_move = Move(start, end, move_id, capture, piece_id, evaluation)
    append_move(new_move)
    print_move_list()


def run_game(process):
    global board, white_turn, screen, press_xy, release_xy, press_square, release_square, mouse_xy, clock_start
    screen = pygame.display.set_mode((400, 400), 0, 32)
    main_clock = pygame.time.Clock()
    pygame.display.init()
    pygame.display.set_caption('Chess')
    pygame.font.init()
    clicking = False
    init_board()
    init_fen(start_pos)
    position_list.append(start_pos)
    refresh_graphics()
    press_xy = (-1, -1)
    release_xy = (-1, -1)
    press_square = -1
    release_square = -1
    promo_key = ''

    while True:
        mouse_xy = pygame.mouse.get_pos()
        for event in pygame.event.get():
            if event.type == QUIT or (event.type == KEYDOWN and event.key == K_ESCAPE):
                pygame.quit()
                sys.exit()
            if event.type == KEYDOWN:
                if event.key == K_SPACE:
                    send_command(process, 'isready')
                elif event.key == K_LEFT:
                    # if there is a move to undo
                    if len(position_list) > 1:
                        position_list.pop()
                        init_fen(position_list[-1])
                        refresh_graphics()
                    else:
                        print("can't undo")
                elif event.key == K_p:
                    send_command(process, 'go perft 6')
                elif event.key == K_n:
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
                    press_xy = mouse_xy
                    press_xy = math.floor(press_xy[0] / 50), math.ceil(7 - press_xy[1] / 50)
                    press_square = coords_to_num(press_xy)
            if event.type == MOUSEBUTTONUP:
                if event.button == BUTTON_LEFT and clicking:
                    clicking = False
                    release_xy = mouse_xy
                    release_xy = math.floor(release_xy[0] / 50), math.ceil(7 - release_xy[1] / 50)
                    release_square = coords_to_num(release_xy)
                    piece = get_piece(press_square)
                    promo_num = get_promo_num(is_white_piece(piece), promo_key)

                    # can't start and end a move on the same square
                    if not press_square == release_square:
                        # human move
                        apply_move(press_square, release_square, promo_num)
                        white_turn = not white_turn
                        fen = board_to_fen()
                        position_list.append(fen)
                        if engine_enabled:
                            send_command(process, 'position fen ' + fen)
                            white_turn = not white_turn
                            send_command(process, 'go depth ' + str(depth))
                            clock_start = time.time()
                        else:
                            send_command(process, 'position fen ' + fen)
                    press_xy = (-1, -1)
                    release_xy = (-1, -1)
                    press_square = -1
                    release_square = -1
                    refresh_graphics()
            if press_square > -1:
                refresh_graphics()

        pygame.display.update()
        main_clock.tick(100)


def init_process(path):
    return subprocess.Popen([path], stdin=subprocess.PIPE, stdout=subprocess.PIPE)


def send_command(process, cmd):
    process.stdin.write((cmd + '\n').encode())
    process.stdin.flush()


def open_communication():
    process = init_process(path_to_exe)
    read_thread = threading.Thread(target=read_from_process, args=(process,))
    # write_thread = threading.Thread(target=write_to_process, args=(process,))
    game_thread = threading.Thread(target=run_game, args=(process,))
    read_thread.start()
    # write_thread.start()
    game_thread.start()
    read_thread.join()
    # write_thread.join()
    game_thread.join()
    print('close communication')
    close_communication(process)


def close_communication(process):
    process.terminate()


def read_from_process(process):
    while True:
        output = process.stdout.readline()
        if output == b'':
            break
        response = output.decode().strip()
        print(response)
        if response.startswith('bestmove'):
            print('time: ', time.time() - clock_start)
            cmd, move = response.split(' ')
            if move == '(none)':
                print('Human wins')
                break
            start, end, promo = decode_notation(move)
            apply_move(start, end, promo)
            fen = board_to_fen()
            position_list.append(fen)
            refresh_graphics()


open_communication()

#Pe2e4 pd7d5 Pe4d5 qd8d5 Nb1c3 qd5e6 Bf1e2 nb8c6 Pf2f4 nc6d4 Pd2d4 qe6g6 Be2f3 nc6b4 Bf3e4 pf7f5 Be4d3 qg6g2 Bd3f5 qg2h1
