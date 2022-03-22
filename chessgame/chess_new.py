import math

import numpy as np
from pygame.locals import *
import pygame
import sys

# globle varabuls
board = np.zeros(64, dtype=int)
screen = None
piece_img = []


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


# turns a tuple n=(x, y) into an index from 0-63
def coords_to_num(n):
    return n[1] * 8 + n[0]


# white: P = 1, N = 2, B = 3, R = 4, Q = 5, K = 6
# black: P = 7, N = 8, B = 9, R = 10, Q = 11, K = 12
def init_board():
    global piece_img
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
    print(board)
    print(piece_img)


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
                    if is_legal_move(start, end):
                        print("legal")
                        board[end] = board[start]
                        board[start] = 0
                        draw_board()
                    else:
                        print("illegal")
        pygame.display.update()
        mainClock.tick(100)

# redraws the squares affected by a move
def redraw_squares(start, end):
    pass

# start and end are both integers from 0-63, representing a square on the board
# returns true if this is a legal move
def is_legal_move(start, end):
    piece = board[start]
    diff = end - start
    w_pawn = {7, 8, 9, 16}
    b_pawn = {-7, -8, -9, -16}
    knight = {10, 17, 15, 6, -10, -17, -15, -6}
    bishop = {7, 9}
    rook = {8, -8, 1, -1}
    king = {7, 8, 9, 1, -1, -7, -8, -9}
    if start == end:
        return False
    if piece == 0:
        return False
    elif piece == 1:
        if diff in w_pawn:
            return True
    elif piece == 7:
        if diff in b_pawn:
            return True
    elif piece == 2 or piece == 8:
        if diff in knight:
            return True
    elif piece == 3 or piece == 9:
        for i in bishop:
            if abs(diff) % i == 0:
                return True
    elif piece == 4 or piece == 10:
        if get_rank(start) == get_rank(end) or get_file(start) == get_file(end):
            return True
    elif piece == 5 or piece == 11:
        for i in bishop:
            if abs(diff) % i == 0:
                return True
        if get_rank(start) == get_rank(end) or get_file(start) == get_file(end):
            return True
    elif piece == 6 or piece == 12:
        if diff in king:
            return True
    return False

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


run_game()
