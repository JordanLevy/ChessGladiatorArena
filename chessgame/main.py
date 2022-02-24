# TODO fix your king check

from pygame.locals import *
import math

import pygame
import sys

from engine import Engine
from board import Board
from gamestate import GameState
import time
mainClock = pygame.time.Clock()

pygame.init()
pygame.display.set_caption('Chess')
screen = pygame.display.set_mode((400, 400), 0, 32)


def run_game():
    clicking = False
    click_x = -1
    click_y = -1
    release_x = -1
    release_y = -1
    click_square = ['', 0]
    release_square = ['', 0]
    square_clicked = None

    game_board = Board()
    game_board.setup_board()
    game_board.draw_board(pygame, screen)

    engine = Engine(game_board, False)
    promo = ''

    while True:
        for event in pygame.event.get():
            if event.type == QUIT or (event.type == KEYDOWN and event.key == K_ESCAPE):
                pygame.quit()
                sys.exit()
            if event.type == KEYDOWN and event.key == K_SPACE:
                game_board.undo_move()
                game_board.draw_board(pygame, screen)
                continue
            if event.type == KEYDOWN:
                if event.key == K_q:
                    promo = 'Q'
                elif event.key == K_r:
                    promo = 'R'
                elif event.key == K_b:
                    promo = 'B'
                elif event.key == K_n:
                    promo = 'N'
            if event.type == KEYUP:
                promo = ''
            if event.type == MOUSEBUTTONDOWN:
                if event.button == BUTTON_LEFT and not clicking:
                    clicking = True
                    click_x, click_y = pygame.mouse.get_pos()
                    click_x, click_y = game_board.files[math.floor(click_x / 50)], math.ceil(8 - click_y / 50)
                    square_clicked = game_board.get_piece(click_x, click_y)
                    # if they clicked a square with a piece on it (and not an en passant marker)
                    if square_clicked:
                        game_board.set_move_preview(square_clicked)
                    game_board.draw_board(pygame, screen)
            if event.type == MOUSEBUTTONUP:
                if event.button == BUTTON_LEFT and clicking:
                    clicking = False
                    game_board.set_move_preview(None)
                    release_x, release_y = pygame.mouse.get_pos()
                    release_x, release_y = game_board.files[math.floor(release_x / 50)], math.ceil(8 - release_y / 50)
                    if square_clicked:
                        piece = game_board.get_piece(click_x, click_y)
                        w = piece.get_is_white()
                        # if it's that player's turn to move
                        if w == game_board.white_turn:
                            #valid_move = game_board.move(click_x, click_y, release_x, release_y)
                            if piece.letter == 'P' and piece.rank == (2, 7)[w] and release_y == (1, 8)[w]:
                                valid_move = game_board.apply_move(click_x, click_y, release_x, release_y, promo)
                            else:
                                valid_move = game_board.apply_move(click_x, click_y, release_x, release_y, '')
                        else:
                            valid_move = False
                        if valid_move:
                            game_board.next_turn()
                            print(game_board.mat_eval)
                            game_state = game_board.is_game_over()
                            if game_state != GameState.IN_PROGRESS:
                               print("white turn", game_state)
                            start_time = time.time()
                            cpu_eval, cpu_move = engine.search_moves(3, -engine.mate_value, engine.mate_value, False) #engine.depth_one(game_board)
                            end_time = time.time()
                            print("the computation time is", str(end_time-start_time))
                            game_board.apply_move_by_ref(cpu_move)
                            game_board.next_turn()
                            print(game_board.mat_eval)
                            game_state = game_board.is_game_over()
                            if game_state != GameState.IN_PROGRESS:
                                print("black turn", game_state)
                    game_board.draw_board(pygame, screen)
        pygame.display.update()
        mainClock.tick(100)


run_game()
