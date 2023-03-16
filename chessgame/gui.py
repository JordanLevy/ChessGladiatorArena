# from ctypes import *
import math
import sys
import threading
import time
import subprocess

import pygame
from pygame.locals import *

# class Move(Structure):
#     _fields_ = [('start', c_int),
#                 ('end', c_int),
#                 ('id', c_int),
#                 ('capture', c_int),
#                 ('piece', c_int),
#                 ('eval', c_int)]


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
move_list_fen = []
# lib = CDLL('./chess_game.so')
#
# lib.init.argtypes = [c_char_p, c_int]
#
# lib.apply_move.restype = c_bool
#
# lib.try_undo_move.restype = c_bool
#
# lib.is_game_legal_move.restype = c_bool
# lib.is_game_legal_move.argtypes = [c_int, c_int, c_int]
#
# lib.get_board_state.restype = POINTER(c_char * 64)
#
# lib.get_white_check.restype = c_bool
#
# lib.get_black_check.restype = c_bool
#
# lib.perft_test.restype = c_int
# lib.perft_test.argtypes = [c_int]
#
# lib.detailed_perft.restype = c_int
# lib.detailed_perft.argtypes = [c_int]
#
# lib.calc_eng_move.restype = c_int
# lib.calc_eng_move.argtypes = [c_int]
#
# lib.calc_eng_move_with_test.restype = c_int
# lib.calc_eng_move_with_test.argtypes = [c_int, c_int]
#
# lib.get_eng_move_start.restype = c_int
# lib.get_eng_move_end.restype = c_int
# lib.get_eng_move_eval.restype = c_int
# lib.get_eng_move_id.restype = c_int
#
# lib.get_possible_moves.restype = POINTER(Move)
#
# lib.get_num_possible_moves.restype = c_int
#
# lib.get_mat_eval.restype = c_int
#
# lib.get_pos_eval.restype = c_int

move_count = 0

# fen = b"6K1/q7/8/5n2/8/8/8/7k w - -"
start_pos = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
test_pos1 = "r1bqkbnr/pppppppp/2n5/8/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 1 2"
crash_fen = 'r3k2r/ppp2p2/2n2q1p/3p2p1/P2P4/2P1P1P1/3NbPP1/2RQK2R w Kkq -'
ke2_fen = 'r3k2r/ppp2p2/2n2q1p/3p2p1/P2P4/2P1P1P1/3NKPP1/2RQ3R b kq -'
g5g4_fen = 'r3k2r/ppp2p2/2n2q1p/3p4/P2P2p1/2P1P1P1/3NKPP1/2RQ3R w kq -'
e3e4_fen = 'r3k2r/ppp2p2/2n2q1p/3p4/P2PP1p1/2P3P1/3NKPP1/2RQ3R b kq -'
Qf6f2_fen = 'r3k2r/ppp2p2/2n4p/3p4/P2PP1p1/2P3P1/3NKqP1/2RQ3R w kq - 0 3'

Qf6f2_fen_w_rook_on_b1 = 'r3k2r/ppp2p2/2n4p/3p4/P2PP1p1/2P3P1/3NKqP1/1R1Q3R w kq - 0 3'
Qf6f2_fen_w_rook_on_b1_and_knight_b4 = 'r3k2r/ppp2p2/2n4p/3p4/PN1PP1p1/2P3P1/2N1KqP1/1R1Q3R w kq - 0 3'
minimal = '4k3/8/8/8/8/8/4Kq2/1R1Q4 w - - 0 3'
minimal_not_touching = '4k3/8/8/8/8/8/4K1q1/1R4Q w - - 0 3'
minimal_diagonal = '4k3/8/8/8/8/8/4K3/1R1Q1q2 w - - 0 3'
minimal_flipped = '3k4/8/8/8/8/8/2qK4/1Q4R1 w - - 0 3'
minimal_3rd_rank = '4k3/8/8/8/8/4Kq2/8/1R1Q4 w - - 0 3'
minimal_rook = '4k3/8/8/8/8/4Kr2/8/1R1Q4 w - - 0 3'

start_e2e3 = 'rnbqkbnr/pppppppp/8/8/8/4P3/PPPP1PPP/RNBQKBNR b KQkq - 0 1'
start_h7h6 = 'rnbqkbnr/ppppppp1/7p/8/8/4P3/PPPP1PPP/RNBQKBNR w KQkq - 0 2'
start_d1h5 = 'rnbqkbnr/ppppppp1/7p/7Q/8/4P3/PPPP1PPP/RNB1KBNR b KQkq - 1 2'  # f7f6 is pinned, but still counted

crash_after_Qxe2 = 'r3k2r/ppp2p2/2n2q1p/3p2p1/P2P4/2P1P1P1/3NQPP1/2R1K2R b Kkq - 0 1'
after_castles = 'r4rk1/ppp2p2/2n2q1p/3p2p1/P2P4/2P1P1P1/3NQPP1/2R1K2R w K - 0 1'
after_f2f3 = 'r4rk1/ppp2p2/2n2q1p/3p2p1/P2P4/2P1PPP1/3NQ1P1/2R1K2R b K - 0 1'
after_Rf8c8 = 'r1r3k1/ppp2p2/2n2q1p/3p2p1/P2P4/2P1PPP1/3NQ1P1/2R1K2R w K - 1 2'
after_g3g4 = 'r1r3k1/ppp2p2/2n2q1p/3p2p1/P2P2P1/2P1PP2/3NQ1P1/2R1K2R b K - 0 2'
black_to_mate = 'r4k2/8/8/8/8/6R1/3QPPPP/6K1 w - - 0 1'
dont_know = '1r2k1r1/ppp2p2/2n2q1p/3p2p1/P2P4/2P1P1P1/3NQPP1/2R1K1R1 b Kkq - 0 1'
mate_in_1_3 = '2K5/4q3/5r2/8/8/8/5k2/8 w - - 0 1'

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

letter_to_piece = {'p': bP, 'n': bN, 'b': bB, 'r': bR, 'q': bQ, 'k': bK, 'P': wP, 'N': wN, 'B': wB, 'R': wR,'Q': wQ, 'K': wK}
piece_to_letter = {bP: 'p', bN: 'n', bB: 'b', bR: 'r', bQ: 'q', bK: 'k', wP: 'P', wN: 'N', wB: 'B', wR: 'R', wQ: 'Q', wK: 'K'}

white_promo = {'n': wN, 'b': wB, 'r': wR, 'q': wQ}
black_promo = {'n': bN, 'b': bB, 'r': bR, 'q': bQ}

show_spec = True

path_to_exe = './ChessEngine/bin/Debug/ChessEngine.exe'


# get what file you are on given an index 0-63
def get_file(n):
    return n % 8


# get what rank you are on given an index 0-63
def get_rank(n):
    return n // 8


# turns a tuple n=(x, y) into an index from 0-63
def coords_to_num(n):
    return n[1] * 8 + (7 - n[0])


def decode_notation(move):
    start_file = move[0]
    start_rank = int(move[1])
    end_file = move[2]
    end_rank = int(move[3])
    promo = None
    if len(move) == 5:
        if white_turn:
            promo = white_promo[move[4]]
        else:
            promo = black_promo[move[4]]

    start_file_num = ord(start_file) - 97
    end_file_num = ord(end_file) - 97

    start_rank -= 1
    end_rank -= 1

    start = coords_to_num((start_file_num, start_rank))
    end = coords_to_num((end_file_num, end_rank))

    return start, end, promo


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


def update_move_list(move):
    move_list_fen.append(move)


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
        if (i%8 == 0):
            if (spaces != 0):
                fen += str(spaces)
                spaces = 0
            if i != 0:
                fen += "/"
    if white_turn:
        fen += " w"
    else:
        fen += " b"
    return fen


# rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1
def init_fen(fen):
    global board, white_turn
    board = [EMPTY_SQUARE for i in range(64)]
    pieces, turn, castling_rights, en_passant, fifty_move_rule, turn_number = fen.split(' ')
    index = 0
    square = 63
    n = len(pieces)
    while index < n:
        char = pieces[index]
        if char == '/':
            index += 1
            continue
        elif char in letter_to_piece:
            set_piece(square, letter_to_piece[char])
            square -= 1
        else:
            square -= int(char)
        index += 1
    white_turn = (turn == 'w')


def draw_board():
    w_check = False
    b_check = False

    font = pygame.font.SysFont('Arial', 18, bold=True)

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


def get_updated_board():
    global board
    board = [i for i in lib.get_board_state().contents]


def print_move(m):
    print(m.piece, m.start, m.end)


def play_human_move(start, end, promo):
    global move_count
    lib.apply_move(start, end, promo)
    lib.update_game_possible_moves()
    get_updated_board()
    refresh_graphics()
    move_count += 1


def play_engine_move():
    st = time.time()
    evaluation = lib.calc_eng_move(6)
    # evaluation = lib.calc_eng_move_with_test(4, 6)

    start = lib.get_eng_move_start()
    end = lib.get_eng_move_end()
    move_id = lib.get_eng_move_id()
    lib.apply_move(start, end, move_id)
    lib.update_game_possible_moves()
    get_updated_board()


def teleport_piece(start, end, promo_val):
    global board
    board[end] = board[start]
    board[start] = EMPTY_SQUARE
    board_to_fen()

def run_game(process):
    global board, white_turn, screen, press_xy, release_xy, press_square, release_square, mouse_xy
    screen = pygame.display.set_mode((400, 400), 0, 32)
    main_clock = pygame.time.Clock()
    pygame.display.init()
    pygame.display.set_caption('Chess')
    pygame.font.init()
    clicking = False
    init_board()
    init_fen(start_pos)
    # lib.init(c_char_p(fen), len(fen))

    # lib.update_game_possible_moves()
    # get_updated_board()
    refresh_graphics()
    press_xy = (-1, -1)
    release_xy = (-1, -1)
    press_square = -1
    release_square = -1
    promo_key = ''

    # lib.game_order_moves()

    while True:
        mouse_xy = pygame.mouse.get_pos()
        for event in pygame.event.get():
            if event.type == QUIT or (event.type == KEYDOWN and event.key == K_ESCAPE):
                pygame.quit()
                sys.exit()
            if event.type == KEYDOWN:
                if event.key == K_SPACE:
                    pass
                    # lib.try_undo_move()
                    # lib.update_game_possible_moves()
                    # get_updated_board()
                    # refresh_graphics()
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

                    # human move
                    teleport_piece(press_square, release_square, promo_num)
                    white_turn = not white_turn
                    fen = board_to_fen()
                    send_command(process, 'position fen ' + fen)

                    white_turn = not white_turn
                    print('fen', fen)
                    send_command(process, 'go depth 6')
                    """
                    if lib.is_game_legal_move(press_square, release_square, promo_num):
                        play_human_move(press_square, release_square, promo_num)
                        play_engine_move()
                        # lib.game_order_moves()
                    else:
                        print('illegal', press_square, release_square, promo_num)
                        pass
                    """
                    press_xy = (-1, -1)
                    release_xy = (-1, -1)
                    press_square = -1
                    release_square = -1
                    refresh_graphics()
            if press_square > -1:
                refresh_graphics()

        pygame.display.update()
        main_clock.tick(100)


# def test():
#     st = time.time()
#     lib.init(c_char_p(fen), len(fen))
#     print(lib.detailed_perft(5))
#     print(time.time() - st)


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
            cmd, move = response.split(' ')
            start, end, promo = decode_notation(move)
            teleport_piece(start, end, promo)
            refresh_graphics()
        if response.startswith('info'):
            pass

# def write_to_process(process):
# while True:
# cmd = input()
# process.stdin.write((cmd + '\n').encode())
# process.stdin.flush()


open_communication()
