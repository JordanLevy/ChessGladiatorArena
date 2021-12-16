import pygame

from move import Move


class EnPassant:

    def __init__(self, board, is_white, file, rank, move_num):
        self.letter = 'E'
        self.board = board
        self.move_list = board.move_list
        self.is_white = is_white
        if is_white:
            self.img = 'Images/WhitePawn.png'
        else:
            self.img = 'Images/BlackPawn.png'
        self.file = file
        self.rank = rank
        self.move_num = move_num
        self.board.set_piece(self)

    def get_img(self):
        return self.img

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

    def get_move_num(self):
        return self.move_num

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

    def get_legal_moves(self):
        return []

    def move(self, f, r):
        pass

    def __str__(self):
        return (self.letter.lower(), self.letter)[self.is_white]
