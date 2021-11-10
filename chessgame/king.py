import pygame


class King:

    def __init__(self, board_grid, is_white, file, rank):
        self.board_grid = board_grid
        self.is_white = is_white
        self.img = pygame.image.load('Images/WhiteKing.png').convert()
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

    # get_legal_moves(String prevMove)
    # return list of legal squares to move to (e.g. ["a1", "a2", "a3", etc.])"""
    def get_legal_moves(self, prevMove):
        pass

    # move(String destination) modifies the state of the board based on the location the piece is moving to (and
    # takes care of any captures that may have happened) e.g. board_grid["a"][3].move("b2")
    def move(self, destination):
        pass
