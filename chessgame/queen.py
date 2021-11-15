import pygame

from enpassant import EnPassant
from move import Move


class Queen:

    def __init__(self, board_grid, move_list, is_white, file, rank):
        self.board_grid = board_grid
        self.board_grid[file][rank] = self
        self.move_list = move_list
        self.is_white = is_white
        if is_white:
            self.img = pygame.image.load('Images/WhiteQueen.png')
        else:
            self.img = pygame.image.load('Images/BlackQueen.png')
        self.img = pygame.transform.scale(self.img, (50, 50))
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

    def get_legal_captures(self):
        return []

    # get_legal_moves(String prev_move)
    # e.g. if the previous move was Bishop to h4, board_grid["a"][3].get_legal_moves("Bh4")
    # return list of legal squares to move to (e.g. ["a1", "a2", "a3", etc.])"""
    def get_legal_moves(self):
        legal_moves = []
        return legal_moves

    # move(String destination) modifies the state of the board based on the location the piece is moving to (and
    # takes care of any captures that may have happened) e.g. board_grid["a"][3].move("b2")
    def move(self, f, r):
        return False
