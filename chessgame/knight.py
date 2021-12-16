from enpassant import EnPassant
from move import Move
from piece import Piece


class Knight(Piece):

    def __init__(self, board, is_white, file, rank):
        super().__init__(board, is_white, file, rank)
        self.letter = 'N'
        if is_white:
            self.img = 'Images/WhiteKnight.png'
        else:
            self.img = 'Images/BlackKnight.png'

    def get_defended_squares(self):
        defended = []
        w = self.get_is_white()
        opposing = lambda x: x and w != x.get_is_white()
        moveable = lambda x: x != -1 and (x is None or type(x) is EnPassant)
        captureable = lambda x: x != -1 and (x is not None)
        add_move = lambda x: defended.append(''.join(map(str, self.get_offset(x))))
        offsets = [(1, 2), (-1, 2), (1, -2), (-1, -2), (2, 1), (2, -1), (-2, 1), (-2, -1)]

        for i in offsets:
            s = self.get_piece_at_offset(i)
            if moveable(s) or captureable(s):
                add_move(i)

        return defended

    def get_possible_moves(self):
        possible = []
        w = self.get_is_white()
        opposing = lambda x: x and w != x.get_is_white()
        moveable = lambda x: x != -1 and (x is None or type(x) is EnPassant)
        captureable = lambda x: x != -1 and (x is not None and opposing(x))
        add_move = lambda x, c, e: possible.append(Move(w, self.letter, self.file, self.rank, self.get_offset(x)[0], int(self.get_offset(x)[1]), c, e))
        offsets = [(1, 2), (-1, 2), (1, -2), (-1, -2), (2, 1), (2, -1), (-2, 1), (-2, -1)]

        for i in offsets:
            s = self.get_piece_at_offset(i)
            if moveable(s) or captureable(s):
                add_move(i, captureable(s), False)

        return possible
