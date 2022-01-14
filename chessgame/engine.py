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
        if depth > self.max_depth:
            return
        for i in all_legal_moves:
            e = self.evaluate(analysis_board.copy_with_move(i), depth + 1)
            if e < min_val:
                min_val = e
        return min_val



    def get_random_move(self):
        all_legal_moves = self.board.get_all_legal_moves(self.is_white)
        n = len(all_legal_moves)
        if n == 0:
            return None
        rand_index = randint(0, n - 1)
        return all_legal_moves[rand_index]

    def eval_position(self):
        white_val = 0
        black_val = 0
        pawn = 1
        knight = 3
        bishop = 3.3
        queen = 9
        rook = 5
        king = 90
        for i in range(8):
            for j in range(8):
                s = self.board.get_piece(self.board.files[i], j)
                if s is None:
                    continue
                if s.is_white:

                    white_val =