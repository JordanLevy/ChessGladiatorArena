#ifndef PIECE_H_INCLUDED
#define PIECE_H_INCLUDED

#include <stdbool.h>

extern unsigned long long** rook_moves_lookup;
extern unsigned long long* rook_magic_numbers;
extern unsigned long long rook_masks[64];
extern int* rook_magic_shift;

extern unsigned long long** bishop_moves_lookup;
extern unsigned long long* bishop_magic_numbers;
extern unsigned long long bishop_masks[64];
extern int* bishop_magic_shift;

unsigned char get_type(unsigned char id);

char piece_id_to_notation(unsigned char id);

void print_piece_locations();

unsigned char letter_to_piece_type(char letter);

int get_num_identical_pieces(unsigned char id);

unsigned long long span_piece(unsigned long long mask, int i, unsigned long long span, int origin, unsigned long long king_bb);

unsigned long long line_between_pieces(unsigned long long direction, int piece_1, int piece_2);

unsigned long long line_attack(unsigned long long o, unsigned long long m, unsigned long long s);

bool is_white_piece(int id);

bool is_black_piece(int id);

void init_magic();

unsigned long long sliding_piece(unsigned long long mask, int i, unsigned long long blockers, bool rook_moves, bool bishop_moves, unsigned long long king_bb);

void possible_P(unsigned long long bb, unsigned long long can_capture, unsigned long long promo_rank, unsigned long long enemy_pawns, unsigned long long double_push_rank, int fwd, unsigned char color, MoveList *move_lists);

void possible_wP(unsigned long long bb, MoveList *move_lists);

void possible_bP(unsigned long long bb, MoveList *move_lists);

void possible_N(unsigned long long bb, unsigned long long mask, unsigned char color, MoveList *move_lists);

void possible_B(unsigned long long bb, unsigned long long mask, unsigned char color, MoveList *move_lists);

void possible_R(unsigned long long bb, unsigned long long mask, unsigned char color, MoveList *move_lists);

void possible_Q(unsigned long long bb, unsigned long long mask, unsigned char color, MoveList *move_lists);

void possible_K(unsigned long long bb, unsigned long long mask, unsigned char color, MoveList *move_lists);

void update_piece_masks();

void possible_moves_white(MoveList *move_lists);

void possible_moves_black(MoveList *move_lists);

void update_possible_moves(MoveList *move_lists);

void update_game_possible_moves();

void apply_rook_move(unsigned char id);

bool apply_castling(unsigned char id, int start, int end);

void undo_rook_move(unsigned char id);

void undo_castling(unsigned char id, int start, int end);

#endif