from random import *

from enpassant import EnPassant


class Engine:

    def __init__(self, board, is_white):
        self.board = board
        self.is_white = is_white
        self.max_depth = 2

    def depth_one(self, analysis_board):
        # w = True if it's white's turn to move
        w = analysis_board.white_turn
        # get a list of all legal moves
        all_legal_moves = analysis_board.get_all_legal_moves(w)
        # n is number of legal moves in list
        n = len(all_legal_moves)
        # if there are no moves, return
        if n == 0:
            return 0, None
        best_eval = 1000
        best_move = None

        # for each legal move
        for i in all_legal_moves:
            # apply the move to the board
            new_board = analysis_board.copy_with_move(i)
            # it's the other player's turn now
            new_board.next_turn()
            # recurse, e is the eval, b is the best move
            e = self.eval_position(new_board)
            if e < best_eval:
                best_eval = e
                best_move = i
        return best_eval, best_move

    def evaluate_best_move(self, analysis_board, depth, line):
        # w = True if it's white's turn to move
        w = analysis_board.white_turn
        # get a list of all legal moves
        all_legal_moves = analysis_board.get_all_legal_moves(w)
        # n is number of legal moves in list
        n = len(all_legal_moves)
        # if there are no moves, return
        if n == 0:
            return None
        best_black_score = 1000
        best_white_score = -1000
        best_move = None
        # if we reached max depth, return the position evaluation
        if depth > self.max_depth:
            e = self.eval_position(analysis_board)
            return e, None
        # for each legal move
        for i in all_legal_moves:
            # apply the move to the board
            new_board = analysis_board.copy_with_move(i)
            # it's the other player's turn now
            new_board.next_turn()
            # recurse, e is the eval, b is the best move
            e, b = self.evaluate_best_move(new_board, depth + 1, line + ' ' + i.__str__())
            # if it's white's turn
            if w:
                # if this is the best white eval, set it as the best move
                if e > best_white_score:
                    best_white_score = e
                    best_move = b
            else:
                if e < best_black_score:
                    best_black_score = e
                    best_move = b
        if w:
            return best_white_score, best_move
        return best_black_score, best_move

    def search_moves(self, analysis_board, depth):
        if depth == 0:
            return self.eval_position(analysis_board), None

        w = analysis_board.white_turn
        all_legal_moves = analysis_board.get_all_legal_moves(w)

        if len(all_legal_moves) == 0:
            return 0, None

        best_eval = -1000
        best_move = None

        for m in all_legal_moves:
            new_board = analysis_board.copy_with_move(m)
            new_board.next_turn()
            a = self.search_moves(new_board, depth - 1)
            eval, q = -a[0], a[1]
            if eval > best_eval:
                best_eval = eval
                best_move = m

        return best_eval, best_move

    def get_random_move(self):
        all_legal_moves = self.board.get_all_legal_moves(self.is_white)
        n = len(all_legal_moves)
        if n == 0:
            return None
        rand_index = randint(0, n - 1)
        return all_legal_moves[rand_index]

    def eval_position(self, analysis_board):
        return analysis_board.mat_eval
