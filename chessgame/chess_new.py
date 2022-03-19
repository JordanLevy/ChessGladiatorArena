import numpy as np
from pygame.locals import *
import pygame
import sys

# globle varabuls
board = np.zeros(64, dtype=int)
screen = None
piece_img = []
#get wha file you are on
def get_file(n):
    return n % 8

#get what rank you are on
def get_rank(n):
    return n//8

def get_rank_start(n):
    return n * 8

def get_rank_end(n):
    return (n+1) * 8

#white: P = 1, N = 2, B = 3, R = 4, Q = 5, K = 6
#black: P = 7, N = 8, B = 9, R = 10, Q = 11, K = 12
def init_board():
    global piece_img
    # wight powns
    board[get_rank_start(1):get_rank_end(1)] = 1

    #black pawns
    board[get_rank_start(6):get_rank_end(6)] = 7
    #this wight back rank

    w = np.array([4, 2, 3, 5, 6, 3, 2, 4])
    board[get_rank_start(0):get_rank_end(0)] = w

    board[get_rank_start(7):get_rank_end(7)] = w + 6

    piece_img = [None] * 13

    files = ['', 'Images/WhitePawn.png', 'Images/WhiteKnight.png', 'Images/WhiteBishop.png',
             'Images/WhiteRook.png', 'Images/WhiteQueen.png', 'Images/WhiteKing.png',

             'Images/BlackPawn.png', 'Images/BlackKnight.png', 'Images/BlackBishop.png',
             'Images/BlackRook.png', 'Images/BlackQueen.png', 'Images/BlackKing.png']
    for j in range(1, 13):
        piece_img[j] = pygame.image.load(files[j])
        piece_img[j] = pygame.transform.scale(piece_img[j], (50, 50))
    print(board)
    print(piece_img)

def run_game():
    global screen
    mainClock = pygame.time.Clock()
    pygame.init()
    pygame.display.set_caption('Chess')
    screen = pygame.display.set_mode((400, 400), 0, 32)
    init_board()
    draw_board()
    while True:
        for event in pygame.event.get():
            if event.type == QUIT or (event.type == KEYDOWN and event.key == K_ESCAPE):
                pygame.quit()
                sys.exit()
        pygame.display.update()
        mainClock.tick(100)

def draw_board():
    BLUE = (18, 201, 192)
    WHITE = (249, 255, 212)
    RED = (255, 0, 0)
    GREEN = (25, 166, 0, 150)
    GREY = (150, 150, 150, 150)
    YELLOW = (255, 255, 0)


    for i in range(64):
        if (get_file(i)+get_rank(i)) % 2 == 0:
            square_color = BLUE
        else:
            square_color = WHITE
        pygame.draw.rect(screen, square_color, (get_file(i) * 50, 350 - get_rank(i) * 50, 50, 50))
        p = board[i]
        if p > 0:
            screen.blit(pygame.transform.rotate(piece_img[p], 0), (get_file(i) * 50, 350 - get_rank(i) * 50))
run_game()