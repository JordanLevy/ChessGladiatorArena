class Move:

    def __init__(self, is_white, piece, from_file, from_rank, is_capture, is_en_passant, to_file, to_rank):
        self.is_white = is_white
        self.piece = piece  # 'K', 'Q', 'B', 'N', or 'P'
        self.from_file = from_file
        self.from_rank = from_rank
        self.is_capture = is_capture
        self.is_en_passant = is_en_passant
        self.to_file = to_file
        self.to_rank = to_rank

    def get_is_white(self):
        return self.is_white

    def get_is_black(self):
        return not self.is_white

    def get_piece(self):
        return self.piece

    def get_from_file(self):
        return self.from_file

    def get_from_rank(self):
        return self.from_rank

    def get_is_capture(self):
        return self.is_capture

    def get_is_en_passant(self):
        return self.is_en_passant

    def get_to_file(self):
        return self.to_file

    def get_to_rank(self):
        return self.to_rank

    def __str__(self):
        return self.piece + self.from_file + str(self.from_rank) + ('', 'x')[self.is_capture] + self.to_file + str(self.to_rank) + ('', '_ep')[self.is_en_passant]
