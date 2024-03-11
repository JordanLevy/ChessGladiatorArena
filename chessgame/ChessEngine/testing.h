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

void removeLastPathComponent(char* path);

FILE* openFileInProjectFolder(const char* filename, const char* mode);

void write_rook_moves_lookup_to_file(unsigned long long* magic, int* shift);

bool is_valid_rook_magic_number(int square, unsigned long long magic_number, int shift);

unsigned long long find_single_rook_magic_number(int square, int shift, int num_iterations);

void generate_rook_magic_numbers(int min_shift, int min_num_iterations, unsigned long long* result_magic, int* result_shift, int amount_run, int t_limit);

int get_index_from_magic(unsigned long long blocker, unsigned long long magic_number, int shift);

#endif
