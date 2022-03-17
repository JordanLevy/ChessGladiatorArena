from move import Move
from piece import Piece


class Queen(Piece):

    def __init__(self, board, is_white, file, rank):
        super().__init__(board, is_white, file, rank)
        self.letter = 'Q'
        if is_white:
            self.img = 'Images/WhiteQueen.png'
        else:
            self.img = 'Images/BlackQueen.png'

    def get_defended_squares(self):
        defended = []
        w = self.get_is_white()
        opposing = lambda x: x and w != x.get_is_white()
        moveable = lambda x: x != -1 and x is None
        captureable = lambda x: x != -1 and (x is not None)
        add_move = lambda x: defended.append(''.join(map(str, self.get_offset(x))))
        searching = [True, True, True, True]  # True if still searching a direction [r, l, u, d]

        for i in range(1, 9):
            offsets = [(i, 0), (-i, 0), (0, i), (0, -i)]
            for j in range(len(offsets)):
                if not searching[j]:
                    continue
                k = offsets[j]
                s = self.get_piece_at_offset(k)
                if moveable(s) or captureable(s):
                    add_move(k)
                    if captureable(s) and not (s.letter == 'K' and opposing(s)):
                        searching[j] = False
                else:
                    if s != -1 and not opposing(s):
                        searching[j] = False

        searching = [True, True, True, True]  # True if still searching a direction [ur, ul, dr, dl]

        for i in range(1, 9):
            offsets = [(i, i), (-i, i), (i, -i), (-i, -i)]
            for j in range(len(offsets)):
                if not searching[j]:
                    continue
                k = offsets[j]
                s = self.get_piece_at_offset(k)
                if moveable(s) or captureable(s):
                    add_move(k)
                    if captureable(s) and not (s.letter == 'K' and opposing(s)):
                        searching[j] = False
                else:
                    if s != -1 and not opposing(s):
                        searching[j] = False

        return defended

    def get_possible_moves(self):
        possible = []
        w = self.get_is_white()
        friendly = lambda x: x != -1 and w == x.get_is_white()
        opposing = lambda x: x and w != x.get_is_white()
        empty = lambda x: x != -1 and x is None
        enemy = lambda x: x != -1 and (x is not None and opposing(x))
        add_move = lambda x, c, e: possible.append(
            Move(w, self.letter, self.file, self.rank, self.get_offset(x)[0], int(self.get_offset(x)[1]), c, e))
        searching = [True, True, True, True, True, True, True,
                     True]  # True if still searching a direction [r, l, u, d, ur, ul, dr, dl]

        enemy_king = None
        possible_pin = -1  # -1 if queen shares no file, rank, diag with enemy king. Otherwise, 0 through 7 denoting
        # which direction to check [r, l, u, d, ur, ul, dr, dl]
        if w:
            enemy_king = self.board.black_king
        else:
            enemy_king = self.board.white_king
        file_diff = self.board.files.index(enemy_king.file) - self.board.files.index(self.file)
        rank_diff = enemy_king.rank - self.rank

        # king below queen
        if rank_diff < 0:
            # d
            if file_diff == 0:
                possible_pin = 3
            # dl
            elif file_diff == rank_diff:
                possible_pin = 7
            # dr
            elif file_diff == -rank_diff:
                possible_pin = 6
        # king above queen
        elif rank_diff > 0:
            # u
            if file_diff == 0:
                possible_pin = 2
            # ur
            elif file_diff == rank_diff:
                possible_pin = 4
            # ul
            elif file_diff == -rank_diff:
                possible_pin = 5
        # king and queen on same rank
        else:
            # l
            if file_diff < 0:
                possible_pin = 1
            # r
            if file_diff > 0:
                possible_pin = 0

        searching_pin = False
        pinned_piece = None
        offsets = [(1, 0), (-1, 0), (0, 1), (0, -1), (1, 1), (-1, 1), (1, -1), (-1, -1)]
        for i in range(len(offsets)):
            for j in range(1, 9):
                k = (offsets[i][0] * j, offsets[i][1] * j)
                s = self.get_piece_at_offset(k)
                if searching_pin:
                    if empty(s):
                        continue
                    elif enemy(s):
                        if s is enemy_king:
                            pinned_piece.pin = possible_pin
                            print(pinned_piece, possible_pin)
                        searching_pin = False
                        break
                    elif friendly(s):
                        searching_pin = False
                        break
                if empty(s):
                    add_move(k, s, False)
                elif enemy(s):
                    add_move(k, s, False)
                    if i == possible_pin:
                        searching_pin = True
                        pinned_piece = s
                    else:
                        break
                elif friendly(s):
                    break

        return possible
