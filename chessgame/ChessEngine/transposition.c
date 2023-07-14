#include "transposition.h"
#include "values.h"
#include <stdbool.h>
#include <stdlib.h>

unsigned long long zobrist_keys[64][15];
unsigned long long zobrist_hash;

// pseudo random number state
unsigned int random_state = 1804289383;

void init_zobrist_keys(){
    for(int i = 0; i < 64; i++){
        for(int j = 0; j < 15; j++){
            zobrist_keys[i][j] = get_random_U64_number();
        }
    }
}

void print_zobrist_keys(){
    for(int i = 0; i < 64; i++){
        for(int j = 0; j < 15; j++){
            printf("%llu\n", zobrist_keys[i][j]);
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
