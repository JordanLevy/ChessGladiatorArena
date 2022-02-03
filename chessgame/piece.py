from enpassant import EnPassant
from move import Move


class Piece:

    def __init__(self, board, is_white, file, rank):
        self.letter = '-'
        self.board = board
        self.is_white = is_white
        if is_white:
            self.img = ''
        else:
            self.img = ''
        self.file = file
        self.rank = rank
        self.num_times_moved = 0
        self.board.set_piece(self)

    def get_img(self):
        return self.img

    def get_has_moved(self):
        return self.num_times_moved > 0

    def get_num_times_moved(self):
        return self.num_times_moved

    def set_num_times_moved(self, num):
        self.num_times_moved = num

    def increment_num_times_moved(self):
        self.num_times_moved += 1

    def decrement_num_times_moved(self):
        self.num_times_moved -= 1

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

    # returns a tuple (f, r) representing the square at an offset d=(df, dr) from this piece
    # returns -1 if the offset refers to a square that is out of bounds
    def get_offset(self, d):
        df, dr = d
        f = self.board.files.index(self.file) + df
        r = self.rank + dr
        if f < 0 or f > 7:
            return -1
        if r < 1 or r > 8:
            return -1
        f = self.board.files[f]
        return f, r

    # returns the piece on the board at an offset d=(df, dr) from this piece
    # returns -1 if the offset refers to a square that is out of bounds
    def get_piece_at_offset(self, d):
        a = self.get_offset(d)
        if a == -1:
            return -1
        f, r = a
        return self.board.get_piece(f, r)

    def get_defended_squares(self):
        return self.get_possible_moves()

    # return list of squares this piece could move to, regardless of legality
    # e.g. even if a move results in a check to one's own king, it is still returned by this function
    def get_possible_moves(self):
        return []

    # e.g. if the previous move was Bishop to h4, board_grid["a"][3].get_legal_moves("Bh4")
    # return list of legal squares to move to (e.g. ["a1", "a2", "a3", etc.])"""
    def get_legal_moves(self):
        legal_moves = self.get_possible_moves()
        for i in range(len(legal_moves) - 1, -1, -1):
            # f = legal_moves[i][0]
            # r = int(legal_moves[i][1])
            # m = Move(self.is_white, self.letter, self.file, self.rank, False, False, f, r)
            m = legal_moves[i]
            if self.your_king_check(m):
                legal_moves.pop(i)
        return legal_moves

    def is_legal_move(self, move):
        legal_moves = self.get_legal_moves()
        for m in legal_moves:
            if m == move:
                return True
        return False

    # move(String destination) modifies the state of the board based on the location the piece is moving to (and
    # takes care of any captures that may have happened) e.g. board_grid["a"][3].move("b2")
    def move(self, move, check_legality=True):
        if check_legality:
            if not self.is_legal_move(move):
                print('illegal move')
                return False
        f = move.to_file
        r = move.to_rank
        self.board.remove_piece(self.file, self.rank)
        self.file = f
        self.rank = r
        self.increment_num_times_moved()
        self.board.set_piece(self)
        return True

    # returns true if making this move puts your own king in check and false if anything else
    def your_king_check(self, move):
        # if it's a promotion move, we don't check legality
        # TODO, make this actually check if promotion is legal
        if move.is_promotion:
            return
        #self.board.move_by_ref(move)
        new_board = self.board.copy_with_move(move)
        if move.is_white:
            if new_board.white_king.is_in_check():
                #self.board.undo_move()
                return True
        else:
            if new_board.black_king.is_in_check():
                #self.board.undo_move()
                return True
        #self.board.undo_move()
        return False

    def __str__(self):
        return (self.letter.lower(), self.letter)[self.is_white]
