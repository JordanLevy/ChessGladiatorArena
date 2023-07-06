#ifndef BITWISE_H_INCLUDED
#define BITWISE_H_INCLUDED


unsigned long long r_shift(unsigned long long x, int n);

unsigned long long l_shift(unsigned long long x, int n);

unsigned long long generate_bitboard(int squares[], int num_squares);

unsigned long long generate_bitboard_from_range(int a, int b);

unsigned long long reverse(unsigned long long i);

int leading_zeros(unsigned long long i);

void print_bitboard(unsigned long long bitboard);

unsigned long long offset_span(unsigned long long span, int origin, int i);

unsigned long long remove_span_warps(unsigned long long span, int i);

#endif