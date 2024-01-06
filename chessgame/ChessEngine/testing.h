#ifndef TESTING_H_INCLUDED
#define TESTING_H_INCLUDED

#include "values.h"

void print_legal_moves(Move *moves, int *numMoves);

void print_move(Move move);

unsigned long long perft_test(int depth);

unsigned long long detailed_perft(int depth);

void run_game();

void print_line(Move *line, size_t n);

unsigned long long rook_moves_single_square(int square, unsigned long long blockers);

unsigned long long *get_blockers_rook_single_square(unsigned long long movement);

unsigned long long get_rook_masks(int square);

void write_rook_moves_lookup_to_file();

bool is_valid_rook_magic_number(int square, int magic_number, int shift);

bool generate_rook_magic_numbers();

int get_index_from_magic(unsigned long long blocker, int magic_number, int shift);

#endif
