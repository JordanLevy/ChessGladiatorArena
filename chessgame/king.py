import pygame

from enpassant import EnPassant
from move import Move
from pawn import Pawn
from rook import Rook


class King:

    def __init__(self, board_grid, move_list, is_white, file, rank):
        self.board_grid = board_grid
        self.board_grid[file][rank] = self
        self.move_list = move_list
        self.is_white = is_white
        if is_white:
            self.img = pygame.image.load('Images/WhiteKing.png')
        else:
            self.img = pygame.image.load('Images/BlackKing.png')
        self.img = pygame.transform.scale(self.img, (50, 50))
        self.file = file
        self.rank = rank
        self.has_moved = False

    def get_img(self):
        return self.img

    def get_has_moved(self):
        return self.has_moved

    def get_file(self):
        return self.file

    def get_rank(self):
        return self.rank

    def set_file(self, file):
        self.file = file

    def set_rank(self, rank):
        self.rank = rank

    def get_is_white(self):
        return self.is_white

    def get_is_black(self):
        return not self.is_white

    def is_in_check(self):
        return self.defended_by_enemy(self.file, self.rank)

    def defended_by_enemy(self, f, r):
        files = ['a', 'b', 'c', 'd', 'e', 'f', 'g', 'h']
        target = f + str(r)
        for i in range(0, 8):
            for j in range(1, 9):
                s = self.board_grid[files[i]][j]
                if s and (not type(s) is EnPassant) and (self.is_white != s.get_is_white()) and (
                        target in s.get_defended_squares()):
                    return True
        return False

    def get_defended_squares(self):
        files = ['a', 'b', 'c', 'd', 'e', 'f', 'g', 'h']
        legal_moves = []
        w = self.get_is_white()
        b = self.get_is_black()
        file_num = files.index(self.file)

        for i in range(-1, 2):
            f = file_num + i
            # if file out of bounds
            if f < 0 or f > 7:
                continue
            for j in range(-1, 2):
                r = self.rank + j
                # if rank out of bounds
                if r < 1 or r > 8:
                    continue
                legal_moves.append(files[f] + str(r))

        return legal_moves

    # get_legal_moves(String prev_move)
    # e.g. if the previous move was Bishop to h4, board_grid["a"][3].get_legal_moves("Bh4")
    # return list of legal squares to move to (e.g. ["a1", "a2", "a3", etc.])"""
    def get_legal_moves(self):
        files = ['a', 'b', 'c', 'd', 'e', 'f', 'g', 'h']
        legal_moves = []
        w = self.get_is_white()
        b = self.get_is_black()
        file_num = files.index(self.file)

        for i in range(-1, 2):
            f = file_num + i
            # if file out of bounds
            if f < 0 or f > 7:
                continue
            for j in range(-1, 2):
                r = self.rank + j
                # if rank out of bounds
                if r < 1 or r > 8:
                    continue
                s = self.board_grid[files[f]][r]
                # if square is occupied by a friendly piece, it's not en en passant marker, and it's friendly
                if s and not type(s) is EnPassant and w == s.get_is_white():
                    # king is blocked by its own piece
                    continue
                # if square is controlled by an enemy piece
                if self.defended_by_enemy(files[f], r):
                    continue
                legal_moves.append(files[f] + str(r))

        # if the king hasn't moved
        if not self.has_moved:

            rook = self.board_grid['a'][self.rank]
            # if there's a rook on a1 and it hasn't moved
            if rook and type(rook) is Rook and not rook.get_has_moved():
                # check for pieces in the way
                piece_blocking = False
                through_check = False
                for i in ['b', 'c', 'd']:
                    s = self.board_grid[i][self.rank]
                    if s:
                        piece_blocking = True
                for i in ['c', 'd', 'e']:
                    if self.defended_by_enemy(i, self.rank):
                        through_check = True
                # if there are no pieces in the way of long castle
                if not piece_blocking and not through_check:
                    # add long castle to legal moves
                    legal_moves.append('O-O-O')

            rook = self.board_grid['h'][self.rank]
            # if there's a rook on h1 and it hasn't moved
            if rook and type(rook) is Rook and not rook.get_has_moved():
                # check for pieces in the way
                piece_blocking = False
                through_check = False
                for i in ['f', 'g']:
                    s = self.board_grid[i][self.rank]
                    if s:
                        piece_blocking = True
                for i in ['e', 'f', 'g']:
                    if self.defended_by_enemy(i, self.rank):
                        through_check = True
                # if there are no pieces in the way of long castle
                if not piece_blocking and not through_check:
                    # add long castle to legal moves
                    legal_moves.append('O-O')

        return legal_moves

    # move(String destination) modifies the state of the board based on the location the piece is moving to (and
    # takes care of any captures that may have happened) e.g. board_grid["a"][3].move("b2")
    def move(self, f, r):
        legal_moves = self.get_legal_moves()
        w = self.get_is_white()
        if not f + str(r) in legal_moves:
            # trying to castle
            if r == self.rank:
                # trying to long castle
                if f == 'c':
                    # if long castle is legal
                    if 'O-O-O' in legal_moves:
                        # only pawns can capture en passant markers
                        is_capture = False
                        is_en_passant = False
                        self.move_list.append(
                            Move(self.is_white, 'K', self.file, self.rank, is_capture, is_en_passant, f, r))
                        # move the rook
                        self.board_grid['d'][self.rank] = self.board_grid['a'][self.rank]
                        self.board_grid['d'][self.rank].set_file('d')
                        self.board_grid['d'][self.rank].set_rank(self.rank)
                        self.board_grid['a'][self.rank] = None
                        # move the king
                        self.board_grid[self.file][self.rank] = None
                        self.file = f
                        self.rank = r
                        self.board_grid[self.file][self.rank] = self
                        self.has_moved = True
                        return True
                # trying to long castle
                if f == 'g':
                    # if short castle is legal
                    if 'O-O' in legal_moves:
                        # only pawns can capture en passant markers
                        is_capture = False
                        is_en_passant = False
                        self.move_list.append(
                            Move(self.is_white, 'K', self.file, self.rank, is_capture, is_en_passant, f, r))
                        # move the rook
                        self.board_grid['f'][self.rank] = self.board_grid['h'][self.rank]
                        self.board_grid['f'][self.rank].set_file('f')
                        self.board_grid['f'][self.rank].set_rank(self.rank)
                        self.board_grid['h'][self.rank] = None
                        # move the king
                        self.board_grid[self.file][self.rank] = None
                        self.file = f
                        self.rank = r
                        self.board_grid[self.file][self.rank] = self
                        self.has_moved = True
                        return True
            print('illegal move')
            return False
        # only pawns can capture en passant markers
        is_capture = not self.board_grid[f][r] is None and not type(self.board_grid[f][r]) is EnPassant
        is_en_passant = False
        self.move_list.append(Move(self.is_white, 'K', self.file, self.rank, is_capture, is_en_passant, f, r))
        self.board_grid[self.file][self.rank] = None
        self.file = f
        self.rank = r
        self.board_grid[self.file][self.rank] = self
        self.has_moved = True
        return True
