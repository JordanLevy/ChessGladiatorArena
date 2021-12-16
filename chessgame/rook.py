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
        defended = []
        w = self.get_is_white()
        opposing = lambda x: x and w != x.get_is_white()
        moveable = lambda x: x != -1 and (x is None or type(x) is EnPassant)
        captureable = lambda x: x != -1 and (x is not None)
        add_move = lambda x: defended.append(''.join(map(str, self.get_offset(x))))
        searching = [True, True, True, True]  # True if still searching a direction [ur, ul, dr, dl]

        for i in range(1, 9):
            offsets = [(i, 0), (-i, 0), (0, i), (0, -i)]
            for j in range(len(offsets)):
                if not searching[j]:
                    continue
                k = offsets[j]
                s = self.get_piece_at_offset(k)
                if moveable(s) or captureable(s):
                    add_move(k)
                    # x-ray the opposing king
                    if captureable(s) and not (s.letter == 'K' and opposing(s)):
                        searching[j] = False
                else:
                    # defend your own piece and stop looking
                    if s != -1 and not opposing(s):
                        add_move(k)
                        searching[j] = False

        return defended

    def get_possible_moves(self):
        possible = []
        w = self.get_is_white()
        opposing = lambda x: x and w != x.get_is_white()
        moveable = lambda x: x != -1 and (x is None or type(x) is EnPassant)
        captureable = lambda x: x != -1 and (x is not None and opposing(x))
        add_move = lambda x, c, e: possible.append(Move(w, self.letter, self.file, self.rank, self.get_offset(x)[0], int(self.get_offset(x)[1]), c, e))
        searching = [True, True, True, True] # True if still searching a direction [r, l, u, d]

        for i in range(1, 9):
            offsets = [(i, 0), (-i, 0), (0, i), (0, -i)]
            for j in range(len(offsets)):
                if not searching[j]:
                    continue
                k = offsets[j]
                s = self.get_piece_at_offset(k)
                if moveable(s) or captureable(s):
                    add_move(k, captureable(s), False)
                    if captureable(s):
                        searching[j] = False
                else:
                    if s != -1 and not opposing(s):
                        searching[j] = False

        return possible
