#ifndef TRANSPOSITION_H_INCLUDED
#define TRANSPOSITION_H_INCLUDED

#include "values.h"

#define UNASSIGNED_FLAG -1
#define EXACT_FLAG 0
#define ALPHA_FLAG 1
#define BETA_FLAG 2

typedef struct NullableInt {
    bool isNull;
    int value;
} NullableInt;

typedef struct HashPosition {
    unsigned long long key;
    int depth;
    int flag;
    int value;
    Move best;
} HashPosition;

extern unsigned long long zobrist_hash;
extern unsigned long long zobrist_keys[64][15];

void init_zobrist_keys();

void init_hash_table();

void print_zobrist_keys();

unsigned int get_random_U32_number();

unsigned long long get_random_U64_number();

NullableInt ProbeHash(int depth, int alpha, int beta, Move move);

void RecordHash(int depth, int value, int flag);

void SetBestMove(Move move);

#endif