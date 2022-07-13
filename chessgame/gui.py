import ctypes
import math
import sys
import time

import pygame
from pygame.locals import *

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

lib = ctypes.CDLL('./chess_game.so')

lib.apply_move.restype = ctypes.c_bool

lib.try_undo_move.restype = ctypes.c_bool

lib.is_game_legal_move.restype = ctypes.c_bool
lib.is_game_legal_move.argtypes = [ctypes.c_int, ctypes.c_int, ctypes.c_int]

lib.get_board_state.restype = ctypes.POINTER(ctypes.c_int * 64)

lib.get_white_check.restype = ctypes.c_bool

lib.get_black_check.restype = ctypes.c_bool

lib.perft_test.restype = ctypes.c_int
lib.perft_test.argtypes = [ctypes.c_int]

lib.calc_eng_move.argtypes = [ctypes.c_int]

lib.get_eng_move_start.restype = ctypes.c_int
lib.get_eng_move_end.restype = ctypes.c_int
lib.get_eng_move_eval.restype = ctypes.c_int
lib.get_eng_move_id.restype = ctypes.c_int

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


def run_game():
    global board, screen, press_xy, release_xy, press_square, release_square, mouse_xy
    screen = pygame.display.set_mode((400, 400), 0, 32)
    mainClock = pygame.time.Clock()
    pygame.display.init()
    pygame.display.set_caption('Chess')
    clicking = False
    init_board()
    lib.init()
    lib.update_game_possible_moves()
    get_updated_board()
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
                        # print(lib.perft_test(1))
                        board = [i for i in lib.get_board_state().contents]
                        # print(white_in_checkmate(), black_in_checkmate())
                        refresh_graphics()
                        lib.calc_eng_move(4)
                        s = lib.get_eng_move_start()
                        e = lib.get_eng_move_end()
                        id = lib.get_eng_move_id()
                        eval = lib.get_eng_move_eval()
                        a = lib.apply_move(s, e, id)

                        lib.update_game_possible_moves()
                        # print(lib.perft_test(1))
                        board = [i for i in lib.get_board_state().contents]
                    else:
                        print('illegal', press_square, release_square, promo_num)
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
    lib.init()
    print(lib.perft_test(6))
    print(time.time() - st)


run_game()
