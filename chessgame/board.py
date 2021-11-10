#!/usr/bin/python3.4
# YOU MUST PROVIDE YOUR OWN pic.png
# Setup Python ----------------------------------------------- #
import pygame, sys

# Setup pygame/window ---------------------------------------- #
mainClock = pygame.time.Clock()
from pygame.locals import *

pygame.init()
# just fare start screan
pygame.display.set_caption('test game')
screen = pygame.display.set_mode((800, 800), 0, 32)

offset = [0, 0]

clicking = False
right_clicking = False
middle_click = False

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
            if (i+j)%2 == 1:
                square_color = BLUE
            pygame.draw.rect(screen, square_color, (i*100, j*100, 100, 100))

    # screen.blit(pygame.transform.rotate(img, rot), (loc[0] + offset[0], loc[1] + offset[1]))

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