import pygame

from move import Move


class EnPassant:

    def __init__(self, board_grid, move_list, is_white, file, rank, move_num):
        self.board_grid = board_grid
        self.board_grid[file][rank] = self
        self.move_list = move_list
        self.is_white = is_white
        if is_white:
            self.img = pygame.image.load('Images/WhitePawn.png')
        else:
            self.img = pygame.image.load('Images/BlackPawn.png')
        self.img = pygame.transform.scale(self.img, (25, 25))
        self.file = file
        self.rank = rank
        self.move_num = move_num

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

    def get_legal_moves(self):
        return []

    def move(self, f, r):
        pass
