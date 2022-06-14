from collections import deque

# TODO
# speed optimizations
# automated moves depth testing a.k.a. Perft Testing
# refactoring
# main.py
# moves.py
# utils.py
# FEN
# run game two-player
# run game engine

# DONE
# en-passant capture bug allows self-check (done)
# pinned pieces/making moves that put you into check (done)
# checkmate (done)
# turns (done)
# replace bitwise_for() with a generator function (done)
# color the king square for check (done)
# castling bugs w/ into, out of, through check. black works (done)
# white queenside seems off(done)
# reversing bits (done)
# capturing piece movement (done)
# regular piece movement (done)
# en-passant - legal_moves(done)
# castling - legal_moves (done)
# promotion - legal_moves (done)
# undo moves (done)

import math
import time
import numpy as np
from pygame.locals import *
import pygame
import sys

screen = None
piece_img = []

num_moves = 0
white_turn = True

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

unsafe_white = np.int64(0)
unsafe_black = np.int64(0)

white_check = False
black_check = False

blocking_squares = np.int64(0)
pinning_squares = dict()
en_passant_pinned = -1

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


# returns false if this move would put your own king in check
# returns true otherwise
def resolves_check(start, end, move_id):
    moved_piece = get_piece(start)
    if white_turn:
        # if the piece being moved is pinned, make sure it doesn't reveal a check
        if start in pinning_squares:
            pinning_line = pinning_squares[start]
            if not np.bitwise_and(np.left_shift(np.int64(1), end), pinning_line):
                return False
        # trying to capture en passant but en passant is pinned
        if moved_piece == 1 and end == en_passant_pinned:
            return False
        # if it's a king move
        if moved_piece == 6:
            # king cannot move to an unsafe square
            if np.bitwise_and(np.left_shift(np.int64(1), end), unsafe_white):
                return False
        # if it's not a king move and white is in check
        elif white_check:
            # the move must either block check or capture the piece that's delivering check
            if not np.bitwise_and(np.left_shift(np.int64(1), end), blocking_squares):
                return False
    else:
        if start in pinning_squares:
            pinning_line = pinning_squares[start]
            if not np.bitwise_and(np.left_shift(np.int64(1), end), pinning_line):
                return False
        # trying to capture en passant but en passant is pinned
        if moved_piece == 7 and end == en_passant_pinned:
            return False
        if moved_piece == 12:
            if np.bitwise_and(np.left_shift(np.int64(1), end), unsafe_black):
                return False
        elif black_check:
            if not np.bitwise_and(np.left_shift(np.int64(1), end), blocking_squares):
                return False
    # if this piece is pinned and moves out of line, return false
    # if this piece blocks or captures the piece delivering check, return true
    return True


def add_moves_offset(moves, mask, start_offset, end_offset, move_id=[0], condition=lambda x: True):
    for i in find_ones(mask):
        if condition(i):
            for j in move_id:
                if resolves_check(i + start_offset, i + end_offset, j):
                    moves.add((i + start_offset, i + end_offset, j))


def add_moves_position(moves, mask, start_position, move_id=[0], condition=lambda x: True):
    for i in find_ones(mask):
        if condition(i):
            for j in move_id:
                if resolves_check(start_position, i, j):
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


def span_piece(mask, i, span, origin, king_bb=np.int64(0)):
    global blocking_squares
    squares = offset_span(span, origin, i)
    squares = remove_span_warps(squares, i)
    squares = multi_and([squares, mask])
    # if this piece is delivering check in this direction
    if np.bitwise_and(squares, king_bb):
        # allow the checking piece to be blocked or captured
        blocking_squares = np.left_shift(np.int64(1), i)
    return squares


def sliding_piece(mask, i, blockers, rook_moves=False, bishop_moves=False, king_bb=np.int64(0)):
    global blocking_squares, pinning_squares, en_passant_pinned
    squares = np.int64(0)
    slider = np.left_shift(np.int64(1), i)
    # int representing which square index the king is on
    king_square = 64 - leading_zeros(king_bb) - 1
    king_color = is_white_piece(get_piece(king_square))

    directions = set()
    if rook_moves:
        directions.add(rank[get_rank(i) + 1])
        directions.add(file[8 - get_file(i)])
    if bishop_moves:
        directions.add(l_diag[get_l_diag(i)])
        directions.add(r_diag[get_r_diag(i)])

    for d in directions:
        new_squares = np.bitwise_and(line_attack(blockers, d, slider), mask)
        squares = np.bitwise_or(new_squares, squares)
        # if this piece is delivering check in this direction
        if np.bitwise_and(new_squares, king_bb):
            # allow the checking piece to be blocked or captured
            blocking_squares = line_between_pieces(d, i, king_square)
            # slider can also be captured to "block" check
            blocking_squares = np.bitwise_or(blocking_squares, slider)
        # this is for pined pieces
        elif king_square >= 0:
            king_line = line_between_pieces(d, i, king_square)
            if not king_line:
                continue
            pos_pin = np.bitwise_and(blockers, king_line)
            counter = 0
            pin_loc = [-1, -1]
            for i in find_ones(pos_pin):
                counter += 1
                if counter > 2:
                    break
                pin_loc[counter - 1] = i

            if counter == 1:
                pinned_piece_color = is_white_piece(get_piece(pin_loc[0]))
                # print(pinned_piece_color, king_color, pin_loc, king_square)
                if pinned_piece_color == king_color:
                    # print(pin_loc)
                    king_line = np.bitwise_or(king_line, slider)
                    pinning_squares[pin_loc[0]] = king_line
            elif counter == 2:
                p1 = get_piece(pin_loc[0])
                p2 = get_piece(pin_loc[1])
                if (p1 == 1 and p2 == 7) or (p1 == 7 and p2 == 1):
                    s, e, m, c = move_list[-1]
                    # last move was double pawn push
                    if m == 13:
                        if is_white_piece(get_piece(e)):
                            en_passant_pinned = e - 8
                        else:
                            en_passant_pinned = e + 8

    return squares


# direction: bitboard representing the file/rank/diagonal that the pieces both occupy
# piece_1: int representing the location of the 1st piece
# piece_2: int representing the location of the 2nd piece
# returns a bitboard representing squares in a straight line between the two pieces, ignoring other pieces on the board
def line_between_pieces(direction, piece_1, piece_2):
    if not np.bitwise_and(direction, l_shift(np.int64(1), piece_1)):
        return np.int64(0)
    if not np.bitwise_and(direction, l_shift(np.int64(1), piece_2)):
        return np.int64(0)
    if piece_1 < piece_2:
        mask = generate_bitboard(list(range(piece_1 + 1, piece_2)))
    else:
        mask = generate_bitboard(list(range(piece_2 + 1, piece_1)))
    return np.bitwise_and(direction, mask)


# generator function that yields the indices of ones in a bitboard bb
def find_ones(bb):
    for i in range(64):
        if np.bitwise_and(l_shift(np.int64(1), i), bb):
            yield i


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
    for i in find_ones(bb):
        add_moves_position(moves, span_piece(mask, i, knight_span, 18), i)


def possible_B(bb, moves, mask, is_white):
    for i in find_ones(bb):
        add_moves_position(moves, sliding_piece(mask, i, occupied, rook_moves=False, bishop_moves=True), i)


def possible_R(bb, moves, mask, is_white):
    for i in find_ones(bb):
        add_moves_position(moves, sliding_piece(mask, i, occupied, rook_moves=True, bishop_moves=False), i)


def possible_Q(bb, moves, mask, is_white):
    for i in find_ones(bb):
        add_moves_position(moves, sliding_piece(mask, i, occupied, rook_moves=True, bishop_moves=True), i)


def possible_K(bb, moves, mask, is_white):
    safe = np.bitwise_not(unsafe_white)
    if not is_white:
        safe = np.bitwise_not(unsafe_black)
    empty_and_safe = multi_and([empty, safe])
    for i in find_ones(bb):
        add_moves_position(moves, span_piece(np.bitwise_and(mask, safe), i, king_span, 9), i)
    # if the king is in check, king cannot castle
    if not multi_and([bb, safe]):
        return
    # this is white king, hasn't moved yet
    if is_white and king_num_moves[0] == 0:
        # white queenside castle
        if rook_num_moves[0] == 0:
            squares = multi_and([l_shift(bb, 2), l_shift(empty_and_safe, 1), empty_and_safe, l_shift(empty, -1)])
            add_moves_offset(moves, squares, -2, 0)
        # white kingside castle
        if rook_num_moves[1] == 0:
            squares = multi_and([l_shift(bb, -2), l_shift(empty_and_safe, -1), empty_and_safe])
            add_moves_offset(moves, squares, 2, 0)
    # this is black king, hasn't moved yet
    elif not is_white and king_num_moves[1] == 0:
        # black queenside castle
        if rook_num_moves[2] == 0:
            squares = multi_and([l_shift(bb, 2), l_shift(empty_and_safe, 1), empty_and_safe, l_shift(empty, -1)])
            add_moves_offset(moves, squares, -2, 0)
        # black kingside castle
        if rook_num_moves[3] == 0:
            squares = multi_and([l_shift(bb, -2), l_shift(empty_and_safe, -1), empty_and_safe])
            add_moves_offset(moves, squares, 2, 0)


def update_unsafe():
    global unsafe_white, unsafe_black, white_check, black_check
    unsafe_white = unsafe_for_white()
    white_check = white_in_check()
    unsafe_black = unsafe_for_black()
    black_check = black_in_check()


def possible_moves_white():
    global white_moves
    update_piece_masks()
    st = time.time()
    update_unsafe()
    print('unsafe', time.time() - st)
    st = time.time()
    white_moves = set()
    possible_wP(bitboards[1], white_moves)
    possible_N(bitboards[2], white_moves, not_white_pieces, True)
    possible_B(bitboards[3], white_moves, not_white_pieces, True)
    possible_R(bitboards[4], white_moves, not_white_pieces, True)
    possible_Q(bitboards[5], white_moves, not_white_pieces, True)
    possible_K(bitboards[6], white_moves, not_white_pieces, True)
    print('moves', time.time() - st)


def update_piece_masks():
    global white_pieces, black_pieces, not_white_pieces, not_black_pieces, empty, occupied
    b = bitboards
    white_pieces = multi_or(b[1:6])
    black_pieces = multi_or(b[7:12])
    not_white_pieces = np.bitwise_not(multi_or(b[1:7] + b[12]))
    not_black_pieces = np.bitwise_not(multi_or(b[7:13] + b[6]))
    empty = np.bitwise_not(multi_or(b[1:13]))
    occupied = np.bitwise_not(empty)


def possible_moves_black():
    global black_moves
    update_piece_masks()
    update_unsafe()
    black_moves = set()
    possible_bP(bitboards[7], black_moves)
    possible_N(bitboards[8], black_moves, not_black_pieces, False)
    possible_B(bitboards[9], black_moves, not_black_pieces, False)
    possible_R(bitboards[10], black_moves, not_black_pieces, False)
    possible_Q(bitboards[11], black_moves, not_black_pieces, False)
    possible_K(bitboards[12], black_moves, not_black_pieces, False)


def white_in_checkmate():
    # can't be in checkmate if it's not your turn
    if not white_turn:
        return False
    # can't be in checkmate if you're not in check
    if not white_check:
        return False
    # can't be in checkmate if you have legal moves
    if white_moves:
        return False
    return True


def black_in_checkmate():
    if white_turn:
        return False
    if not black_in_check():
        return False
    if black_moves:
        return False
    return True


# this is for the kings
# this is where the white king cant go
def unsafe_for_white():
    global blocking_squares, pinning_squares, en_passant_pinned
    blocking_squares = np.int64(0)
    pinning_squares = dict()
    en_passant_pinned = False

    unsafe = np.int64(0)

    king = bitboards[6]
    occupied_no_king = np.bitwise_and(occupied, np.bitwise_not(king))

    # pawns
    # threaten to capture right
    p_right = multi_and([l_shift(bitboards[7], -8 - 1), np.bitwise_not(file[1])])
    unsafe = np.bitwise_or(unsafe, p_right)

    # threaten to capture left
    p_left = multi_and([l_shift(bitboards[7], -8 + 1), np.bitwise_not(file[8])])
    unsafe = np.bitwise_or(unsafe, p_left)

    # knight
    for i in find_ones(bitboards[8]):
        n = span_piece(all_squares, i, knight_span, 18, king_bb=king)
        unsafe = np.bitwise_or(unsafe, n)

    # king
    for i in find_ones(bitboards[12]):
        k = span_piece(all_squares, i, king_span, 9)
        unsafe = np.bitwise_or(unsafe, k)

    # queen
    for i in find_ones(bitboards[11]):
        q = sliding_piece(all_squares, i, occupied_no_king, rook_moves=True, bishop_moves=True, king_bb=king)
        unsafe = np.bitwise_or(unsafe, q)

    # rook
    for i in find_ones(bitboards[10]):
        r = sliding_piece(all_squares, i, occupied_no_king, rook_moves=True, bishop_moves=False, king_bb=king)
        unsafe = np.bitwise_or(unsafe, r)

    # bishop
    for i in find_ones(bitboards[9]):
        b = sliding_piece(all_squares, i, occupied_no_king, rook_moves=False, bishop_moves=True, king_bb=king)
        unsafe = np.bitwise_or(unsafe, b)

    return unsafe


# this is where the black king cant go
def unsafe_for_black():
    global blocking_squares, pinning_squares
    # blocking_squares = np.int64(0)
    # pinning_squares = dict()

    unsafe = np.int64(0)

    king = bitboards[12]
    occupied_no_king = np.bitwise_and(occupied, np.bitwise_not(king))
    # pawns
    # threaten to capture right
    p_right = multi_and([l_shift(bitboards[1], 8 - 1), np.bitwise_not(file[1])])
    unsafe = np.bitwise_or(unsafe, p_right)

    # threaten to capture left
    p_left = multi_and([l_shift(bitboards[1], 8 + 1), np.bitwise_not(file[8])])
    unsafe = np.bitwise_or(unsafe, p_left)

    # knight
    for i in find_ones(bitboards[2]):
        n = span_piece(all_squares, i, knight_span, 18, king_bb=king)
        unsafe = np.bitwise_or(unsafe, n)

    # king
    for i in find_ones(bitboards[6]):
        k = span_piece(all_squares, i, king_span, 9)
        unsafe = np.bitwise_or(unsafe, k)

    # queens
    for i in find_ones(bitboards[5]):
        q = sliding_piece(all_squares, i, occupied_no_king, rook_moves=True, bishop_moves=True, king_bb=king)
        unsafe = np.bitwise_or(unsafe, q)

    # rook
    for i in find_ones(bitboards[4]):
        r = sliding_piece(all_squares, i, occupied_no_king, rook_moves=True, bishop_moves=False, king_bb=king)
        unsafe = np.bitwise_or(unsafe, r)

    # bishop
    for i in find_ones(bitboards[3]):
        b = sliding_piece(all_squares, i, occupied_no_king, rook_moves=False, bishop_moves=True, king_bb=king)
        unsafe = np.bitwise_or(unsafe, b)

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


def incr_num_moves():
    global num_moves
    num_moves += 1


def decr_num_moves():
    global num_moves
    num_moves -= 1


def flip_turns():
    global white_turn
    white_turn = not white_turn


# this is a list of move_id
# 0 is just a normal move or a capture
# 1 to 12 is for move promotion
# 13 is for double pawn push
# 14 is en_passant
# 15 is castling
def apply_move(start, end, move_id):
    moved_piece = get_piece(start)
    # not your turn to move
    if white_turn != is_white_piece(moved_piece):
        print("Not your turn")
        return
    captured_piece = get_piece(end)
    remove_piece(moved_piece, start)
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
    # update_piece_masks()
    # update_unsafe()
    # # white is still in check at the end of their turn
    # if is_white_turn and white_in_check():
    #     print('white put themself in check')
    #     undo_move()
    #     return False
    # # black is still in check at the end of their turn
    # elif not is_white_turn and black_in_check():
    #     print('black put themself in check')
    #     undo_move()
    #     return False
    incr_num_moves()
    flip_turns()
    # return True


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


def white_in_check():
    return np.bitwise_and(bitboards[6], unsafe_white) > 0


def black_in_check():
    return np.bitwise_and(bitboards[12], unsafe_black) > 0


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
    draw_bitboard(blocking_squares, RED, 2)
    for i in pinning_squares:
        draw_bitboard(pinning_squares[i], YELLOW, 7)
    # draw_bitboard(unsafe_black, GREEN, 7)
    # draw_bitboard(unsafe_white, RED, 4)


def update_possible_moves():
    if white_turn:
        possible_moves_white()
    else:
        possible_moves_black()


def get_legal_moves():
    if white_turn:
        return white_moves
    else:
        return black_moves


def perft_test(depth):
    if depth == 0:
        return 1
    st = time.time()
    update_possible_moves()
    print('update', time.time() - st)
    moves = get_legal_moves()
    num_positions = 0

    for move in moves:
        s, e, m = move
        apply_move(s, e, m)
        num_positions += perft_test(depth - 1)
        undo_move()
        decr_num_moves()
        flip_turns()

    return num_positions


def test():
    init_board()
    st = time.time()
    print(perft_test(3))
    print(time.time() - st)



def run_game():
    global screen, press_xy, release_xy, press_square, release_square, mouse_xy
    screen = pygame.display.set_mode((400, 400), 0, 32)
    mainClock = pygame.time.Clock()
    pygame.display.init()
    pygame.display.set_caption('Chess')
    clicking = False
    init_board()
    update_possible_moves()
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
                    decr_num_moves()
                    flip_turns()
                    update_possible_moves()
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
                        update_possible_moves()
                        print(white_in_checkmate(), black_in_checkmate())
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
        # color the dark squares
        if (get_file(i) + get_rank(i)) % 2 == 1:
            square_color = BLUE
        # color the light squares
        else:
            square_color = WHITE
        # draw the squares on the board
        pygame.draw.rect(screen, square_color, (350 - (i % 8) * 50, 350 - (i // 8) * 50, 50, 50))

    # draw red square if king is in check
    square_color = RED
    if white_check:
        for i in find_ones(bitboards[6]):
            pygame.draw.rect(screen, square_color, (350 - (i % 8) * 50, 350 - (i // 8) * 50, 50, 50))
    if black_check:
        for i in find_ones(bitboards[12]):
            pygame.draw.rect(screen, square_color, (350 - (i % 8) * 50, 350 - (i // 8) * 50, 50, 50))

    for i in range(64):
        if i == press_square:
            continue
        # draw the pieces on the board
        for b in range(1, len(bitboards)):
            if np.bitwise_and(np.left_shift(np.int64(1), i), bitboards[b]):
                screen.blit(pygame.transform.rotate(piece_img[b], 0), (350 - (i % 8) * 50, 350 - (i // 8) * 50))
    # if there is a piece being held, draw the held piece at the mouse position
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


# run_game()
test()
