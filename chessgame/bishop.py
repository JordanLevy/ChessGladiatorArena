import pygame

from enpassant import EnPassant
from move import Move


class Bishop:

    def __init__(self, board, is_white, file, rank):
        self.board = board
        self.board_grid = board.board_grid
        self.board_grid[file][rank] = self
        self.move_list = board.move_list
        self.is_white = is_white
        if is_white:
            self.img = 'Images/WhiteBishop.png'
        else:
            self.img = 'Images/BlackBishop.png'
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

    def get_defended_squares(self):
        files = ['a', 'b', 'c', 'd', 'e', 'f', 'g', 'h']
        legal_moves = []
        w = self.get_is_white()
        b = self.get_is_black()
        file_num = files.index(self.file)
        # up/right
        for i in range(1, 9):
            f = file_num + i
            r = self.rank + i
            if f < 0 or f > 7:
                continue
            if r < 1 or r > 8:
                continue
            s = self.board_grid[files[f]][r]
            if s:
                if type(s) is EnPassant:
                    legal_moves.append(files[f] + str(r))
                    continue
                else:
                    legal_moves.append(files[f] + str(r))
                    break
            else:
                legal_moves.append(files[f] + str(r))

        # down/right
        for i in range(1, 9):
            f = file_num + i
            r = self.rank - i
            if f < 0 or f > 7:
                continue
            if r < 1 or r > 8:
                continue
            s = self.board_grid[files[f]][r]
            if s:
                if type(s) is EnPassant:
                    legal_moves.append(files[f] + str(r))
                    continue
                else:
                    legal_moves.append(files[f] + str(r))
                    break
            else:
                legal_moves.append(files[f] + str(r))
        # up/left
        for i in range(1, 9):
            f = file_num - i
            r = self.rank + i
            if f < 0 or f > 7:
                continue
            if r < 1 or r > 8:
                continue
            s = self.board_grid[files[f]][r]
            if s:
                if type(s) is EnPassant:
                    legal_moves.append(files[f] + str(r))
                    continue
                else:
                    legal_moves.append(files[f] + str(r))
                    break
            else:
                legal_moves.append(files[f] + str(r))
            # down/left
        for i in range(1, 9):
            f = file_num - i
            r = self.rank - i
            if f < 0 or f > 7:
                continue
            if r < 1 or r > 8:
                continue
            s = self.board_grid[files[f]][r]
            if s:
                if type(s) is EnPassant:
                    legal_moves.append(files[f] + str(r))
                    continue
                else:
                    legal_moves.append(files[f] + str(r))
                    break
            else:
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
        # up/right
        for i in range(1,9):
            f = file_num + i
            r = self.rank + i
            if f < 0 or f > 7:
                continue
            if r < 1 or r > 8:
                continue
            s = self.board_grid[files[f]][r]
            if s:
                if type(s) is EnPassant:
                    legal_moves.append(files[f] + str(r))
                    continue
                else:
                    if w != s.get_is_white():
                        legal_moves.append(files[f] + str(r))
                    break
            else:
                legal_moves.append(files[f] + str(r))

        # down/right
        for i in range(1, 9):
            f = file_num + i
            r = self.rank - i
            if f < 0 or f > 7:
                continue
            if r < 1 or r > 8:
                continue
            s = self.board_grid[files[f]][r]
            if s:
                if type(s) is EnPassant:
                    legal_moves.append(files[f] + str(r))
                    continue
                else:
                    if w != s.get_is_white():
                        legal_moves.append(files[f] + str(r))
                    break
            else:
                legal_moves.append(files[f] + str(r))
        # up/left
        for i in range(1, 9):
            f = file_num - i
            r = self.rank + i
            if f < 0 or f > 7:
                continue
            if r < 1 or r > 8:
                continue
            s = self.board_grid[files[f]][r]
            if s:
                if type(s) is EnPassant:
                    legal_moves.append(files[f] + str(r))
                    continue
                else:
                    if w != s.get_is_white():
                        legal_moves.append(files[f] + str(r))
                    break
            else:
                legal_moves.append(files[f] + str(r))
            # down/left
        for i in range(1, 9):
            f = file_num - i
            r = self.rank - i
            if f < 0 or f > 7:
                continue
            if r < 1 or r > 8:
                continue
            s = self.board_grid[files[f]][r]
            if s:
                if type(s) is EnPassant:
                    legal_moves.append(files[f] + str(r))
                    continue
                else:
                    if w != s.get_is_white():
                        legal_moves.append(files[f] + str(r))
                    break
            else:
                legal_moves.append(files[f] + str(r))

        return legal_moves
    # move(String destination) modifies the state of the board based on the location the piece is moving to (and
    # takes care of any captures that may have happened) e.g. board_grid["a"][3].move("b2")
    def move(self, f, r):
        legal_moves = self.get_legal_moves()
        if not f + str(r) in legal_moves:
            print('illegal move')
            return False
        is_capture = not self.board_grid[f][r] is None and not type(self.board_grid[f][r]) is EnPassant
        is_en_passant = False
        self.move_list.append(Move(self.is_white, 'K', self.file, self.rank, is_capture, is_en_passant, f, r))
        self.board_grid[self.file][self.rank] = None
        self.file = f
        self.rank = r
        self.board_grid[self.file][self.rank] = self
        self.has_moved = True
        return True

    def __str__(self):
        return ('b', 'w')[self.is_white] + 'B'