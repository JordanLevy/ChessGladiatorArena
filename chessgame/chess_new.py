from collections import deque

# TODO
# reversing bits (done)
# regular piece movement
# en-passant - legal_moves
# castling - legal_moves
# promotion - legal_moves
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

wP = np.int64(0)
wN = np.int64(0)
wB = np.int64(0)
wR = np.int64(0)
wQ = np.int64(0)
wK = np.int64(0)

bP = np.int64(0)
bN = np.int64(0)
bB = np.int64(0)
bR = np.int64(0)
bQ = np.int64(0)
bK = np.int64(0)

w_threats = np.int64(0)
b_threats = np.int64(0)

not_white_pieces = np.int64(0)
black_pieces = np.int64(0)
empty = np.int64(0)

file_a = np.int64(0)
file_h = np.int64(0)

rank_1 = np.int64(0)
rank_4 = np.int64(0)
rank_8 = np.int64(0)

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
    global wP, wN, wB, wR, wQ, wK, bP, bN, bB, bR, bQ, bK
    wP = generate_bitboard(list(range(8, 16)))
    wN = generate_bitboard([1, 6])
    wB = generate_bitboard([2, 5])
    wR = generate_bitboard([0, 7])
    wQ = generate_bitboard([4])
    wK = generate_bitboard([3])

    diff = 8 * 5  # 5 ranks between white and black's pawn ranks
    bP = np.left_shift(wP, diff)

    diff = 8 * 7  # 7 ranks between white and black's back ranks
    bN = np.left_shift(wN, diff)
    bB = np.left_shift(wB, diff)
    bR = np.left_shift(wR, diff)
    bQ = np.left_shift(wQ, diff)
    bK = np.left_shift(wK, diff)

def init_masks():
    global file_a, file_h, rank_1, rank_4, rank_8
    file_a = generate_bitboard([7, 15, 23, 31, 39, 47, 55, 63])
    file_h = generate_bitboard([0, 8, 16, 24, 32, 40, 48, 56])

    rank_1 = generate_bitboard([0, 1, 2, 3, 4, 5, 6, 7])
    rank_4 = generate_bitboard([24, 25, 26, 27, 28, 29, 30, 31])
    rank_8 = generate_bitboard([56, 57, 58, 59, 60, 61, 62, 63])

# imagine starting at the top left (63), reading left to right, then going to rank below, counting down
# 0 is h1, 63 is a8
def print_numbering_scheme():
    print('Bitboard binary representation:')
    for i in range(63, -1, -1):
        print(i, end=' ')
    print()
    print('----------------')
    for i in range(63, -1, -1):
        e = '    '
        if i >= 10:
            e = '   '
        if i % 8 == 0:
            e = '\n'
        print(i, end=e)


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
    for i in range(0, 32//num_ones):
        a = np.bitwise_or(a, np.left_shift(base, i * (2*num_ones)))
    return a

def reverse_chunk(x, m):
    mask = get_reverse_mask(m)
    return np.bitwise_or(np.right_shift(np.bitwise_and(x, np.bitwise_not(mask)), m), np.left_shift(np.bitwise_and(x, mask), m))

def reverse_bits(x):
    r = reverse_chunk(x, 1)
    r = reverse_chunk(r, 2)
    r = reverse_chunk(r, 4)
    r = reverse_chunk(r, 8)
    r = reverse_chunk(r, 16)
    r = np.bitwise_or(np.right_shift(r, 32), np.left_shift(r, 32))
    return r

def possible_pW():
    moves = set()
    # capture right
    pawnMoves = multi_and([np.left_shift(wP, 7), black_pieces, np.bitwise_not(rank_8), np.bitwise_not(file_a)])
    for i in range(64):
        if np.bitwise_and(np.left_shift(np.int64(1), i), pawnMoves):
            moves.add((i - 7, i))
    # capture left
    pawnMoves = multi_and([np.left_shift(wP, 9), black_pieces, np.bitwise_not(rank_8), np.bitwise_not(file_h)])
    for i in range(64):
        if np.bitwise_and(np.left_shift(np.int64(1), i), pawnMoves):
            moves.add((i - 9, i))
    # one forward
    pawnMoves = multi_and([np.left_shift(wP, 8), empty, np.bitwise_not(rank_8)])
    for i in range(64):
        if np.bitwise_and(np.left_shift(np.int64(1), i), pawnMoves):
            moves.add((i - 8, i))
    # two forward
    pawnMoves = multi_and([np.left_shift(wP, 16), empty, np.left_shift(empty, 8), rank_4])
    for i in range(64):
        if np.bitwise_and(np.left_shift(np.int64(1), i), pawnMoves):
            moves.add((i - 16, i))
    return moves

def possible_moves_white():
    global white_moves, not_white_pieces, black_pieces, empty
    not_white_pieces = np.bitwise_not(multi_or([wP, wN, wB, wR, wQ, wK, bK]))
    black_pieces = multi_or([bP, bN, bB, bR, bQ])
    empty = np.bitwise_not(multi_or([wP, wN, wB, wR, wQ, wK, bP, bN, bB, bR, bQ, bK]))
    white_moves = possible_pW()


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


def is_legal_move(start, end):
    return (start, end) in white_moves

def apply_move(start, end):
    global wP, wN, wB, wR, wQ, wK, bP, bN, bB, bR, bQ, bK
    remove_mask = np.bitwise_not(np.left_shift(np.int64(1), start))
    add_mask = np.left_shift(np.int64(1), end)
    wP = np.bitwise_and(wP, remove_mask)
    wP = np.bitwise_or(wP, add_mask)
    print(start, end)

def run_game():
    global screen
    mainClock = pygame.time.Clock()
    pygame.init()
    pygame.display.set_caption('Chess')
    screen = pygame.display.set_mode((400, 400), 0, 32)
    clicking = False
    init_board()
    draw_board()
    possible_moves_white()
    draw_possible_moves(white_moves, GREEN)

    a = np.int64(82374)
    print(np.binary_repr(a, 64))
    print(np.binary_repr(reverse_bits(a), 64))

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
                        apply_move(start, end)
                        draw_board()
                        possible_moves_white()
                        draw_possible_moves(white_moves, GREEN)
                    #
                    # # a move hase been made
                    # if is_legal_move(start, end):
                    #     apply_move(start, end)
                    #     draw_board()
                    # else:
                    #     print("illegal")
        pygame.display.update()
        mainClock.tick(100)


def draw_board():
    for i in range(64):
        if (get_file(i) + get_rank(i)) % 2 == 0:
            square_color = BLUE
        else:
            square_color = WHITE
        pygame.draw.rect(screen, square_color, (350 - (i % 8) * 50, 350 - (i // 8) * 50, 50, 50))

        bitboards = [wP, wN, wB, wR, wQ, wK, bP, bN, bB, bR, bQ, bK]
        for b in range(len(bitboards)):
            if np.bitwise_and(np.left_shift(np.int64(1), i), bitboards[b]):
                screen.blit(pygame.transform.rotate(piece_img[b + 1], 0), (350 - (i % 8) * 50, 350 - (i // 8) * 50))

        draw_bitboard(w_threats, YELLOW)
        draw_bitboard(b_threats, GREY)
        # p = board[i]
        # if p > 0:
        #     screen.blit(pygame.transform.rotate(piece_img[p], 0), (get_file(i) * 50, 350 - get_rank(i) * 50))

def draw_bitboard(bitboard, color):
    for i in range(64):
        if np.bitwise_and(np.left_shift(np.int64(1), i), bitboard):
            pygame.draw.circle(screen, color, (350 - (i % 8) * 50 + 25, 350 - (i // 8) * 50 + 25), 5)

def draw_possible_moves(moves, color):
    for i in moves:
        pygame.draw.line(screen, color, (350 - (i[0] % 8) * 50 + 25, 350 - (i[0] // 8) * 50 + 25), (350 - (i[1] % 8) * 50 + 25, 350 - (i[1] // 8) * 50 + 25))
        pygame.draw.circle(screen, color, (350 - (i[1] % 8) * 50 + 25, 350 - (i[1] // 8) * 50 + 25), 5)

run_game()
