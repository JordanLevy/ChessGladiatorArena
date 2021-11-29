from enpassant import EnPassant
from move import Move


class Piece:

    def __init__(self, board, is_white, file, rank):
        self.letter = '-'
        self.board = board
        self.board_grid = board.board_grid
        self.board_grid[file][rank] = self
        self.move_list = board.move_list
        self.is_white = is_white
        if is_white:
            self.img = ''
        else:
            self.img = ''
        self.file = file
        self.rank = rank
        self.has_moved = False

    def get_img(self):
        return self.img

    def get_has_moved(self):
        return self.has_moved

    def get_file(self):
        return self.file

    def get_rank(self):
        return self.rank

    def set_file(self, file):
        self.file = file

    def set_rank(self, rank):
        self.rank = rank

    def get_is_white(self):
        return self.is_white

    def get_is_black(self):
        return not self.is_white

    def get_defended_squares(self):
        return self.get_legal_moves()

    # get_legal_moves(String prev_move)
    # e.g. if the previous move was Bishop to h4, board_grid["a"][3].get_legal_moves("Bh4")
    # return list of legal squares to move to (e.g. ["a1", "a2", "a3", etc.])"""
    def get_legal_moves(self):
        return []

    # move(String destination) modifies the state of the board based on the location the piece is moving to (and
    # takes care of any captures that may have happened) e.g. board_grid["a"][3].move("b2")
    def move(self, f, r):
        legal_moves = self.get_legal_moves()
        if not f + str(r) in legal_moves:
            print('illegal move')
            return False
        is_capture = not self.board_grid[f][r] is None and not type(self.board_grid[f][r]) is EnPassant
        is_en_passant = False
        self.move_list.append(Move(self.is_white, self.letter, self.file, self.rank, is_capture, is_en_passant, f, r))
        self.board_grid[self.file][self.rank] = None
        self.file = f
        self.rank = r
        self.board_grid[self.file][self.rank] = self
        self.has_moved = True
        return True

    def __str__(self):
        return ('b', 'w')[self.is_white] + self.letter

    # returns true if making this move puts your own king in check and false if anything else

    def your_k_check(self, move):
        new_board = self.board.copy()
        legal_move = new_board.move(move.from_file, move.from_rank, move.to_file, move.to_rank)
        if not legal_move:
            return False
        color = move.is_white
        if color:
            if new_board.white_king.is_in_check():
                return True
        else:
            if new_board.black_king.is_in_check():
                return True
        return False
