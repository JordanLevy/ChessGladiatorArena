from enum import Enum


class GameState(Enum):
    IN_PROGRESS = 0  # game has not ended yet
    WHITE_CHECKMATE = 1  # white wins by checkmate
    BLACK_CHECKMATE = 2  # black wins by checkmate
    STALEMATE = 3  # draw, one side has no legal moves and is not in check
    FIFTY_MOVE_RULE = 4  # draw, 50 moves have passed since a capture or pawn move
    THREEFOLD_REPETITION = 5  # draw, the same position reached 3 times
