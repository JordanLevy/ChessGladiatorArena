import copy

from bishop import Bishop
from enpassant import EnPassant
from king import King
from knight import Knight
from pawn import Pawn
from queen import Queen
from rook import Rook
import copy


class Board:

    def __init__(self):
        self.white_king = None
        self.black_king = None
        self.board_grid = {"a": [None] * 9, "b": [None] * 9, "c": [None] * 9, "d": [None] * 9, "e": [None] * 9,
                           "f": [None] * 9,
                           "g": [None] * 9,
                           "h": [None] * 9}
        self.move_list = []
        self.files = ['a', 'b', 'c', 'd', 'e', 'f', 'g', 'h']

    def copy(self):
        new_board = Board()
        for i in self.files:
            for j in range(len(self.board_grid[i])):
                s = self.board_grid[i][j]
                new_piece = None
                # if there's no piece here, set it to None
                if not s:
                    new_board.board_grid[i][j] = None
                # otherwise, create a new piece with the same attributes on the new board
                elif type(s) is EnPassant:
                    new_piece = EnPassant(new_board, s.get_is_white(), s.get_file(), s.get_rank(), s.get_move_num())
                elif type(s) is Pawn:
                    new_piece = Pawn(new_board, s.get_is_white(), s.get_file(), s.get_rank())
                elif type(s) is Knight:
                    new_piece = Knight(new_board, s.get_is_white(), s.get_file(), s.get_rank())
                elif type(s) is Bishop:
                    new_piece = Bishop(new_board, s.get_is_white(), s.get_file(), s.get_rank())
                elif type(s) is Rook:
                    new_piece = Rook(new_board, s.get_is_white(), s.get_file(), s.get_rank())
                elif type(s) is Queen:
                    new_piece = Queen(new_board, s.get_is_white(), s.get_file(), s.get_rank())
                elif type(s) is King:
                    new_piece = King(new_board, s.get_is_white(), s.get_file(), s.get_rank())
                    if s.get_is_white():
                        new_board.white_king = new_piece
                    else:
                        new_board.black_king = new_piece
                new_board.board_grid[i][j] = new_piece
        print(self.move_list)
        print(new_board.move_list)
        for i in range(len(self.move_list)):

            new_board.move_list.append(copy.deepcopy(self.move_list[i]))

        return new_board

    def setup_board(self):

        self.white_king = King(self, True, 'e', 1)
        self.black_king = King(self, False, 'e', 8)

        Queen(self, True, 'd', 1)
        Queen(self, False, 'd', 8)

        Rook(self, True, 'a', 1)
        Rook(self, True, 'h', 1)
        Rook(self, False, 'a', 8)
        Rook(self, False, 'h', 8)

        Bishop(self, True, 'c', 1)
        Bishop(self, True, 'f', 1)
        Bishop(self, False, 'c', 8)
        Bishop(self, False, 'f', 8)

        Knight(self, True, 'b', 1)
        Knight(self, True, 'g', 1)
        Knight(self, False, 'b', 8)
        Knight(self, False, 'g', 8)

        # white pawn row
        for i in range(8):
            Pawn(self, True, self.files[i], 2)
        # black pawn row
        for i in range(8):
            Pawn(self, False, self.files[i], 7)

    # removes old en passant markers from the board
    def clear_en_passant_markers(self):
        f = [0, "a", "b", "c", "d", "e", "f", "g", "h"]
        for i in range(1, 9):
            for j in range(1, 9):
                s = self.board_grid[f[i]][j]
                # if it's an en passant marker and it's more than 1 move old
                if type(s) is EnPassant and (len(self.move_list) - s.get_move_num()) > 1:
                    self.board_grid[f[i]][j] = None

    def draw_board(self, pygame, screen):

        BLUE = (18, 201, 192)
        WHITE = (249, 255, 212)
        RED = (255, 0, 0)

        square_color = BLUE
        for i in range(8):
            for j in range(8):
                square_color = WHITE
                if (i + j) % 2 == 1:
                    square_color = BLUE
                if self.white_king.is_in_check() and i == self.files.index(
                        self.white_king.get_file()) and (8 - j) == self.white_king.get_rank():
                    square_color = RED
                if self.black_king.is_in_check() and i == self.files.index(
                        self.black_king.get_file()) and (8 - j) == self.black_king.get_rank():
                    square_color = RED
                pygame.draw.rect(screen, square_color, (i * 50, j * 50, 50, 50))

        f = [0, "a", "b", "c", "d", "e", "f", "g", "h"]
        for i in range(1, 9):
            for j in range(1, 9):
                # if it's not equal to None
                if self.board_grid[f[i]][j]:
                    img = pygame.image.load(self.board_grid[f[i]][j].img)
                    if type(self.board_grid[f[i]][j]) is EnPassant:
                        img = pygame.transform.scale(img, (25, 25))
                    else:
                        img = pygame.transform.scale(img, (50, 50))
                    screen.blit(pygame.transform.rotate(img, 0),
                                ((i - 1) * 50, 400 - j * 50))

    def move(self, from_file, from_rank, to_file, to_rank):
        return self.board_grid[from_file][from_rank].move(to_file, to_rank)

    def __str__(self):
        a = ''
        for i in self.files:
            a += i + ': '
            for j in range(1, len(self.board_grid[i])):
                a += self.board_grid[i][j].__str__() + ', '
            a += '\n'
        return a
