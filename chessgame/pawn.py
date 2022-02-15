from bishop import Bishop
from knight import Knight
from move import Move
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
        elif promo_piece == 'N':
            self.board.set_piece(Knight(self.board, self.is_white, self.file, self.rank))
        else:
            print("promotion error")
            return

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
        moveable = lambda x: x != -1 and x is None
        captureable = lambda x: x != -1 and (x is not None and opposing(x))
        add_move = lambda x, c, e, p, a: possible.append(
            Move(w, self.letter, self.file, self.rank, self.get_offset(x)[0], int(self.get_offset(x)[1]), c, e, False,
                 False, p, a))
        # add_move = lambda x: possible.append(''.join(map(str, self.get_offset(x))))
        fwd_1 = (0, (-1, 1)[w])
        fwd_2 = (0, (-2, 2)[w])
        diag_left = (-1, (-1, 1)[w])
        diag_right = (1, (-1, 1)[w])
        left = (-1, 0)
        right = (1, 0)
        promo_options = ['N', 'B', 'R', 'Q']
        promotable = self.rank == (7, 2)[not w]
        # moving forward one square
        if moveable(self.get_piece_at_offset(fwd_1)):
            if promotable:
                for promo in promo_options:
                    add_move(fwd_1, None, False, promo, None)
            else:
                add_move(fwd_1, None, False, '', None)
            # moving forward two squares
            if not self.get_has_moved() and self.rank == (7, 2)[w] and moveable(self.get_piece_at_offset(fwd_2)):
                add_move(fwd_2, None, False, '', None)
        # diagonal captures
        for i in [diag_left, diag_right]:
            s = self.get_piece_at_offset(i)
            if captureable(s):
                if promotable:
                    for promo in promo_options:
                        add_move(i, s, False, promo, None)
                else:
                    add_move(i, s, False, '', None)
        # EnPassant capture
        if self.board.move_list:
            last_m = self.board.move_list[-1]
            if last_m.letter == "P" and last_m.is_white != w:
                if self.rank == (4, 5)[w]:
                    if abs(last_m.to_rank - last_m.from_rank) == 2:
                        f = last_m.from_file
                        enemy_file_num = self.board.files.index(f)
                        file_num = self.board.files.index(self.file)
                        if file_num - enemy_file_num == 1:
                            # capture en passant to the left
                            add_move(diag_left, self.get_piece_at_offset(left), True, '', None)
                        elif file_num - enemy_file_num == -1:
                            add_move(diag_right, self.get_piece_at_offset(right), True, '', None)
        return possible

    def move(self, move):
        f = move.to_file
        r = move.to_rank
        is_capture = not self.board.get_piece(f, r) is None
        is_en_passant = move.is_en_passant
        promotion_letter = move.promotion_letter
        # if it's en passant, handle the capture
        if is_en_passant:
            self.board.remove_piece(f, (4, 5)[self.is_white])
        self.board.remove_piece(move.from_file, move.from_rank)
        self.file = f
        self.rank = r
        self.increment_num_times_moved()
        self.board.set_piece(self)

        if promotion_letter:
            self.promote(promotion_letter)

        return True
