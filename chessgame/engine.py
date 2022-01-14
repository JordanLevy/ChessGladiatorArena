from random import *


class Engine:

    def __init__(self, board, is_white):
        self.board = board
        self.is_white = is_white
        self.max_depth = 2

    def evaluate(self, analysis_board, depth):
        all_legal_moves = self.board.get_all_legal_moves(self.is_white)
        n = len(all_legal_moves)
        if n == 0:
            return None
        min_val = 1000
        for i in all_legal_moves:
            e = self.evaluate(analysis_board.copy_with_move(i), depth + 1)
            if e < min_val:
                min_val = e
        if depth > self.max_depth:
            return



    def get_random_move(self):
        all_legal_moves = self.board.get_all_legal_moves(self.is_white)
        n = len(all_legal_moves)
        if n == 0:
            return None
        rand_index = randint(0, n - 1)
        return all_legal_moves[rand_index]
