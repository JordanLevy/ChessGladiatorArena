from enpassant import EnPassant
from king import King
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
        return legal_moves
