import pygame

from enpassant import EnPassant
from move import Move
from piece import Piece


class Rook(Piece):

    def __init__(self, board, is_white, file, rank):
        super().__init__(board, is_white, file, rank)
        self.letter = 'R'
        if is_white:
            self.img = 'Images/WhiteRook.png'
        else:
            self.img = 'Images/BlackRook.png'

    def get_defended_squares(self):
        files = ['a', 'b', 'c', 'd', 'e', 'f', 'g', 'h']
        legal_moves = []
        w = self.get_is_white()
        b = self.get_is_black()
        file_num = files.index(self.file)

        # left/right
        for k in [range(file_num - 1, -1, -1), range(file_num + 1, 8)]:
            for i in k:
                s = self.board_grid[files[i]][self.rank]
                # if there's something on this square
                if s:
                    # if it's an en passant marker
                    if type(s) is EnPassant or (s.letter == 'K' and w != s.get_is_white()):
                        # add move and keep looking
                        legal_moves.append(files[i] + str(self.rank))
                        continue
                    # if it's a piece
                    else:
                        # add the move
                        legal_moves.append(files[i] + str(self.rank))
                        # stop looking
                        break
                # if it's an empty square
                else:
                    # add the move and keep looking
                    legal_moves.append(files[i] + str(self.rank))

        # down/up
        for k in [range(self.rank - 1, 0, -1), range(self.rank + 1, 9)]:
            for i in k:
                s = self.board_grid[self.file][i]
                if s:
                    if type(s) is EnPassant or (s.letter == 'K' and w != s.get_is_white()):
                        legal_moves.append(self.file + str(i))
                        continue
                    else:
                        legal_moves.append(self.file + str(i))
                        break
                else:
                    legal_moves.append(self.file + str(i))

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

        # left/right
        for k in [range(file_num - 1, -1, -1), range(file_num + 1, 8)]:
            for i in k:
                s = self.board_grid[files[i]][self.rank]
                # if there's something on this square
                if s:
                    # if it's an en passant marker
                    if type(s) is EnPassant:
                        # add move and keep looking
                        legal_moves.append(files[i] + str(self.rank))
                        continue
                    # if it's a piece
                    else:
                        # if it's an opposing piece
                        if w != s.get_is_white():
                            # add the move
                            legal_moves.append(files[i] + str(self.rank))
                        # stop looking
                        break
                # if it's an empty square
                else:
                    # add the move and keep looking
                    legal_moves.append(files[i] + str(self.rank))

        # down/up
        for k in [range(self.rank - 1, 0, -1), range(self.rank + 1, 9)]:
            for i in k:
                s = self.board_grid[self.file][i]
                if s:
                    if type(s) is EnPassant:
                        legal_moves.append(self.file + str(i))
                        continue
                    else:
                        if w != s.get_is_white():
                            legal_moves.append(self.file + str(i))
                        break
                else:
                    legal_moves.append(self.file + str(i))

        return legal_moves
