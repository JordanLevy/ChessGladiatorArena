class Move:

    def __init__(self, is_white, letter, from_file, from_rank, to_file, to_rank, is_capture=False, is_en_passant=False, is_short_castle=False, is_long_castle=False):
        self.is_white = is_white
        self.letter = letter  # 'K', 'Q', 'B', 'N', or 'P'
        self.from_file = from_file
        self.from_rank = from_rank
        self.to_file = to_file
        self.to_rank = to_rank
        self.is_capture = is_capture
        self.is_en_passant = is_en_passant
        self.is_short_castle = is_short_castle
        self.is_long_castle = is_long_castle

    def get_is_white(self):
        return self.is_white

    def get_is_black(self):
        return not self.is_white

    def get_letter(self):
        return self.letter

    def get_from_file(self):
        return self.from_file

    def get_from_rank(self):
        return self.from_rank

    def get_to_file(self):
        return self.to_file

    def get_to_rank(self):
        return self.to_rank

    def get_is_capture(self):
        return self.is_capture

    def get_is_en_passant(self):
        return self.is_en_passant

    def get_is_short_castle(self):
        return self.is_short_castle

    def get_is_long_castle(self):
        return self.is_long_castle

    def __eq__(self, m):
        return self.is_white == m.is_white and self.letter == m.letter and self.from_file == m.from_file and self.from_rank == m.from_rank and self.to_file == m.to_file and self.to_rank == m.to_rank

    def __str__(self):
        if self.is_short_castle:
            return 'O-O'
        if self.is_long_castle:
            return 'O-O-O'
        return self.letter + self.from_file + str(self.from_rank) + ('', 'x')[self.is_capture] + self.to_file + str(self.to_rank) + ('', '_ep')[self.is_en_passant]
