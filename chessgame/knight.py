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
        files = ['a', 'b', 'c', 'd', 'e', 'f', 'g', 'h']
        legal_moves = []
        w = self.get_is_white()
        b = self.get_is_black()
        file_num = files.index(self.file)

        for i in range(-2, 3):
            f = file_num + i
            # if file out of bounds
            if f < 0 or f > 7:
                continue
            for j in range(-2, 3):
                r = self.rank + j
                # if rank out of bounds
                if r < 1 or r > 8:
                    continue
                # this is for every move a king can make all 8 of them
                if (f != file_num + 2 or r != self.rank + 1) and (f != file_num + 2 or r != self.rank - 1) and (
                        f != file_num - 2 or r != self.rank + 1) and (f != file_num - 2 or r != self.rank - 1) and (
                        f != file_num + 1 or r != self.rank + 2) and (f != file_num + 1 or r != self.rank - 2) and (
                        f != file_num - 1 or r != self.rank + 2) and (f != file_num - 1 or r != self.rank - 2):
                    continue

                s = self.board_grid[files[f]][r]
                # if square is occupied by a friendly piece, it's not en en passant marker, and it's friendly
                if s and not type(s) is EnPassant and w == s.get_is_white():
                    # king is blocked by its own piece
                    continue
                legal_moves.append(files[f] + str(r))
        return legal_moves

    def get_legal_moves(self):
        files = ['a', 'b', 'c', 'd', 'e', 'f', 'g', 'h']
        legal_moves = []
        w = self.get_is_white()
        b = self.get_is_black()
        file_num = files.index(self.file)

        for i in range(-2, 3):
            f = file_num + i
            # if file out of bounds
            if f < 0 or f > 7:
                continue
            for j in range(-2, 3):
                r = self.rank + j
                # if rank out of bounds
                if r < 1 or r > 8:
                    continue
                # this is for every move a king can make all 8 of them
                if (f != file_num + 2 or r != self.rank + 1) and (f != file_num + 2 or r != self.rank - 1) and (
                        f != file_num - 2 or r != self.rank + 1) and (f != file_num - 2 or r != self.rank - 1) and (
                        f != file_num + 1 or r != self.rank + 2) and (f != file_num + 1 or r != self.rank - 2) and (
                        f != file_num - 1 or r != self.rank + 2) and (f != file_num - 1 or r != self.rank - 2):
                    continue

                s = self.board_grid[files[f]][r]
                # if square is occupied by a friendly piece, it's not en en passant marker, and it's friendly
                if s and not type(s) is EnPassant and w == s.get_is_white():
                    # king is blocked by its own piece
                    continue
                legal_moves.append(files[f] + str(r))
        for i in range(len(legal_moves) - 1, -1, -1):
            f = legal_moves[i][0]
            r = int(legal_moves[i][1])
            m = Move(self.is_white, self.letter, self.file, self.rank, False, False, f, r)
            if self.your_k_check(m):
                legal_moves.pop(i)
        return legal_moves
