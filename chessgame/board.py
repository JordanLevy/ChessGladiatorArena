#!/usr/bin/python3.4
# YOU MUST PROVIDE YOUR OWN pic.png
# Setup Python ----------------------------------------------- #
import pygame
import sys

# Setup pygame/window ---------------------------------------- #
from king import King

mainClock = pygame.time.Clock()
from pygame.locals import *

pygame.init()
# just fare start screan
pygame.display.set_caption('test game')
screen = pygame.display.set_mode((800, 800), 0, 32)

board_grid = {"a": [None] * 9, "b": [None] * 9, "c": [None] * 9, "d": [None] * 9, "e": [None] * 9, "f": [None] * 9,
              "g": [None] * 9,
              "h": [None] * 9}

white_king = King(board_grid, True, "h", 3)
board_grid["h"][3] = white_king

offset = [0, 0]

clicking = False
right_clicking = False
middle_click = False


def draw_board():
    files = [0, "a", "b", "c", "d", "e", "f", "g", "h"]
    for i in range(1, 9):
        for j in range(1, 9):
            # if it's not equal to None
            if board_grid[files[i]][j]:
                screen.blit(pygame.transform.rotate(board_grid[files[i]][j].get_img(), rot),
                            ((i - 1) * 100, 800 - j * 100))


# Loop ------------------------------------------------------- #
while True:

    # Background coler--------------------------------------------- #
    screen.fill((0, 0, 50))

    mx, my = pygame.mouse.get_pos()

    rot = 0
    loc = [mx, my]
    if clicking:
        rot -= 90
    if right_clicking:
        rot += 180
    if middle_click:
        rot += 90

    BLUE = (18, 201, 192)
    WHITE = (249, 255, 212)

    square_color = BLUE
    for i in range(8):
        for j in range(8):
            square_color = WHITE
            if (i + j) % 2 == 1:
                square_color = BLUE
            pygame.draw.rect(screen, square_color, (i * 100, j * 100, 100, 100))

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

    # Update ------------------------------------------------- #
    pygame.display.update()
    mainClock.tick(10)
