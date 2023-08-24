#ifndef TRANSPOSITION_H_INCLUDED
#define TRANSPOSITION_H_INCLUDED

#define TABLE_SIZE 6972593
#define NO_HASH_ENTRY 197483537
#define EXACT_FLAG 0
#define ALPHA_FLAG 1
#define BETA_FLAG 2

typedef struct HashPosition {
    unsigned long long key;
    int depth;
    int flag;
    int value;
} HashPosition;

extern unsigned long long piece_keys[64][15];
extern unsigned long long side_key;
extern unsigned long long en_passant_keys[64];
extern unsigned long long castle_keys[16];
extern unsigned long long zobrist_hash;

void init_zobrist_keys();

void init_hash_table();

void print_zobrist_keys();

unsigned int get_random_U32_number();

unsigned long long get_random_U64_number();

int ReadHash(int depth, int alpha, int beta);

void print_table_entry();

void WriteHash(int depth, int value, int flag);

#endif