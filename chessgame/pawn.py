import pygame

from move import Move
from enpassant import EnPassant


class Pawn:

    def __init__(self, board_grid, move_list, is_white, file, rank):
        self.board_grid = board_grid
        self.board_grid[file][rank] = self
        self.move_list = move_list
        self.is_white = is_white
        if is_white:
            self.img = pygame.image.load('Images/WhitePawn.png')
        else:
            self.img = pygame.image.load('Images/BlackPawn.png')
        self.img = pygame.transform.scale(self.img, (50, 50))
        self.file = file
        self.rank = rank
        self.has_moved = False

    def get_img(self):
        return self.img

    def get_has_moved(self):
        return self.has_moved

    def get_file(self):
        return self.file

    def get_rank(self):
        return self.rank

    def set_file(self, file):
        self.file = file

    def set_rank(self, rank):
        self.rank = rank

    def get_is_white(self):
        return self.is_white

    def get_is_black(self):
        return not self.is_white

    def get_legal_captures(self):
        return[]

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
        self.move_list.append(Move(self.is_white, 'P', self.file, self.rank, is_capture, is_en_passant, f, r))
        # if it's en passant, handle the capture
        if is_en_passant:
            self.board_grid[f][(4, 5)[self.is_white]] = None
        # if it's a double pawn move, set an en passant marker behind it
        if (not self.has_moved) and r == (5, 4)[self.is_white]:
            self.board_grid[self.file][(6, 3)[self.is_white]] = EnPassant(self.board_grid, self.move_list,
                                                                          self.is_white, self.file,
                                                                          (6, 3)[self.is_white],
                                                                          len(self.move_list) - 1)
        self.board_grid[self.file][self.rank] = None
        self.file = f
        self.rank = r
        self.board_grid[self.file][self.rank] = self
        self.has_moved = True
        return True
