#include <string.h>

#ifndef BOARD_H_INCLUDED
#define BOARD_H_INCLUDED

#define CAN_CASTLE_WK 1
#define CAN_CASTLE_WQ 2
#define CAN_CASTLE_BK 4
#define CAN_CASTLE_BQ 8

// preserve board state
#define copy_board()                                             \
    unsigned char board_copy[64];                                \
    unsigned long long bitboards_copy[15];                       \
    unsigned long long white_pieces_copy = white_pieces;         \
    unsigned long long black_pieces_copy = black_pieces;         \
    unsigned long long not_white_pieces_copy = not_white_pieces; \
    unsigned long long not_black_pieces_copy = not_black_pieces; \
    unsigned long long empty_copy = empty;                       \
    unsigned long long occupied_copy = occupied;                 \
    COPY_ARRAY(board_copy, board, 64);                           \
    COPY_ARRAY(bitboards_copy, bitboards, 15);                   \
    bool white_turn_copy = white_turn;                           \
    int  enpassant_square_copy = enpassant_square;               \
    int castling_rights_copy = castling_rights;                  \
//unsigned long long hash_key_copy = hash_key;

// restore board state
//the new undo_move
#define take_back()                            \
    COPY_ARRAY(board, board_copy, 64);         \
    COPY_ARRAY(bitboards, bitboards_copy, 15); \
    white_pieces = white_pieces_copy;          \
    black_pieces = black_pieces_copy;          \
    not_white_pieces = not_white_pieces_copy;  \
    not_black_pieces = not_black_pieces_copy;  \
    empty = empty_copy;                        \
    occupied = occupied_copy;                  \
    white_turn = white_turn_copy;              \
    enpassant_square = enpassant_square_copy;  \
    castling_rights = castling_rights_copy;    \

unsigned char get_piece(int square);

void draw_board();

void remove_piece(unsigned char id, int square);

void destroy_piece(unsigned char id, int square);

unsigned char add_piece(unsigned char id, int square);

void revive_piece(unsigned char id, int square);

void move_piece(unsigned char id, int start, int end);

void reset_board();

int notation_to_number(char c, int i);

void append_move(Move *arr, Move m, int *i);

void init_fen(char *fen, size_t fen_length);

int get_file(int n);

int get_rank(int n);

int get_r_diag(int n);

int get_l_diag(int n);

bool resolves_check(int start, int end, int move_id);

void add_moves_offset(unsigned long long mask, int start_offset, int end_offset, int min_id, int max_id, MoveList* move_lists);

void add_moves_position(unsigned long long mask, int start_position, int min_id, int max_id, MoveList* move_lists);

bool white_in_check();

bool black_in_check();

bool white_in_checkmate(int numMoves);

bool black_in_checkmate(int numMoves);

void init_board(char *fen, size_t len);

bool is_legal_move(int start, int end, int promo, Move *moves, size_t n);

void incr_num_moves();

void decr_num_moves();

void flip_turns();

bool apply_move(int start, int end, int move_id);

bool get_white_check();

bool get_black_check();

bool is_game_legal_move(int start, int end, int promo);

char piece_letter(int piece_id, bool caps);

char file_letter(int n);

void init(char *fen, int len);

#endif