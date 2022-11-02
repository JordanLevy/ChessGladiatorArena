from ctypes import *
import math
import sys
import time

import pygame
from pygame.locals import *


class Move(Structure):
    _fields_ = [('start', c_int),
                ('end', c_int),
                ('id', c_int),
                ('capture', c_int),
                ('piece', c_int),
                ('eval', c_int)]


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

board = []
# test
lib = CDLL('./chess_game.so')

lib.init.argtypes = [c_char_p, c_int]

lib.apply_move.restype = c_bool

lib.try_undo_move.restype = c_bool

lib.is_game_legal_move.restype = c_bool
lib.is_game_legal_move.argtypes = [c_int, c_int, c_int]

lib.get_board_state.restype = POINTER(c_int * 64)

lib.get_white_check.restype = c_bool

lib.get_black_check.restype = c_bool

lib.perft_test.restype = c_int
lib.perft_test.argtypes = [c_int]

lib.detailed_perft.restype = c_int
lib.detailed_perft.argtypes = [c_int]

lib.calc_eng_move.restype = c_int
lib.calc_eng_move.argtypes = [c_int]

lib.calc_eng_move_with_test.restype = c_int
lib.calc_eng_move_with_test.argtypes = [c_int, c_int]

lib.get_eng_move_start.restype = c_int
lib.get_eng_move_end.restype = c_int
lib.get_eng_move_eval.restype = c_int
lib.get_eng_move_id.restype = c_int

lib.get_possible_moves.restype = POINTER(Move)

lib.get_num_possible_moves.restype = c_int

lib.get_mat_eval.restype = c_int

lib.get_pos_eval.restype = c_int

lib.get_rook_pos.restype = POINTER(c_int * 4)

move_count = 0

# fen = b"6K1/q7/8/5n2/8/8/8/7k w - -"
start_pos = b"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq -"
crash_fen = b'r3k2r/ppp2p2/2n2q1p/3p2p1/P2P4/2P1P1P1/3NbPP1/2RQK2R w Kkq -'
ke2_fen = b'r3k2r/ppp2p2/2n2q1p/3p2p1/P2P4/2P1P1P1/3NKPP1/2RQ3R b kq -'
g5g4_fen = b'r3k2r/ppp2p2/2n2q1p/3p4/P2P2p1/2P1P1P1/3NKPP1/2RQ3R w kq -'
e3e4_fen = b'r3k2r/ppp2p2/2n2q1p/3p4/P2PP1p1/2P3P1/3NKPP1/2RQ3R b kq -'
Qf6f2_fen = b'r3k2r/ppp2p2/2n4p/3p4/P2PP1p1/2P3P1/3NKqP1/2RQ3R w kq - 0 3'

Qf6f2_fen_w_rook_on_b1 = b'r3k2r/ppp2p2/2n4p/3p4/P2PP1p1/2P3P1/3NKqP1/1R1Q3R w kq - 0 3'
Qf6f2_fen_w_rook_on_b1_and_knight_b4 = b'r3k2r/ppp2p2/2n4p/3p4/PN1PP1p1/2P3P1/2N1KqP1/1R1Q3R w kq - 0 3'
minimal = b'4k3/8/8/8/8/8/4Kq2/1R1Q4 w - - 0 3'
minimal_not_touching = b'4k3/8/8/8/8/8/4K1q1/1R4Q w - - 0 3'
minimal_diagonal = b'4k3/8/8/8/8/8/4K3/1R1Q1q2 w - - 0 3'
minimal_flipped = b'3k4/8/8/8/8/8/2qK4/1Q4R1 w - - 0 3'
minimal_3rd_rank = b'4k3/8/8/8/8/4Kq2/8/1R1Q4 w - - 0 3'
minimal_rook = b'4k3/8/8/8/8/4Kr2/8/1R1Q4 w - - 0 3'

start_e2e3 = b'rnbqkbnr/pppppppp/8/8/8/4P3/PPPP1PPP/RNBQKBNR b KQkq - 0 1'
start_h7h6 = b'rnbqkbnr/ppppppp1/7p/8/8/4P3/PPPP1PPP/RNBQKBNR w KQkq - 0 2'
start_d1h5 = b'rnbqkbnr/ppppppp1/7p/7Q/8/4P3/PPPP1PPP/RNB1KBNR b KQkq - 1 2'  # f7f6 is pinned, but still counted

crash_after_Qxe2 = b'r3k2r/ppp2p2/2n2q1p/3p2p1/P2P4/2P1P1P1/3NQPP1/2R1K2R b Kkq - 0 1'
after_castles = b'r4rk1/ppp2p2/2n2q1p/3p2p1/P2P4/2P1P1P1/3NQPP1/2R1K2R w K - 0 1'
after_f2f3 = b'r4rk1/ppp2p2/2n2q1p/3p2p1/P2P4/2P1PPP1/3NQ1P1/2R1K2R b K - 0 1'
after_Rf8c8 = b'r1r3k1/ppp2p2/2n2q1p/3p2p1/P2P4/2P1PPP1/3NQ1P1/2R1K2R w K - 1 2'
after_g3g4 = b'r1r3k1/ppp2p2/2n2q1p/3p2p1/P2P2P1/2P1PP2/3NQ1P1/2R1K2R b K - 0 2'

fen = b'1r2k1r1/ppp2p2/2n2q1p/3p2p1/P2P4/2P1P1P1/3NQPP1/2R1K1R1 b Kkq - 0 1'


# get what file you are on given an index 0-63
def get_file(n):
    return n % 8


# get what rank you are on given an index 0-63
def get_rank(n):
    return n // 8


# turns a tuple n=(x, y) into an index from 0-63
def coords_to_num(n):
    return n[1] * 8 + (7 - n[0])


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
    return board[square]


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


def init_board():
    global piece_img
    piece_img = [None] * 13

    files = ['', 'Images/WhitePawn.png', 'Images/WhiteKnight.png', 'Images/WhiteBishop.png',
             'Images/WhiteRook.png', 'Images/WhiteQueen.png', 'Images/WhiteKing.png',

             'Images/BlackPawn.png', 'Images/BlackKnight.png', 'Images/BlackBishop.png',
             'Images/BlackRook.png', 'Images/BlackQueen.png', 'Images/BlackKing.png']
    for j in range(1, 13):
        piece_img[j] = pygame.image.load(files[j])
        piece_img[j] = pygame.transform.scale(piece_img[j], (50, 50))


def draw_board():
    w_check = lib.get_white_check()
    b_check = lib.get_black_check()

    for i in range(64):
        piece = get_piece(i)
        # color the dark squares
        if (get_file(i) + get_rank(i)) % 2 == 1:
            square_color = BLUE
        # color the light squares
        else:
            square_color = WHITE
        # if a king is in check, color their square red
        if (w_check and piece == 6) or (b_check and piece == 12):
            square_color = RED
        # draw the squares on the board
        pygame.draw.rect(screen, square_color, (350 - (i % 8) * 50, 350 - (i // 8) * 50, 50, 50))
        # if there is a piece on this square and it's not currently being held
        if piece > 0 and i != press_square:
            # draw the piece on the board
            screen.blit(pygame.transform.rotate(piece_img[piece], 0), (350 - (i % 8) * 50, 350 - (i // 8) * 50))
    # if there is a piece being held, draw it at the mouse position
    if press_square > -1 and get_piece(press_square) > 0:
        screen.blit(pygame.transform.rotate(piece_img[get_piece(press_square)], 0),
                    (mouse_xy[0] - 25, mouse_xy[1] - 25))


def refresh_graphics():
    draw_board()


def get_updated_board():
    global board
    board = [i for i in lib.get_board_state().contents]


def print_move(m):
    print(m.piece, m.start, m.end)


def run_game():
    global board, screen, press_xy, release_xy, press_square, release_square, mouse_xy, move_count
    screen = pygame.display.set_mode((400, 400), 0, 32)
    mainClock = pygame.time.Clock()
    pygame.display.init()
    pygame.display.set_caption('Chess')
    clicking = False
    init_board()

    lib.init(c_char_p(fen), len(fen))

    lib.update_game_possible_moves()
    get_updated_board()
    refresh_graphics()
    press_xy = (-1, -1)
    release_xy = (-1, -1)
    press_square = -1
    release_square = -1
    promo_key = ''

    print([i for i in lib.get_rook_pos().contents])

    while True:
        mouse_xy = pygame.mouse.get_pos()
        for event in pygame.event.get():
            if event.type == QUIT or (event.type == KEYDOWN and event.key == K_ESCAPE):
                pygame.quit()
                sys.exit()
            if event.type == KEYDOWN:
                if event.key == K_SPACE:
                    lib.try_undo_move()
                    lib.update_game_possible_moves()
                    get_updated_board()
                    refresh_graphics()
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
                    if lib.is_game_legal_move(press_square, release_square, promo_num):
                        a = lib.apply_move(press_square, release_square, promo_num)
                        lib.update_game_possible_moves()
                        get_updated_board()
                        refresh_graphics()
                        st = time.time()
                        move_count += 1
                        # eval = lib.calc_eng_move(6)
                        # eval = lib.calc_eng_move_with_test(4, 6)
                        # print("time to engine move", time.time() - st)
                        # print('eval', eval)
                        # print('wc', lib.get_white_check())
                        # print('bc', lib.get_black_check())

                        s = lib.get_eng_move_start()
                        e = lib.get_eng_move_end()
                        id = lib.get_eng_move_id()
                        # a = lib.apply_move(s, e, id)
                        lib.update_game_possible_moves()
                        get_updated_board()
                    else:
                        pass
                        # print('illegal', press_square, release_square, promo_num)
                    press_xy = (-1, -1)
                    release_xy = (-1, -1)
                    press_square = -1
                    release_square = -1
                    refresh_graphics()
            if press_square > -1:
                refresh_graphics()

        pygame.display.update()
        mainClock.tick(100)


def test():
    st = time.time()
    lib.init(c_char_p(fen), len(fen))
    print(lib.detailed_perft(1))
    print(time.time() - st)


run_game()
# test()

# cash_fen depth 4: 1350847 vs stockfish: 1350762
