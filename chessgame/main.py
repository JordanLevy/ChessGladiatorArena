from pygame.locals import *
import math

import pygame
import sys

from engine import Engine
from enpassant import EnPassant
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

    while True:
        for event in pygame.event.get():
            if event.type == QUIT or (event.type == KEYDOWN and event.key == K_ESCAPE):
                pygame.quit()
                sys.exit()
            if event.type == MOUSEBUTTONDOWN:
                if event.button == BUTTON_LEFT and not clicking:
                    clicking = True
                    click_x, click_y = pygame.mouse.get_pos()
                    click_x, click_y = game_board.files[math.floor(click_x / 50)], math.ceil(8 - click_y / 50)
                    square_clicked = game_board.get_piece(click_x, click_y)
                    # if they clicked a square with a piece on it (and not an en passant marker)
                    if square_clicked and not type(square_clicked) is EnPassant:
                        game_board.set_move_preview(square_clicked)
                    game_board.draw_board(pygame, screen)
            if event.type == MOUSEBUTTONUP:
                if event.button == BUTTON_LEFT and clicking:
                    clicking = False
                    game_board.set_move_preview(None)
                    release_x, release_y = pygame.mouse.get_pos()
                    release_x, release_y = game_board.files[math.floor(release_x / 50)], math.ceil(8 - release_y / 50)
                    if square_clicked:
                        # if it's that player's turn to move
                        if game_board.get_piece(click_x, click_y).get_is_white() == game_board.white_turn:
                            valid_move = game_board.move(click_x, click_y, release_x, release_y)
                        else:
                            valid_move = False
                        if valid_move:
                            print(game_board.mat_eval)
                            game_board.next_turn()
                            game_state = game_board.is_game_over()
                            if game_state != GameState.IN_PROGRESS:
                                print("game over")
                            start_time = time.time()
                            cpu_eval, cpu_move = engine.depth_one(game_board) #engine.search_moves(game_board, 3)
                            end_time = time.time()
                            print("the computation time is", str(end_time-start_time))
                            game_board.move(cpu_move.get_from_file(), cpu_move.get_from_rank(), cpu_move.get_to_file(), cpu_move.get_to_rank())
                            game_board.next_turn()
                            game_state = game_board.is_game_over()
                            if game_state != GameState.IN_PROGRESS:
                                print("game over")
                            # print(game_board.get_move_num())
                            # print(game_board.to_fen())
                            # print(game_board.board_repetitions)
                    game_board.draw_board(pygame, screen)
        pygame.display.update()
        mainClock.tick(100)


run_game()
