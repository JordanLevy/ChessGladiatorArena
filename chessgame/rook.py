import pygame

from enpassant import EnPassant
from move import Move


class Rook:

    def __init__(self, board_grid, move_list, is_white, file, rank):
        self.board_grid = board_grid
        self.board_grid[file][rank] = self
        self.move_list = move_list
        self.is_white = is_white
        if is_white:
            self.img = pygame.image.load('Images/WhiteRook.png')
        else:
            self.img = pygame.image.load('Images/BlackRook.png')
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
        return []

    # get_legal_moves(String prev_move)
    # e.g. if the previous move was Bishop to h4, board_grid["a"][3].get_legal_moves("Bh4")
    # return list of legal squares to move to (e.g. ["a1", "a2", "a3", etc.])"""
    def get_legal_moves(self):
        files = ['a', 'b', 'c', 'd', 'e', 'f', 'g', 'h']
        legal_moves = []
        w = self.get_is_white()
        b = self.get_is_black()
        file_num = files.index(self.file)

        for i in range(0, 5):
            if i == 2:
                continue
            print(i)

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

        return legal_moves

    # move(String destination) modifies the state of the board based on the location the piece is moving to (and
    # takes care of any captures that may have happened) e.g. board_grid["a"][3].move("b2")
    def move(self, f, r):
        legal_moves = self.get_legal_moves()
        if not f + str(r) in legal_moves:
            print('illegal move')
            return False
        # only pawns can capture en passant markers
        is_capture = not self.board_grid[f][r] is None and not type(self.board_grid[f][r]) is EnPassant
        is_en_passant = False
        self.move_list.append(Move(self.is_white, 'R', self.file, self.rank, is_capture, is_en_passant, f, r))
        self.board_grid[self.file][self.rank] = None
        self.file = f
        self.rank = r
        self.board_grid[self.file][self.rank] = self
        self.has_moved = True
        return True
