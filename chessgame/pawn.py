from bishop import Bishop
from knight import Knight
from move import Move
from enpassant import EnPassant
from piece import Piece
from queen import Queen
from rook import Rook


class Pawn(Piece):

    def __init__(self, board, is_white, file, rank):
        super().__init__(board, is_white, file, rank)
        self.letter = 'P'
        if is_white:
            self.img = 'Images/WhitePawn.png'
        else:
            self.img = 'Images/BlackPawn.png'

    def promote(self, promo_piece):
        if promo_piece == 'Q':
            self.board.set_piece(Queen(self.board, self.is_white, self.file, self.rank))
        elif promo_piece == 'R':
            self.board.set_piece(Rook(self.board, self.is_white, self.file, self.rank))
        elif promo_piece == 'B':
            self.board.set_piece(Bishop(self.board, self.is_white, self.file, self.rank))
        else:
            self.board.set_piece(Knight(self.board, self.is_white, self.file, self.rank))

    def get_defended_squares(self):
        files = ['a', 'b', 'c', 'd', 'e', 'f', 'g', 'h']
        defended = []
        w = self.get_is_white()
        b = self.get_is_black()
        add_move = lambda x: defended.append(''.join(map(str, self.get_offset(x))))
        file_num = files.index(self.file)
        diag_left = (-1, (-1, 1)[w])
        diag_right = (1, (-1, 1)[w])

        if file_num > 0:
            add_move(diag_left)
        if file_num < 7:
            add_move(diag_right)
        return defended

    def get_possible_moves(self):
        possible = []
        w = self.get_is_white()
        opposing = lambda x: x and w != x.get_is_white()
        moveable = lambda x: x != -1 and (x is None or type(x) is EnPassant)
        captureable = lambda x: x != -1 and (x is not None and opposing(x))
        add_move = lambda x, c, e: possible.append(Move(w, self.letter, self.file, self.rank, self.get_offset(x)[0], int(self.get_offset(x)[1]), c, e))
        # add_move = lambda x: possible.append(''.join(map(str, self.get_offset(x))))
        fwd_1 = (0, (-1, 1)[w])
        fwd_2 = (0, (-2, 2)[w])
        diag_left = (-1, (-1, 1)[w])
        diag_right = (1, (-1, 1)[w])

        # moving forward one square
        if moveable(self.get_piece_at_offset(fwd_1)):
            add_move(fwd_1, False, False)
            # moving forward two squares
            if not self.has_moved and self.rank == (7, 2)[w] and moveable(self.get_piece_at_offset(fwd_2)):
                add_move(fwd_2, False, False)
        # diagonal captures
        for i in [diag_left, diag_right]:
            s = self.get_piece_at_offset(i)
            if captureable(s):
                add_move(i, True, type(s) is EnPassant)

        return possible

    def move(self, move, check_legality=True):
        if check_legality:
            if not self.is_legal_move(move):
                print('illegal move')
                return False
        f = move.to_file
        r = move.to_rank
        is_capture = not self.board.get_piece(f, r) is None
        is_en_passant = type(self.board.get_piece(f, r)) is EnPassant
        is_promotion = (r == 8)
        self.move_list.append(move)
        # if it's en passant, handle the capture
        if is_en_passant:
            self.board.remove_piece(f, (4, 5)[self.is_white])
        # if it's a double pawn move, set an en passant marker behind it
        if (not self.has_moved) and r == (5, 4)[self.is_white]:
            self.board.set_piece(
                EnPassant(self.board, self.is_white, self.file, (6, 3)[self.is_white], self.board.turn_num))
        self.board.remove_piece(self.file, self.rank)
        self.file = f
        self.rank = r
        self.has_moved = True
        self.board.set_piece(self)

        if is_promotion:
            promo_piece = ''
            while not promo_piece in ['Q', 'R', 'B', 'N']:
                print('Choose which piece to promote to (Q, R, B, N):\n')
                promo_piece = input()
            self.promote(promo_piece)
        return True
