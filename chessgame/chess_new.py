from collections import deque

# TODO
# castling bugs w/ into, out of, through check. black works, white queenside seems off
# reversing bits (done)
# capturing piece movement (done)
# regular piece movement (done)
# en-passant - legal_moves(done)
# castling - legal_moves (rooks can move and still castle)
# promotion - legal_moves (done)
# undo moves (done)
# automated moves depth testing a.k.a. Perft Testing
# run game two-player
# run game engine

import math

import numpy as np
from pygame.locals import *
import pygame
import sys

screen = pygame.display.set_mode((400, 400), 0, 32)
piece_img = []

bitboards = []
move_list = deque()

not_black_pieces = np.int64(0)
not_white_pieces = np.int64(0)
all_squares = np.int64(0)

white_pieces = np.int64(0)
black_pieces = np.int64(0)

empty = np.int64(0)
occupied = np.int64(0)

file = [np.int64(0), np.int64(0), np.int64(0), np.int64(0), np.int64(0), np.int64(0), np.int64(0), np.int64(0),
        np.int64(0)]
rank = [np.int64(0), np.int64(0), np.int64(0), np.int64(0), np.int64(0), np.int64(0), np.int64(0), np.int64(0),
        np.int64(0)]
l_diag = [np.int64(0), np.int64(0), np.int64(0), np.int64(0), np.int64(0), np.int64(0), np.int64(0), np.int64(0),
          np.int64(0), np.int64(0), np.int64(0), np.int64(0), np.int64(0), np.int64(0), np.int64(0)]
r_diag = [np.int64(0), np.int64(0), np.int64(0), np.int64(0), np.int64(0), np.int64(0), np.int64(0), np.int64(0),
          np.int64(0), np.int64(0), np.int64(0), np.int64(0), np.int64(0), np.int64(0), np.int64(0)]

square_a8 = np.int64(0)

knight_span = np.int64(0)
king_span = np.int64(0)

file_ab = np.int64(0)
file_gh = np.int64(0)

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

white_moves = set()
black_moves = set()

white_promo_pieces = list(range(2, 6))
black_promo_pieces = list(range(8, 12))

rook_pos = [7, 0, 63, 56]
rook_num_moves = [0, 0, 0, 0]

king_num_moves = [0, 0]


# given a list of squares to place the pieces, returns a numpy 64-bit integer representing the bitboard
# follows numbering scheme
# e.g. generate_bitboard([0, 2]) = 0000 0000 ... 0000 0101, means there are pieces on squares 0 and 2
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
    global file, rank, square_a8, knight_span, king_span, file_ab, file_gh, all_squares
    all_squares = generate_bitboard(list(range(64)))
    square_a8 = np.left_shift(np.int64(1), 63)

    # assuming knight is on square 18 (f3)
    knight_span = generate_bitboard([1, 8, 24, 33, 35, 28, 12, 3])

    # assuming king is on square 9 (g2)
    king_span = generate_bitboard([0, 1, 2, 8, 10, 16, 17, 18])

    # initialize file masks
    # file[1] is the a-file
    file[1] = generate_bitboard([7, 15, 23, 31, 39, 47, 55, 63])
    for i in range(1, 8):
        file[i + 1] = r_shift(file[i], 1)

    # initialize rank masks
    # rank[1] is the 1st rank
    rank[1] = generate_bitboard([0, 1, 2, 3, 4, 5, 6, 7])
    for i in range(1, 8):
        rank[i + 1] = l_shift(rank[i], 8)

    for i in range(64):
        left = get_l_diag(i)
        right = get_r_diag(i)
        l_diag[left] = np.bitwise_or(np.left_shift(np.int64(1), i), l_diag[left])
        r_diag[right] = np.bitwise_or(np.left_shift(np.int64(1), i), r_diag[right])

    file_ab = np.bitwise_or(file[1], file[2])
    file_gh = np.bitwise_or(file[7], file[8])


# imagine starting at the top left (63), reading left to right, then going to rank below, counting down
# 0=h1=MSB, 63=a8=LSB
def print_numbering_scheme():
    print('Bitboard binary representation:')
    for i in range(63, -1, -1):
        e = '    '
        if i >= 10:
            e = '   '
        if i % 8 == 0:
            e = '\n'
        print(i, end=e)


def print_l_diag():
    print("Left diagonal")
    for i in range(63, -1, -1):
        e = '    '
        if get_l_diag(i) >= 10:
            e = '   '
        if i % 8 == 0:
            e = '\n'
        print(get_l_diag(i), end=e)


def print_r_diag():
    print("Right diagonal")
    for i in range(63, -1, -1):
        e = '    '
        if get_r_diag(i) >= 10:
            e = '   '
        if i % 8 == 0:
            e = '\n'
        print(get_r_diag(i), end=e)


# get what file you are on given an index 0-63
def get_file(n):
    return n % 8


# get what rank you are on given an index 0-63
def get_rank(n):
    return n // 8


# get what right diagonal you are on given an index 0-63
def get_r_diag(n):
    return get_rank(n) + get_file(n)


# get what left diagonal you are on given an index 0-63
def get_l_diag(n):
    return get_rank(n) + 7 - get_file(n)


def get_rank_start(n):
    return n * 8


def get_rank_end(n):
    return (n + 1) * 8


def rank_diff(s, f):
    d = get_rank(f) - get_rank(s)
    return d


def file_diff(s, f):
    d = get_file(f) - get_file(s)
    return d


# turns a tuple n=(x, y) into an index from 0-63
def coords_to_num(n):
    return n[1] * 8 + (7 - n[0])


def l_shift(x, n):
    # if we are shifting by a negative, use right shift instead
    if n <= 0:
        return r_shift(x, abs(n))
    return np.left_shift(x, n)


# np.right_shift appends the sign bit (0's if positive, 1's if negative)
# r_shift returns the right bitwise shift, appending 0's regardless of sign
def r_shift(x, n):
    # if we are shifting by a negative, use left shift instead
    if n <= 0:
        return np.left_shift(x, abs(n))
    # if sign bit is 0, shift normally
    if x > 0:
        return np.right_shift(x, n)
    # if sign bit is 1, right shift once
    y = np.right_shift(x, 1)
    # then set sign bit to 0
    y = np.bitwise_and(y, np.bitwise_not(square_a8))
    # then complete the remaining n-1 right shifts
    y = np.right_shift(y, n - 1)
    return y


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


def reverse(n):
    result = np.int64(0)
    p = n
    for i in range(64):
        result = np.left_shift(result, 1)
        result = np.bitwise_or(result, np.bitwise_and(p, 1))
        p = np.right_shift(p, 1)
    return result


# returns the number of leading zeros in the 64-bit binary representation of i
def leading_zeros(i):
    # 0 has 64 leading zeros
    if i == 0:
        return 64
    # negative numbers have 0 leading zeros (sign bit is 1)
    if i < 0:
        return 0
    n = np.int64(1)
    x = np.right_shift(i, 32)
    if x == 0:
        n += 32
        x = i
    if np.right_shift(x, 16) == 0:
        n += 16
        x = np.left_shift(x, 16)
    if np.right_shift(x, 24) == 0:
        n += 8
        x = np.left_shift(x, 8)
    if np.right_shift(x, 28) == 0:
        n += 4
        x = np.left_shift(x, 4)
    if np.right_shift(x, 30) == 0:
        n += 2
        x = np.left_shift(x, 2)
    n -= np.right_shift(x, 31)
    return n


def add_moves_offset(moves, mask, start_offset, end_offset, move_id=[0], condition=lambda x: True):
    for i in range(64):
        if np.bitwise_and(l_shift(np.int64(1), i), mask):
            if condition(i):
                for j in move_id:
                    moves.add((i + start_offset, i + end_offset, j))


def add_moves_position(moves, mask, start_position, move_id=[0], condition=lambda x: True):
    for i in range(64):
        # if the knight targets this square
        if np.bitwise_and(l_shift(np.int64(1), i), mask):
            if condition(i):
                for j in move_id:
                    moves.add((start_position, i, j))


# given the span mask representing the squares that a piece at origin could move to
# returns the squares that could be reached by a piece on square i
# for use with knight and king, which are defined by spans with origins 18 and 9
def offset_span(span, origin, i):
    if i > origin:
        return l_shift(span, i - origin)
    return r_shift(span, origin - i)


# removes bits in an offset span that would cause warps between the ab and gh files
# for use with knight and king
def remove_span_warps(span, i):
    # piece on the right half of the board
    if i % 8 < 4:
        return multi_and([span, np.bitwise_not(file_ab)])
    return multi_and([span, np.bitwise_not(file_gh)])


def span_piece(mask, i, span, origin):

    squares = offset_span(span, origin, i)
    squares = remove_span_warps(squares, i)
    squares = multi_and([squares, mask])
    return squares

def span_piece_unsafe(moves, mask, i, span, origin):

    squares = offset_span(span, origin, i)
    squares = remove_span_warps(squares, i)
    squares = multi_and([squares, mask])
    add_moves_position(moves, squares, i)

def sliding_piece(moves, mask, i, is_white, rook_moves=False, bishop_moves=False):

    directions = set()
    if rook_moves:
        directions.add(rank[get_rank(i) + 1])
        directions.add(file[8 - get_file(i)])
    if bishop_moves:
        directions.add(l_diag[get_l_diag(i)])
        directions.add(r_diag[get_r_diag(i)])
    for d in directions:
        slider = np.left_shift(np.int64(1), i)
        squares = np.bitwise_and(line_attack(occupied, d, slider), mask)
        add_moves_position(moves, squares, i)

# calls on_find_one function every time a 1 is found and condition is true for that square
# TODO leading_zeros optimization so we don't need to iterate through all 64 squares
def bitwise_for(mask, on_find_one=lambda x: True, condition=lambda x: True):
    for i in range(64):
        if np.bitwise_and(l_shift(np.int64(1), i), mask) and condition(i):
            on_find_one(i)


def possible_wP(bb, moves):
    possible_P(bb, moves, black_pieces, rank[8], bitboards[7], white_promo_pieces, rank[4], 1, True)


def possible_bP(bb, moves):
    possible_P(bb, moves, white_pieces, rank[1], bitboards[1], black_promo_pieces, rank[5], -1, False)


def possible_P(bb, moves, can_capture, promo_rank, enemy_pawns, promo_pieces, double_push_rank, fwd, is_white):

    # start square, end square, and move id of the previous move
    s, e, m, c = (0, 0, 0, 0)
    if move_list:
        s, e, m, c = move_list[-1]

    not_promo_rank = np.bitwise_not(promo_rank)

    # capture right
    mask = multi_and([l_shift(bb, fwd * 8 - 1), can_capture, not_promo_rank, np.bitwise_not(file[1])])
    add_moves_offset(moves, mask, -(fwd * 8 - 1), 0)

    # capture left
    mask = multi_and([l_shift(bb, fwd * 8 + 1), can_capture, not_promo_rank, np.bitwise_not(file[8])])
    add_moves_offset(moves, mask, -(fwd * 8 + 1), 0)

    # one forward
    mask = multi_and([l_shift(bb, fwd * 8), empty, not_promo_rank])
    add_moves_offset(moves, mask, -fwd * 8, 0)

    # two forward
    mask = multi_and([l_shift(bb, 2 * fwd * 8), empty, l_shift(empty, fwd * 8), double_push_rank])
    add_moves_offset(moves, mask, -(2 * fwd * 8), 0)

    # promotion by capture right
    mask = multi_and([l_shift(bb, fwd * 8 - 1), can_capture, promo_rank, np.bitwise_not(file[1])])
    add_moves_offset(moves, mask, -(fwd * 8 - 1), 0, move_id=promo_pieces)

    # promotion by capture left
    mask = multi_and([l_shift(bb, fwd * 8 + 1), can_capture, promo_rank, np.bitwise_not(file[8])])
    add_moves_offset(moves, mask, -(fwd * 8 + 1), 0, move_id=promo_pieces)

    # promotion by one forward
    mask = multi_and([l_shift(bb, fwd * 8), empty, promo_rank])
    add_moves_offset(moves, mask, -fwd * 8, 0, move_id=promo_pieces)

    # if the previous move was a double pawn push, en passant might be possible
    if m == 13:
        cond = lambda x: x == e

        # left en passant
        mask = multi_and([l_shift(bb, 1), enemy_pawns, not_promo_rank, np.bitwise_not(file[8])])
        add_moves_offset(moves, mask, -1, fwd * 8, condition=cond)

        # right en passant
        mask = multi_and([l_shift(bb, -1), enemy_pawns, not_promo_rank, np.bitwise_not(file[1])])
        add_moves_offset(moves, mask, 1, fwd * 8, condition=cond)

def possible_N(bb, moves, mask, is_white):
    bitwise_for(bb, lambda i: add_moves_position(moves, span_piece(mask, i, knight_span, 18), i))

def possible_B(bb, moves, mask, is_white):
    bitwise_for(bb, lambda i: sliding_piece(moves, mask, i, is_white, bishop_moves=True))


def possible_R(bb, moves, mask, is_white):
    bitwise_for(bb, lambda i: sliding_piece(moves, mask, i, is_white, rook_moves=True))


def possible_Q(bb, moves, mask, is_white):
    bitwise_for(bb, lambda i: sliding_piece(moves, mask, i, is_white, rook_moves=True, bishop_moves=True))


def possible_K(bb, moves, mask, is_white):
    bitwise_for(bb, lambda i: add_moves_position(moves, span_piece(mask, i, king_span, 9), i))
    safe = np.bitwise_not(unsafe_for_white())
    if not is_white:
        safe = np.bitwise_not(unsafe_for_black())
    empty_and_safe = multi_and([empty, safe])

    # this is white king, hasn't moved yet, isn't trying to castle out of check
    if is_white and king_num_moves[0] == 0 and multi_and([bb, safe]):
        # white queenside castle
        squares = multi_and([l_shift(bb, 2), l_shift(empty_and_safe, 1), empty_and_safe, l_shift(empty, -1)])
        # print_bitboard(l_shift(bb, 2))
        # print_bitboard(l_shift(empty, 1))
        # print_bitboard(empty)
        #
        add_moves_offset(moves, squares, -2, 0)

        # white kingside castle
        squares = multi_and([l_shift(bb, -2), l_shift(empty_and_safe, -1), empty_and_safe])
        add_moves_offset(moves, squares, 2, 0)

    # this is black king, hasn't moved yet, isn't trying to castle out of check
    elif not is_white and king_num_moves[1] == 0 and multi_and([bb, safe]):
        # black queenside castle
        squares = multi_and([l_shift(bb, 2), l_shift(empty_and_safe, 1), empty_and_safe, l_shift(empty, -1)])

        add_moves_offset(moves, squares, -2, 0)

        # black kingside castle
        squares = multi_and([l_shift(bb, -2), l_shift(empty_and_safe, -1), empty_and_safe])
        add_moves_offset(moves, squares, 2, 0)


def possible_moves_white():
    global white_moves, not_white_pieces, black_pieces, empty, occupied
    b = bitboards
    not_white_pieces = np.bitwise_not(multi_or(b[1:7] + b[12]))
    black_pieces = multi_or(b[7:12])
    empty = np.bitwise_not(multi_or(b[1:13]))
    occupied = np.bitwise_not(empty)
    white_moves = set()
    possible_wP(bitboards[1], white_moves)
    possible_N(bitboards[2], white_moves, not_white_pieces, True)
    possible_B(bitboards[3], white_moves, not_white_pieces, True)
    possible_R(bitboards[4], white_moves, not_white_pieces, True)
    possible_Q(bitboards[5], white_moves, not_white_pieces, True)
    possible_K(bitboards[6], white_moves, not_white_pieces, True)


def possible_moves_black():
    global black_moves, not_black_pieces, white_pieces, empty, occupied
    b = bitboards
    not_black_pieces = np.bitwise_not(multi_or(b[7:13] + b[6]))
    white_pieces = multi_or(b[1:6])
    empty = np.bitwise_not(multi_or(b[1:13]))
    occupied = np.bitwise_not(empty)
    black_moves = set()
    possible_bP(bitboards[7], black_moves)
    possible_N(bitboards[8], black_moves, not_black_pieces, False)
    possible_B(bitboards[9], black_moves, not_black_pieces, False)
    possible_R(bitboards[10], black_moves, not_black_pieces, False)
    possible_Q(bitboards[11], black_moves, not_black_pieces, False)
    possible_K(bitboards[12], black_moves, not_black_pieces, False)
# this is for the kings
def unsafe_for_white():
    unsafe = np.int64(0)
    # pawns
    # threaten to capture right
    mask = multi_and([l_shift(bitboards[7], -8 - 1), np.bitwise_not(file[1])])

    unsafe = np.bitwise_or(unsafe, mask)

    # threaten to capture left
    mask = multi_and([l_shift(bitboards[7], -8 + 1), np.bitwise_not(file[8])])

    unsafe = np.bitwise_or(unsafe, mask)
    # knight
    for i in range(64):
        if np.bitwise_and(l_shift(np.int64(1), i), bitboards[8]):
            unsafe = np.bitwise_or(unsafe, span_piece(all_squares, i, knight_span, 18))
    return unsafe



    # # siding pieces
    # a = line_attack(np.bitwise_and(occupied, np.bitwise_not(bitboards[6])), d, slider)
    # unsafe_for_white = multi_or([unsafe_for_white, squares, a])


def unsafe_for_black():
    unsafe = np.int64(0)
    # pawns
    # threaten to capture right
    mask = multi_and([l_shift(bitboards[1], 8 - 1), np.bitwise_not(file[1])])

    unsafe = np.bitwise_or(unsafe, mask)

    # threaten to capture left
    mask = multi_and([l_shift(bitboards[1], 8 + 1), np.bitwise_not(file[8])])

    unsafe = np.bitwise_or(unsafe, mask)

    return unsafe

def print_bitboard(x):
    b = np.binary_repr(x, 64)
    for i in range(0, len(b), 8):
        print(b[i:i + 8])


# returns the bitboard representing squares threatened by a sliding piece along a certain axis
# o - occupied: the bitboard representing squares that have a piece on them
# m - mask: the rank, file, or diagonal bitboard where the piece is active
# s - slider: the bitboard containing the sliding piece
def line_attack(o, m, s):
    o_and_m = np.bitwise_and(o, m)  # o&m
    rev_o_and_m = reverse(o_and_m)  # (o&m)'

    two_s = np.multiply(np.int64(2), s)  # 2s
    rev_two_s = np.multiply(np.int64(2), reverse(s))  # 2s'

    left = np.subtract(o_and_m, two_s)  # left = (o&m)-2s
    right = reverse(np.subtract(rev_o_and_m, rev_two_s))  # right = ((o&m)'-2s')'

    left_xor_right = np.bitwise_xor(left, right)  # left^right = ((o&m)-2s)^((o&m)'-2s')'
    ans = np.bitwise_and(left_xor_right, m)  # (left^right)&m = (((o&m)-2s)^((o&m)'-2s')')&m
    return ans


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


# this is a list of move_id
# 0 is just a normal move or a capture
# 1 to 12 is for move promotion
# 13 is for double pawn push
# 14 is en_passant
# 15 is castling
def apply_move(start, end, move_id):
    moved_piece = get_piece(start)
    captured_piece = get_piece(end)
    remove_piece(moved_piece, start)
    # if captured_piece:
    #     print(start, end, -captured_piece)
    # else:
    #     print(start, end, move_id)
    # is to check if rooks have moved
    if moved_piece == 4 or moved_piece == 10:
        for i in range(4):
            if start == rook_pos[i]:
                rook_pos[i] = end
                rook_num_moves[i] += 1
    # this is to check for castling rights
    if moved_piece == 6:
        # this is castling king side
        if end - start == -2:
            remove_piece(4, rook_pos[1])
            add_piece(4, 2)
            rook_pos[1] = 2
            move_list.append((start, end, 15, 0))
        if end - start == 2:
            remove_piece(4, rook_pos[0])
            add_piece(4, 4)
            rook_pos[0] = 4
            move_list.append((start, end, 15, 0))
        king_num_moves[0] += 1

    elif moved_piece == 12:
        if end - start == -2:
            remove_piece(10, rook_pos[3])
            add_piece(10, 58)
            rook_pos[3] = 58
            move_list.append((start, end, 15, 0))
        if end - start == 2:
            remove_piece(10, rook_pos[2])
            add_piece(10, 60)
            rook_pos[2] = 60
            move_list.append((start, end, 15, 0))
        king_num_moves[1] += 1

    if captured_piece:
        remove_piece(captured_piece, end)
    if move_id:
        add_piece(move_id, end)
    else:
        add_piece(moved_piece, end)
        # is a pawn
    if (moved_piece == 1 or moved_piece == 7) and abs(end - start) == 16:
        # double pawn push
        move_list.append((start, end, 13, 0))
    # if the move was castling
    elif (moved_piece == 6 or moved_piece == 12) and abs(end - start) == 2:
        pass
    else:
        # previous move start, end, and move_id
        if move_list:
            s, e, m, c = move_list[-1]
            ep_pawn = get_piece(e)  # pawn that was captured en passant
            # white capturing en passant
            if m == 13 and moved_piece == 1 and ep_pawn == 7 and end - e == 8:
                remove_piece(ep_pawn, e)
                move_list.append((start, end, 14, 0))
            # black capturing en passant
            elif m == 13 and moved_piece == 7 and ep_pawn == 1 and end - e == -8:
                remove_piece(ep_pawn, e)
                move_list.append((start, end, 14, 0))
            else:
                move_list.append((start, end, move_id, captured_piece))
        else:
            move_list.append((start, end, move_id, captured_piece))
    print(king_num_moves)


def undo_move():
    if not move_list:
        return
    start, end, move_id, capture = move_list.pop()
    moved_piece = get_piece(end)
    is_white = is_white_piece(moved_piece)
    remove_piece(moved_piece, end)
    add_piece(moved_piece, start)
    # last move was a capture
    if capture:
        add_piece(capture, end)
    if move_id == 0:
        pass
    # last move was pawn promotion
    elif 1 <= move_id <= 12:
        remove_piece(move_id, start)
        if is_white:
            add_piece(1, start)
        else:
            add_piece(7, start)
    # last move was double pawn push
    elif move_id == 13:
        pass
    # last move was en passant
    elif move_id == 14:
        if is_white:
            add_piece(7, end - 8)
        else:
            add_piece(1, end + 8)
    # last move was castling
    elif move_id == 15:
        # white king
        if moved_piece == 6:
            if end - start == -2:
                remove_piece(4, rook_pos[1])
                add_piece(4, 0)
                rook_pos[1] = 0
            if end - start == 2:
                remove_piece(4, rook_pos[0])
                add_piece(4, 7)
                rook_pos[0] = 7
        # black king
        elif moved_piece == 12:
            if end - start == -2:
                remove_piece(10, rook_pos[3])
                add_piece(10, 56)
                rook_pos[3] = 56
            if end - start == 2:
                remove_piece(10, rook_pos[2])
                add_piece(10, 63)
                rook_pos[2] = 63
    if moved_piece == 4 or moved_piece == 10:
        for i in range(4):
            if end == rook_pos[i]:
                rook_pos[i] = start
                rook_num_moves[i] -= 1
    elif moved_piece == 6:
        king_num_moves[0] -= 1
    elif moved_piece == 12:
        king_num_moves[1] -= 1


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


def refresh_graphics():
    draw_board()
    # draw_possible_moves(white_moves, GREEN, 6)
    # draw_possible_moves(black_moves, RED, 2)
    draw_bitboard(unsafe_for_black(), GREEN, 7)
    draw_bitboard(unsafe_for_white(), RED, 4)


def run_game():
    global press_xy, release_xy, press_square, release_square, mouse_xy
    mainClock = pygame.time.Clock()
    pygame.display.init()
    pygame.display.set_caption('Chess')
    clicking = False
    init_board()
    possible_moves_white()
    possible_moves_black()
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
                    undo_move()
                    possible_moves_white()
                    possible_moves_black()
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
                    if is_legal_move(press_square, release_square, promo_num):
                        apply_move(press_square, release_square, promo_num)
                        possible_moves_white()
                        possible_moves_black()
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


def draw_board():
    for i in range(64):
        if (get_file(i) + get_rank(i)) % 2 == 1:
            square_color = BLUE
        else:
            square_color = WHITE
        pygame.draw.rect(screen, square_color, (350 - (i % 8) * 50, 350 - (i // 8) * 50, 50, 50))
        if i == press_square:
            continue
        for b in range(1, len(bitboards)):
            if np.bitwise_and(np.left_shift(np.int64(1), i), bitboards[b]):
                screen.blit(pygame.transform.rotate(piece_img[b], 0), (350 - (i % 8) * 50, 350 - (i // 8) * 50))
    if press_square > -1 and get_piece(press_square) > 0:
        screen.blit(pygame.transform.rotate(piece_img[get_piece(press_square)], 0),
                    (mouse_xy[0] - 25, mouse_xy[1] - 25))


def draw_bitboard(bitboard, color, radius):
    for i in range(64):
        if np.bitwise_and(np.left_shift(np.int64(1), i), bitboard):
            pygame.draw.circle(screen, color, (350 - (i % 8) * 50 + 25, 350 - (i // 8) * 50 + 25), radius)


def draw_possible_moves(moves, color, line_width):
    for i in moves:
        pygame.draw.line(screen, color, (350 - (i[0] % 8) * 50 + 25, 350 - (i[0] // 8) * 50 + 25),
                         (350 - (i[1] % 8) * 50 + 25, 350 - (i[1] // 8) * 50 + 25), width=line_width)
        # pygame.draw.circle(screen, color, (350 - (i[1] % 8) * 50 + 25, 350 - (i[1] // 8) * 50 + 25), 5)


run_game()
