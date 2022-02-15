from move import Move
from piece import Piece
from rook import Rook


class King(Piece):

    def __init__(self, board, is_white, file, rank):
        super().__init__(board, is_white, file, rank)
        self.letter = 'K'
        if is_white:
            self.img = 'Images/WhiteKing.png'
        else:
            self.img = 'Images/BlackKing.png'

    def is_in_check(self):
        return self.defended_by_enemy(self.file, self.rank)

    def defended_by_enemy(self, f, r):
        files = ['a', 'b', 'c', 'd', 'e', 'f', 'g', 'h']
        target = f + str(r)
        for i in range(0, 8):
            for j in range(1, 9):
                s = self.board.get_piece(files[i], j)
                if not s:
                    continue
                if (self.is_white != s.get_is_white()) and (
                        target in s.get_defended_squares()):
                    return True
        return False

    def get_defended_squares(self):
        defended = []
        w = self.get_is_white()
        opposing = lambda x: x and w != x.get_is_white()
        moveable = lambda x: x != -1 and x is None
        captureable = lambda x: x != -1 and (x is not None)
        add_move = lambda x: defended.append(''.join(map(str, self.get_offset(x))))

        for i in range(-1, 2):
            for j in range(-1, 2):
                if i == 0 and j == 0:
                    continue
                s = self.get_piece_at_offset((i, j))
                if moveable(s) or captureable(s):
                    add_move((i, j))

        return defended

    def get_possible_moves(self):
        possible = []
        w = self.get_is_white()
        opposing = lambda x: x and w != x.get_is_white()
        moveable = lambda x: x != -1 and x is None
        captureable = lambda x: x != -1 and (x is not None and opposing(x))
        add_move = lambda x, c, e, s, l: possible.append(Move(w, self.letter, self.file, self.rank, self.get_offset(x)[0], int(self.get_offset(x)[1]), c, e, s, l))

        # regular king movement
        for i in range(-1, 2):
            for j in range(-1, 2):
                if i == 0 and j == 0:
                    continue
                s = self.get_piece_at_offset((i, j))
                if moveable(s) or captureable(s):
                    add_move((i, j), s, False, False, False)

        # short castling
        # king cannot have moved
        short_castle = not self.get_has_moved()
        # h1/8 rook can't have moved
        s = self.board.get_piece('h', self.rank)
        if not (s and type(s) is Rook and not s.num_times_moved):
            short_castle = False
        # f1/8 and g1.8 must be empty and undefended
        for i in ['f', 'g']:
            s = self.board.get_piece(i, self.rank)
            if s or self.defended_by_enemy(i, self.rank):
                short_castle = False
        if short_castle:
            add_move((2, 0), None, False, True, False)

        # long castling
        # king cannot have moved
        long_castle = not self.get_has_moved()
        # a1/8 rook can't have moved
        s = self.board.get_piece('a', self.rank)
        if not (s and type(s) is Rook and not s.num_times_moved):
            long_castle = False
        # c1/8 and d1/8 must be empty and undefended
        for i in ['c', 'd']:
            s = self.board.get_piece(i, self.rank)
            if s or self.defended_by_enemy(i, self.rank):
                long_castle = False
        # b1/8 only has to be empty
        s = self.board.get_piece('b', self.rank)
        if s:
            long_castle = False
        if long_castle:
            add_move((-2, 0), None, False, False, True)

        return possible

    def move(self, move):
        f = move.to_file
        r = move.to_rank
        self.board.remove_piece(move.from_file, move.from_rank)
        self.file = f
        self.rank = r
        self.increment_num_times_moved()
        self.board.set_piece(self)
        if move.is_short_castle:
            s = self.board.get_piece('h', self.rank)
            self.board.remove_piece_by_ref(s)
            s.file = 'f'
            self.board.set_piece(s)
        if move.is_long_castle:
            s = self.board.get_piece('a', self.rank)
            self.board.remove_piece_by_ref(s)
            s.file = 'd'
            self.board.set_piece(s)
        return True