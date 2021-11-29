#!/usr/bin/python3.4
# YOU MUST PROVIDE YOUR OWN pic.png
# Setup Python ----------------------------------------------- #
from pygame.locals import *
import math

import pygame
import sys

# Setup pygame/window ---------------------------------------- #
from enpassant import EnPassant
from board import Board

mainClock = pygame.time.Clock()

pygame.init()
# just fare start screen
pygame.display.set_caption('Chess')
screen = pygame.display.set_mode((400, 400), 0, 32)

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

game_board = None


def run_game():
    global game_board
    game_board = Board()
    game_board.setup_board()

    new_board = game_board.copy()

    print(game_board)
    print(new_board)

    while True:

        # Background color--------------------------------------------- #
        screen.fill((0, 0, 50))

        mx, my = pygame.mouse.get_pos()

        rot = 0
        loc = [mx, my]

        game_board.draw_board(pygame, screen)

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
                    click_x, click_y = game_board.files[math.floor(click_x / 50)], math.ceil(8 - click_y / 50)
                    square_clicked = game_board.board_grid[click_x][click_y]
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
                    release_x, release_y = game_board.files[math.floor(release_x / 50)], math.ceil(8 - release_y / 50)
                    if square_clicked:
                        valid_move = game_board.move(click_x, click_y, release_x, release_y)
                        if valid_move:
                            print(game_board.move_list[-1])
                            game_board.clear_en_passant_markers()

        # Update ------------------------------------------------- #
        pygame.display.update()
        mainClock.tick(100)


run_game()
