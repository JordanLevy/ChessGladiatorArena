#ifndef TESTING_H_INCLUDED
#define TESTING_H_INCLUDED

#include "values.h"

void print_legal_moves(struct Move *moves, int *numElems);

void print_move(struct Move move);

unsigned long long perft_test(int depth);

unsigned long long detailed_perft(int depth);

void run_game();

void print_line(struct Move *line, size_t n);

#endif
