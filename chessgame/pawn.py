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
            self.board_grid[self.file][self.rank] = Queen(self.board, self.is_white, self.file, self.rank)
        elif promo_piece == 'R':
            self.board_grid[self.file][self.rank] = Rook(self.board, self.is_white, self.file, self.rank)
        elif promo_piece == 'B':
            self.board_grid[self.file][self.rank] = Bishop(self.board, self.is_white, self.file, self.rank)
        else:
            self.board_grid[self.file][self.rank] = Knight(self.board, self.is_white, self.file, self.rank)

    def get_defended_squares(self):
        files = ['a', 'b', 'c', 'd', 'e', 'f', 'g', 'h']
        legal_moves = []
        w = self.get_is_white()
        b = self.get_is_black()
        file_num = files.index(self.file)

        # if it's not on the a-file
        if file_num > 0:
            legal_moves.append(files[file_num - 1] + str(self.rank + (-1, 1)[w]))

        # if it's not on the h-file
        if file_num < 7:
            legal_moves.append(files[file_num + 1] + str(self.rank + (-1, 1)[w]))

        return legal_moves

    # get_legal_moves(String prev_move)
    # e.g. if the previous move was Bishop to h4, board_grid["a"][3].get_legal_moves("Bh4")
    # return list of legal squares to move to (e.g. ["a1", "a2", "a3", etc.])"""
    def get_legal_moves(self):
        files = ['a', 'b', 'c', 'd', 'e', 'f', 'g', 'h']
        legal_moves = []
        w = self.get_is_white()
        b = self.get_is_black()
        # if it's not at the end of the board
        if (w and self.rank < 8) or (b and self.rank > 1):
            # if no piece in the way
            if not self.board_grid[self.file][self.rank + (-1, 1)[w]]:
                # move forward one
                legal_moves.append(self.file + str(self.rank + (-1, 1)[w]))
            # if it hasn't moved yet, on rank 2, and no piece in the way
            if not self.has_moved and self.rank == (7, 2)[w] and not self.board_grid[self.file][self.rank + (-1, 1)[w]] and not self.board_grid[self.file][self.rank + (-2, 2)[w]]:
                # move forward two
                legal_moves.append(self.file + str(self.rank + (-2, 2)[w]))
            file_num = files.index(self.file)

            # if it's not on the a-file
            if file_num > 0:
                left_cap = self.board_grid[files[file_num - 1]][self.rank + (-1, 1)[w]]
                # if there's an opposing piece to capture to the diagonal-left
                if left_cap and w != left_cap.get_is_white():
                    # capture diagonal-left
                    legal_moves.append(left_cap.file + str(left_cap.rank))

            # if it's not on the h-file
            if file_num < 7:
                right_cap = self.board_grid[files[file_num + 1]][self.rank + (-1, 1)[w]]
                # if there's an opposing piece to capture to the diagonal-right
                if right_cap and w != right_cap.get_is_white():
                    # capture diagonal-right
                    legal_moves.append(right_cap.file + str(right_cap.rank))
        for i in range(len(legal_moves) - 1, -1, -1):
            f = legal_moves[i][0]
            r = int(legal_moves[i][1])
            m = Move(self.is_white, self.letter, self.file, self.rank, False, False, f, r)
            if self.your_k_check(m):
                legal_moves.pop(i)
        return legal_moves

    # move(String destination) modifies the state of the board based on the location the piece is moving to (and
    # takes care of any captures that may have happened) e.g. board_grid["a"][3].move("b2")
    def move(self, f, r):
        legal_moves = self.get_legal_moves()
        if not f + str(r) in legal_moves:
            print('illegal move')
            return False
        is_capture = not self.board_grid[f][r] is None
        is_en_passant = type(self.board_grid[f][r]) is EnPassant
        is_promotion = (r == 8)
        self.move_list.append(Move(self.is_white, self.letter, self.file, self.rank, is_capture, is_en_passant, f, r))
        # if it's en passant, handle the capture
        if is_en_passant:
            self.board_grid[f][(4, 5)[self.is_white]] = None
        # if it's a double pawn move, set an en passant marker behind it
        if (not self.has_moved) and r == (5, 4)[self.is_white]:
            self.board_grid[self.file][(6, 3)[self.is_white]] = EnPassant(self.board, self.is_white, self.file, (6, 3)[self.is_white], len(self.move_list) - 1)
        self.board_grid[self.file][self.rank] = None
        self.file = f
        self.rank = r
        self.board_grid[self.file][self.rank] = self
        self.has_moved = True

        if is_promotion:
            promo_piece = ''
            while not promo_piece in ['Q', 'R', 'B', 'N']:
                print('Choose which piece to promote to (Q, R, B, N):\n')
                promo_piece = input()
            self.promote(promo_piece)
        return True
