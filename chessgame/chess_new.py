import numpy as np

board = np.zeros(64, dtype=int)

def get_file(n):
    return n % 8 + 1

def get_rank(n):
    return n//8 + 1

def get_rank_start(n):
    return (n - 1) * 8

def get_rank_end(n):
    return n * 8

#white: P = 1, N = 2, B = 3, R = 4, Q = 5, K = 6
#black: P = 7, N = 8, B = 9, R = 10, Q = 11, K = 12
def init_board():
    board[get_rank_start(2):get_rank_end(2)] = 1
    print(board)


init_board()