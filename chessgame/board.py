import math

from bishop import Bishop
from king import King
from knight import Knight
from move import Move
from pawn import Pawn
from queen import Queen
from rook import Rook
from gamestate import GameState
import copy

values = {"P": 100, "N": 300, "B": 330, "R": 500, "Q": 900, "K": 9000}
class Board:
    def __init__(self):
        self.turn_num = 0  # counts halfmoves. even numbers are white's turn
        self.white_turn = True  # true if it's white's turn, false otherwise
        self.white_king = None
        self.black_king = None
        self.move_preview = None
        self.board_grid = {"a": [None] * 9, "b": [None] * 9, "c": [None] * 9, "d": [None] * 9, "e": [None] * 9,
                           "f": [None] * 9,
                           "g": [None] * 9,
                           "h": [None] * 9}
        self.move_list = []
        self.fifty_move_clock = 0
        self.board_repetitions = {}  # (string fen: int #times_position_repeated), used for threefold repetition
        self.files = ['a', 'b', 'c', 'd', 'e', 'f', 'g', 'h']
        self.pieces = {"P": [], "p": [], "R": [], "r": [], "N": [], "n": [],
                       "B": [], "b": [], "Q": [], "q": [], "K": [], "k": [], "E": [], "e":[]}
        self.mat_eval = 0

    def get_piece(self, f, r):
        return self.board_grid[f][r]

    def set_piece(self, piece):
        self.board_grid[piece.file][piece.rank] = piece

    def remove_piece(self, f, r):
        self.board_grid[f][r] = None

    def remove_piece_by_ref(self, piece):
        self.board_grid[piece.file][piece.rank] = None

    def get_move_preview(self):
        return self.move_preview

    def set_move_preview(self, piece):
        self.move_preview = piece

    def get_move_num(self):
        return math.ceil(self.turn_num / 2.0)

    # increment turn and clear en passant
    def next_turn(self):
        """
        fen = self.to_fen()
        # threefold repetition only cares about pieces, turn, castling rights, and en passant
        fen = ' '.join(fen.split(' ')[:-2])
        if fen in self.board_repetitions:
            self.board_repetitions[fen] += 1
        else:
            self.board_repetitions[fen] = 1
        """
        self.turn_num += 1
        self.white_turn = not self.white_turn
        prev_move = self.move_list[-1]
        # if the previous move was a capture or a pawn move, reset the 50 move rule counter
        if prev_move.get_piece_captured() or prev_move.get_letter() == 'P':
            self.fifty_move_clock = 0
        else:
            self.fifty_move_clock += 1

    def white_in_check(self):
        return self.white_king.is_in_check()

    def black_in_check(self):
        return self.black_king.is_in_check()

    # returns a GameState based on the board position
    def is_game_over(self):
        if self.fifty_move_clock >= 100:
            return GameState.FIFTY_MOVE_RULE
        """
        for i in self.board_repetitions.keys():
            if self.board_repetitions[i] >= 3:
                return GameState.THREEFOLD_REPETITION
                """
        if self.white_turn:
            white_moves = self.get_all_legal_moves(True)
            if not white_moves:
                if self.white_in_check():
                    # black wins by checkmate
                    return GameState.BLACK_CHECKMATE
                # draw, white is in stalemate
                return GameState.STALEMATE
        else:
            black_moves = self.get_all_legal_moves(False)
            if not black_moves:
                if self.black_in_check():
                    # white wins by checkmate
                    return GameState.WHITE_CHECKMATE
                # draw, black is in stalemate
                return GameState.STALEMATE
        # game is still going
        return GameState.IN_PROGRESS

    # returns a list of all legal moves one side has
    def get_all_legal_moves(self, is_white):
        moves = []
        for i in self.files:
            for j in range(1, 9):
                s = self.get_piece(i, j)
                if not s:
                    continue
                if is_white == s.get_is_white():
                    moves.extend(s.get_legal_moves())
        return moves

    # returns a copied board
    def copy(self):
        new_board = Board()
        for i in self.files:
            for j in range(len(self.board_grid[i])):
                s = self.board_grid[i][j]
                new_piece = None
                # if there's no piece here, set it to None
                if not s:
                    new_board.remove_piece(i, j)
                elif type(s) is Pawn:
                    new_piece = Pawn(new_board, s.get_is_white(), i, j)
                elif type(s) is Knight:
                    new_piece = Knight(new_board, s.get_is_white(), i, j)
                elif type(s) is Bishop:
                    new_piece = Bishop(new_board, s.get_is_white(), i, j)
                elif type(s) is Rook:
                    new_piece = Rook(new_board, s.get_is_white(), i, j)
                elif type(s) is Queen:
                    new_piece = Queen(new_board, s.get_is_white(), i, j)
                elif type(s) is King:
                    new_piece = King(new_board, s.get_is_white(), i, j)
                    if s.get_is_white():
                        new_board.white_king = new_piece
                    else:
                        new_board.black_king = new_piece
                if new_piece:
                    new_piece.board = new_board
                    new_board.set_piece(new_piece)
        for i in range(len(self.move_list)):
            new_board.move_list.append(copy.deepcopy(self.move_list[i]))
        return new_board

    # returns a copied board with a move applied to it
    def copy_with_move(self, move):
        new_board = self.copy()
        new_board.move_by_ref(move)
        return new_board

    def setup_board(self):
        self.load_fen('rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1')
        #self.load_fen('rnbqkbnr/pp1ppppp/8/2p5/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 2')

    def draw_circle_alpha(self, pygame, surface, color, center, radius):
        target_rect = pygame.Rect(center, (0, 0)).inflate((radius * 2, radius * 2))
        shape_surf = pygame.Surface(target_rect.size, pygame.SRCALPHA)
        pygame.draw.circle(shape_surf, color, (radius, radius), radius)
        surface.blit(shape_surf, target_rect)

    def draw_board(self, pygame, screen):

        BLUE = (18, 201, 192)
        WHITE = (249, 255, 212)
        RED = (255, 0, 0)
        GREEN = (25, 166, 0, 150)
        GREY = (150, 150, 150, 150)

        moves = []
        color = ()

        # draw board squares
        w_check = self.white_in_check()
        b_check = self.black_in_check()
        square_color = BLUE
        for i in range(8):
            for j in range(8):
                square_color = WHITE
                if (i + j) % 2 == 1:
                    square_color = BLUE
                if w_check and i == self.files.index(
                        self.white_king.get_file()) and (8 - j) == self.white_king.get_rank():
                    square_color = RED
                if b_check and i == self.files.index(
                        self.black_king.get_file()) and (8 - j) == self.black_king.get_rank():
                    square_color = RED
                pygame.draw.rect(screen, square_color, (i * 50, j * 50, 50, 50))

        # draw pieces
        f = [0, "a", "b", "c", "d", "e", "f", "g", "h"]
        for i in range(1, 9):
            for j in range(1, 9):
                # if it's not equal to None
                if self.board_grid[f[i]][j]:
                    img = pygame.image.load(self.board_grid[f[i]][j].img)
                    img = pygame.transform.scale(img, (50, 50))
                    screen.blit(pygame.transform.rotate(img, 0),
                                ((i - 1) * 50, 400 - j * 50))

        if self.move_preview:
            moves = self.move_preview.get_legal_moves()
            color = (GREY, GREEN)[self.move_preview.is_white == self.white_turn]
        # draw move preview
        for m in moves:
            i = self.files.index(m.get_to_file()) + 1
            j = m.get_to_rank()
            self.draw_circle_alpha(pygame, screen, color, (i * 50 - 25, (8 - j) * 50 + 25), 10)
            if m.get_is_short_castle():
                self.draw_circle_alpha(pygame, screen, color,
                                       ((5 + 2) * 50 - 25, (8 - (8, 1)[self.move_preview.is_white]) * 50 + 25), 10)
            if m.get_is_long_castle():
                self.draw_circle_alpha(pygame, screen, color,
                                       ((5 - 2) * 50 - 25, (8 - (8, 1)[self.move_preview.is_white]) * 50 + 25), 10)

    # moves a piece on the board given a Move object
    def move_by_ref(self, move):
        s = self.get_piece(move.from_file, move.from_rank)
        c = move.get_piece_captured()
        if c:
            value_p = values[c.letter]
            if c.is_white:
                self.mat_eval -= value_p
            else:
                self.mat_eval += value_p
        is_legal = s.move(move)
        if is_legal:
            self.move_list.append(move)
        return is_legal

    # moves a piece on the board given a Move object
    def apply_move_by_ref(self, move):
        s = self.get_piece(move.from_file, move.from_rank)
        c = move.get_piece_captured()
        if c:
            value_p = values[c.letter]
            if c.is_white:
                self.mat_eval -= value_p
            else:
                self.mat_eval += value_p
        is_legal = s.move(move)
        if is_legal:
            self.move_list.append(move)
        return is_legal

    # moves a piece on the board given files and ranks
    # promotion is a character
    def apply_move(self, from_file, from_rank, to_file, to_rank, promotion):
        s = self.get_piece(from_file, from_rank)
        legal_moves = s.get_legal_moves()
        move = None
        for m in legal_moves:
            if m.to_file == to_file and m.to_rank == to_rank and m.promotion_letter == promotion:
                move = m
                break
        if move is None:
            return False
        c = move.get_piece_captured()
        if c:
            value_p = values[c.letter]
            if c.is_white:
                self.mat_eval -= value_p
            else:
                self.mat_eval += value_p
        is_legal = s.move(m)
        if is_legal:
            self.move_list.append(m)
        return is_legal

    def undo_move(self):
        # if there are no moves to undo
        if not self.move_list:
            return
        move = self.move_list[-1]
        #where the piece ended up is s
        s = self.get_piece(move.to_file, move.to_rank)
        self.remove_piece_by_ref(s)
        c = move.get_piece_captured()
        a = move.get_affected_piece()
        p = move.get_is_promotion()
        # p is a boolean if pro is true or not
        # a is a piece is piece that is afectid by the move but not captered
        # c is the piece captured on that move
        #if a:
            #self.set_piece(a)
            #this is for undoing prompted piece first
        if p:
            s = Pawn(s.board, s.is_white, s.file, s.rank)
            # this is to fixe long caseling
        if move.get_is_long_castle():
            r = s.get_piece_at_offset((1, 0))
            self.remove_piece_by_ref(r)
            r.set_file("a")
            self.set_piece(r)
        if move.get_is_short_castle():
            r = s.get_piece_at_offset((-1,0))
            self.remove_piece_by_ref(r)
            r.set_file("h")
            self.set_piece(r)
        s.set_file(move.from_file)
        s.set_rank(move.from_rank)
        self.set_piece(s)
        if c:
            self.set_piece(c)
            value_p = values[c.letter]
            # is not done
            if c.is_white:
                self.mat_eval += value_p
            else:
                self.mat_eval -= value_p
        else:
            self.remove_piece(move.to_file, move.to_rank)
        self.move_list = self.move_list[:-1]
        self.turn_num -= 1
        self.white_turn = not self.white_turn
        s.decrement_num_times_moved()

    # moves a piece on the board given files and ranks
    def move(self, from_file, from_rank, to_file, to_rank):
        s = self.get_piece(from_file, from_rank)
        legal_moves = s.get_legal_moves()
        move = None
        for m in legal_moves:
            if m.to_file == to_file and m.to_rank == to_rank:
                move = m
                break
        if move is None:
            return False
        c = move.get_piece_captured()
        if c:
            value_p = values[c.letter]
            if c.is_white:
                self.mat_eval -= value_p
            else:
                self.mat_eval += value_p
        is_legal = s.move(m)
        if is_legal:
            self.move_list.append(m)
        return is_legal

    # returns a string representation of the board using FEN (Forsyth-Edwards Notation)
    def to_fen(self):
        pieces = ''
        turn = ('b', 'w')[self.white_turn]
        ep = '-'
        castling = ''
        fifty_move_clock = str(self.fifty_move_clock)
        full_move_num = str(math.ceil((self.turn_num + 1) / 2.0))
        spaces = 0
        for j in range(8, 0, -1):
            spaces = 0
            for i in self.files:
                s = self.get_piece(i, j)
                if s is None:
                    spaces += 1
                    continue
                pieces += ('', str(spaces))[spaces > 0] + s.__str__()
                spaces = 0
            pieces += ('', str(spaces))[spaces > 0] + ('', '/')[j > 1]
        if not self.white_king.get_num_times_moved():
            rook = self.get_piece('h', 1)
            if rook and type(rook) is Rook and not rook.get_num_times_moved():
                castling += 'K'
            rook = self.get_piece('a', 1)
            if rook and type(rook) is Rook and not rook.get_num_times_moved():
                castling += 'Q'
        if not self.black_king.get_num_times_moved():
            rook = self.get_piece('h', 8)
            if rook and type(rook) is Rook and not rook.get_num_times_moved():
                castling += 'k'
            rook = self.get_piece('a', 8)
            if rook and type(rook) is Rook and not rook.get_num_times_moved():
                castling += 'q'
        if not castling:
            castling = '-'
        # EnPassant capture
        w = self.white_turn
        if self.move_list:
            last_m = self.move_list[-1]
            if last_m.letter == "P":
                if last_m.to_rank == (5, 4)[w]:
                    if abs(last_m.to_rank - last_m.from_rank) == 2:
                        f = last_m.from_file
                        r = str(last_m.from_rank + (-1, 1)[w])
                        ep = f + r
        return pieces + ' ' + turn + ' ' + castling + ' ' + ep + ' ' + fifty_move_clock + ' ' + full_move_num

    # sets up the board position using a FEN (Forsyth-Edwards Notation) string
    def load_fen(self, fen):
        pieces, turn, castling, ep, fifty_move_clock, full_move_num = fen.split(' ')
        self.white_turn = (turn == 'w')
        self.fifty_move_clock = int(fifty_move_clock)
        self.turn_num = 2 * int(full_move_num) - 1
        if ep != '-':
            file = ep[0]
            rank = int(ep[1])
            white_pawn = not self.white_turn
            ep_move = Move(white_pawn, 'P', file, rank + (1, -1)[white_pawn], file, rank + (-1, 1)[white_pawn], None, False, False, False, '', None)
            self.move_list.append(ep_move)
        # if ep != '-':
        #     self.set_piece(EnPassant(self, not self.white_turn, ep[0], int(ep[1]), self.turn_num))
        f = 0
        r = 8
        for p in pieces:
            if p.isnumeric():
                f += int(p)
                continue
            if p == '/':
                f = 0
                r -= 1
                continue
            w = p.isupper()
            letter = p.upper()
            new_piece = None
            file = self.files[f]
            if letter == 'P':
                new_piece = Pawn(self, w, file, r)
            elif letter == 'N':
                new_piece = Knight(self, w, file, r)
            elif letter == 'B':
                new_piece = Bishop(self, w, file, r)
            elif letter == 'R':
                new_piece = Rook(self, w, file, r)
                if file == 'h' and w:
                    new_piece.set_num_times_moved('K' not in castling)
                elif file == 'a' and w:
                    new_piece.set_num_times_moved('Q' not in castling)
                elif file == 'h' and not w:
                    new_piece.set_num_times_moved('k' not in castling)
                elif file == 'a' and not w:
                    new_piece.set_num_times_moved('q' not in castling)
            elif letter == 'Q':
                new_piece = Queen(self, w, file, r)
            elif letter == 'K':
                new_piece = King(self, w, file, r)
                if w:
                    self.white_king = new_piece
                else:
                    self.black_king = new_piece
            if new_piece:
                self.set_piece(new_piece)
                self.pieces[new_piece.__str__()].append(new_piece)
            f += 1

    def __str__(self):
        a = ''
        for j in range(8, 0, -1):
            a += '|'
            for i in self.files:
                s = self.get_piece(i, j)
                if s is None:
                    a += ' ' + '|'
                    continue
                a += s.__str__() + '|'
            a += '\n'
        return a
