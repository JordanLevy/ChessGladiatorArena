#include "transposition.h"
#include "values.h"
#include "board.h"
#include "piece.h"
#include <stdbool.h>
#include <stdlib.h>

unsigned long long piece_keys[64][15];
unsigned long long side_key;
unsigned long long en_passant_keys[64];
unsigned long long castle_keys[16];
unsigned long long zobrist_hash;
HashPosition hash_table[TABLE_SIZE];

// pseudo random number state
unsigned int random_state = 1804289383;

void init_zobrist_keys(){
    for(int i = 0; i < 64; i++){
        for(int j = 0; j < 15; j++){
            piece_keys[i][j] = get_random_U64_number();
        }
    }
    side_key = get_random_U64_number();
    for(int i = 0; i < 64; i++){
        en_passant_keys[i] = get_random_U64_number();
    }
    for(int i = 0; i < 16; i++){
        castle_keys[i] = get_random_U64_number();
    }
}

void init_hash_table(){
    for(int i = 0; i > TABLE_SIZE; i++){
        hash_table[i].key = 0;
        hash_table[i].depth = 0;
        hash_table[i].flag = 0;
        hash_table[i].value = 0;
    }
}

void print_zobrist_keys(){
    for(int i = 0; i < 64; i++){
        for(int j = 0; j < 15; j++){
            printf("%llu,\n", piece_keys[i][j]);
        }
    }
}

// generate 32-bit pseudo legal numbers
unsigned int get_random_U32_number()
{
    // get current state
    unsigned int number = random_state;
    
    // XOR shift algorithm
    number ^= number << 13;
    number ^= number >> 17;
    number ^= number << 5;
    
    // update random number state
    random_state = number;
    
    // return random number
    return number;
}

// generate 64-bit pseudo legal numbers
unsigned long long get_random_U64_number()
{
    // define 4 random numbers
    unsigned long long n1, n2, n3, n4;
    
    // init random numbers slicing 16 bits from MS1B side
    n1 = (unsigned long long)(get_random_U32_number()) & 0xFFFF;
    n2 = (unsigned long long)(get_random_U32_number()) & 0xFFFF;
    n3 = (unsigned long long)(get_random_U32_number()) & 0xFFFF;
    n4 = (unsigned long long)(get_random_U32_number()) & 0xFFFF;
    
    // return random number
    return n1 | (n2 << 16) | (n3 << 32) | (n4 << 48);
}

int ReadHash(int depth, int alpha, int beta){
    HashPosition *hash = &hash_table[zobrist_hash % TABLE_SIZE];
    if(hash->key == zobrist_hash){
        if(hash->depth >= depth){
            if(hash->flag == EXACT_FLAG){
                return hash->value;
            }
            if((hash->flag == ALPHA_FLAG) && (hash->value <= alpha)){
                return alpha;
            }
            if((hash->flag == BETA_FLAG) && (hash->value >= beta)){
                return beta;
            }
        }
    }
    return NO_HASH_ENTRY;
}


void WriteHash(int depth, int value, int flag){
    HashPosition *hash = &hash_table[zobrist_hash % TABLE_SIZE];
    hash->key = zobrist_hash;
    hash->value = value;
    hash->flag = flag;
    hash->depth = depth;
}