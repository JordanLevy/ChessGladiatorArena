import pygame

from enpassant import EnPassant
from move import Move
from piece import Piece


class Bishop(Piece):

    def __init__(self, board, is_white, file, rank):
        super().__init__(board, is_white, file, rank)
        self.letter = 'B'
        if is_white:
            self.img = 'Images/WhiteBishop.png'
        else:
            self.img = 'Images/BlackBishop.png'

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
                if type(s) is EnPassant or (s.letter == 'K' and w != s.get_is_white()):
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
                if type(s) is EnPassant or (s.letter == 'K' and w != s.get_is_white()):
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
                if type(s) is EnPassant or (s.letter == 'K' and w != s.get_is_white()):
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
                if type(s) is EnPassant or (s.letter == 'K' and w != s.get_is_white()):
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
        for i in range(len(legal_moves) - 1, -1, -1):
            f = legal_moves[i][0]
            r = int(legal_moves[i][1])
            m = Move(self.is_white, self.letter, self.file, self.rank, False, False, f, r)
            if self.your_k_check(m):
                legal_moves.pop(i)
        return legal_moves
