#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <limits.h>
#include <time.h>
#include <math.h>
#include <ctype.h>
#include "values.h"
#include "board.h"
#include "piece.h"
#include "bitwise.h"
#include "testing.h"
#include "uci.h"
#include "transposition.h"

int main(){
    init_zobrist_keys();
    init_hash_table();
    init_masks();
    init_magic();
    unsigned long long* result_magic = (unsigned long long*)calloc(64, sizeof(unsigned long long));
    int* result_shift = (int*)calloc(64, sizeof(int));
    generate_rook_magic_numbers(44, 1000, result_magic, result_shift, 3);
    for(int i = 0; i < 64; i++){
        printf("%d magic number: %llu, shift: %d\n", i, result_magic[i], result_shift[i]);
    }
    write_rook_moves_lookup_to_file(result_magic, result_shift);
    free(result_magic);
    free(result_shift);
    //uci_communication();

    return 0;
}
//get_blockers [rook_pos] [blocker_index]
//get_num_blockers [rook_pos]
//get_rook_legal_moves [rook_pos] [blocker_index]