import pygame

class King:

    def __init__(self, is_white, file, rank):
        self.is_white = is_white
        self.img = pygame.image.load('Images/WhiteKing.png').convert()
        self.file = file
        self.rank = rank

    def get_img(self):
        return self.img