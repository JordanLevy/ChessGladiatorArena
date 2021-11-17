#!/usr/bin/python3.4
# YOU MUST PROVIDE YOUR OWN pic.png
# Setup Python ----------------------------------------------- #
from pygame.locals import *
import math

import pygame
import sys

# Setup pygame/window ---------------------------------------- #
from enpassant import EnPassant
from king import King
from pawn import Pawn
from queen import Queen
from Bishop import Bishop
from Knight import Knight
from rook import Rook

mainClock = pygame.time.Clock()

pygame.init()
# just fare start screen
pygame.display.set_caption('test game')
screen = pygame.display.set_mode((400, 400), 0, 32)

board_grid = {"a": [None] * 9, "b": [None] * 9, "c": [None] * 9, "d": [None] * 9, "e": [None] * 9, "f": [None] * 9,
              "g": [None] * 9,
              "h": [None] * 9}
move_list = []

files = ['a', 'b', 'c', 'd', 'e', 'f', 'g', 'h']


offset = [0, 0]

clicking = False
right_clicking = False
middle_click = False
click_x = -1
click_y = -1
release_x = -1
release_y = -1
click_square = ['', 0]
release_square = ['', 0]


def setup_board():
    # white pawn row
    for i in range(8):
        Pawn(board_grid, move_list, True, files[i], 2)

    # black pawn row
    for i in range(8):
        Pawn(board_grid, move_list, False, files[i], 7)

    King(board_grid, move_list, True, 'e', 1)
    King(board_grid, move_list, False, 'e', 8)

    Queen(board_grid, move_list, True, 'd', 1)
    Bishop(board_grid,move_list,False,'c',8)
    Knight(board_grid,move_list,True,'c',1)
    Rook(board_grid, move_list, True, 'h', 4)

    Queen(board_grid, move_list, False, 'd', 8)

    Rook(board_grid, move_list, True, 'a', 1)
    Rook(board_grid, move_list, True, 'h', 1)
    Rook(board_grid, move_list, False, 'a', 8)
    Rook(board_grid, move_list, False, 'h', 8)

    Bishop(board_grid, move_list, True, 'c', 1)
    Bishop(board_grid, move_list, True, 'f', 1)
    Bishop(board_grid, move_list, False, 'c', 8)
    Bishop(board_grid, move_list, False, 'f', 8)

def draw_board():
    f = [0, "a", "b", "c", "d", "e", "f", "g", "h"]
    for i in range(1, 9):
        for j in range(1, 9):
            # if it's not equal to None
            if board_grid[f[i]][j]:
                screen.blit(pygame.transform.rotate(board_grid[f[i]][j].get_img(), 0),
                            ((i - 1) * 50, 400 - j * 50))


# removes old en passant markers from the board
def clear_en_passant_markers():
    f = [0, "a", "b", "c", "d", "e", "f", "g", "h"]
    for i in range(1, 9):
        for j in range(1, 9):
            s = board_grid[f[i]][j]
            # if it's an en passant marker and it's more than 1 move old
            if type(s) is EnPassant and (len(move_list) - s.get_move_num()) > 1:
                board_grid[f[i]][j] = None


def run_game():
    while True:

        # Background color--------------------------------------------- #
        screen.fill((0, 0, 50))

        mx, my = pygame.mouse.get_pos()

        rot = 0
        loc = [mx, my]

        BLUE = (18, 201, 192)
        WHITE = (249, 255, 212)

        square_color = BLUE
        for i in range(8):
            for j in range(8):
                square_color = WHITE
                if (i + j) % 2 == 1:
                    square_color = BLUE
                pygame.draw.rect(screen, square_color, (i * 50, j * 50, 50, 50))

        draw_board()

        # Buttons ------------------------------------------------ #
        right_clicking = False
        for event in pygame.event.get():
            if event.type == QUIT:
                pygame.quit()
                sys.exit()
            if event.type == KEYDOWN:
                if event.key == K_ESCAPE:
                    pygame.quit()
                    sys.exit()
            if event.type == MOUSEBUTTONDOWN:
                if event.button == 1:
                    clicking = True
                    click_x, click_y = pygame.mouse.get_pos()
                    click_x, click_y = files[math.floor(click_x / 50)], math.ceil(8 - click_y / 50)
                    square_clicked = board_grid[click_x][click_y]
                    # if they clicked a square with a piece on it (and not an en passant marker)
                    if square_clicked and not type(square_clicked) is EnPassant:
                        print(square_clicked.get_legal_moves())
                if event.button == 3:
                    right_clicking = True
                if event.button == 2:
                    middle_click = not middle_click
                if event.button == 4:
                    offset[1] -= 10
                if event.button == 5:
                    offset[1] += 10
            if event.type == MOUSEBUTTONUP:
                if event.button == 1:
                    clicking = False
                    release_x, release_y = pygame.mouse.get_pos()
                    release_x, release_y = files[math.floor(release_x / 50)], math.ceil(8 - release_y / 50)
                    if square_clicked:
                        valid_move = board_grid[click_x][click_y].move(release_x, release_y)
                        if valid_move:
                            print(move_list[-1])
                            clear_en_passant_markers()

        # Update ------------------------------------------------- #
        pygame.display.update()
        mainClock.tick(100)


setup_board()
run_game()
