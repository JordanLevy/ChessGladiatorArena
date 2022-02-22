from random import *
from gamestate import GameState

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
            analysis_board.apply_move_by_ref(i)
            # it's the other player's turn now
            analysis_board.next_turn()
            # recurse, e is the eval, b is the best move
            e = self.eval_position(analysis_board)
            if e < best_eval:
                best_eval = e
                best_move = i
            analysis_board.undo_move()
        return best_eval, best_move

    def evaluate_best_move(self, depth, line):
        # w = True if it's white's turn to move
        w = self.board.white_turn
        # get a list of all legal moves
        all_legal_moves = self.board.get_all_legal_moves(w)
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
            e = self.eval_position(self.board)
            return e, None
        # for each legal move
        for i in all_legal_moves:
            # apply the move to the board
            self.board.apply_move_by_ref(i)
            # it's the other player's turn now
            self.board.next_turn()
            # recurse, e is the eval, b is the best move
            e, b = self.evaluate_best_move(depth + 1, line + ' ' + i.__str__())
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
            self.board.undo_move()
        if w:
            return best_white_score, best_move
        return best_black_score, best_move

    def search_moves(self, depth, alpha, beta, is_white):
        if depth == 0 or self.board.is_game_over() != GameState.IN_PROGRESS:
            return self.eval_position(self.board), None
        if is_white:
            best_eval = -40000
            best_move = None
            all_legal_moves = self.board.get_all_legal_moves(is_white)
            for m in all_legal_moves:
                # make the move on the board
                self.board.apply_move_by_ref(m)
                # just sayiing its the next tern
                self.board.next_turn()
                a = self.search_moves(depth - 1, alpha, beta, False)
                # a0 and a1 is the eval and best move eg.Nc3
                eval = a[0]
                q = a[1]
                # if we find a better move
                if eval > best_eval:
                    best_eval = eval
                    best_move = m
                self.board.undo_move()
                alpha = max(alpha, eval)
                if beta <= alpha:
                    break
        # this it for blacks tern
        else:
            best_eval = 40000
            best_move = None
            all_legal_moves = self.board.get_all_legal_moves(is_white)
            for m in all_legal_moves:
                # make the move on the board
                self.board.apply_move_by_ref(m)
                # just sayiing its the next tern
                self.board.next_turn()
                a = self.search_moves(depth - 1, alpha, beta, True)
                # a0 and a1 is the eval and best move eg.Nc3
                eval = a[0]
                q = a[1]
                # if we find a better move
                if eval < best_eval:
                    best_eval = eval
                    best_move = m
                self.board.undo_move()
                beta = min(beta, eval)
                if beta <= alpha:
                    break
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
