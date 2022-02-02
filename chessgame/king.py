from enpassant import EnPassant
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
                if not s or type(s) is EnPassant:
                    continue
                if (self.is_white != s.get_is_white()) and (
                        target in s.get_defended_squares()):
                    return True
        return False

    def get_defended_squares(self):
        defended = []
        w = self.get_is_white()
        opposing = lambda x: x and w != x.get_is_white()
        moveable = lambda x: x != -1 and (x is None or type(x) is EnPassant)
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
        moveable = lambda x: x != -1 and (x is None or type(x) is EnPassant)
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

    def move(self, move, check_legality=True):
        if check_legality:
            if not self.is_legal_move(move):
                print('illegal move')
                return False
        f = move.to_file
        r = move.to_rank
        self.board.remove_piece(self.file, self.rank)
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

    """
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
                s = self.board.get_piece(files[f], r)
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

            rook = self.board.get_piece('a', self.rank)
            # if there's a rook on a1 and it hasn't moved
            if rook and type(rook) is Rook and not rook.get_has_moved():
                # check for pieces in the way
                piece_blocking = False
                through_check = False
                for i in ['b', 'c', 'd']:
                    s = self.board.get_piece(i, self.rank)
                    if s:
                        piece_blocking = True
                for i in ['c', 'd', 'e']:
                    if self.defended_by_enemy(i, self.rank):
                        through_check = True
                # if there are no pieces in the way of long castle
                if not piece_blocking and not through_check:
                    # add long castle to legal moves
                    legal_moves.append('O-O-O')

            rook = self.board.get_piece('h', self.rank)
            # if there's a rook on h1 and it hasn't moved
            if rook and type(rook) is Rook and not rook.get_has_moved():
                # check for pieces in the way
                piece_blocking = False
                through_check = False
                for i in ['f', 'g']:
                    s = self.board.get_piece(i, self.rank)
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
    def move(self, f, r, check_legality=True):
        legal_moves = []
        if check_legality:
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
                        rook = self.board.get_piece('a', self.rank)
                        king = self.board.get_piece('e', self.rank)
                        self.board.remove_piece_by_ref(rook)
                        self.board.remove_piece_by_ref(king)
                        rook.set_file('d')
                        king.set_file('c')
                        self.board.set_piece(rook)
                        self.board.set_piece(king)
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
                        rook = self.board.get_piece('h', self.rank)
                        king = self.board.get_piece('e', self.rank)
                        self.board.remove_piece_by_ref(rook)
                        self.board.remove_piece_by_ref(king)
                        rook.set_file('f')
                        king.set_file('g')
                        self.board.set_piece(rook)
                        self.board.set_piece(king)
                        self.has_moved = True
                        self.has_moved = True
                        return True
            print('illegal move')
            return False
        # only pawns can capture en passant markers
        is_capture = not self.board.get_piece(f, r) is None and not type(self.board.get_piece(f, r)) is EnPassant
        is_en_passant = False
        self.move_list.append(Move(self.is_white, 'K', self.file, self.rank, is_capture, is_en_passant, f, r))
        self.board.remove_piece_by_ref(self)
        self.file = f
        self.rank = r
        self.board.set_piece(self)
        self.has_moved = True
        return True"""
