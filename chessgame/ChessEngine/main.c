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
    uci_communication();
    /*unsigned long long movement = file[2] ^ rank[4];
    print_bitboard(movement);
    printf("\n");
    unsigned long long* square_blockers = get_blockers_rook_single_square(movement);
    for(int i = 0; i < (1 << 12); i++){
        printf("%d\n", i);
        print_bitboard(square_blockers[i]);
    }*/
    /*for(int i = 0; i < 64; i++){
        unsigned long long movement_mask = get_rook_masks(i);
        unsigned long long* blockers = get_blockers_rook_single_square(movement_mask);
        for(int j = 0; j < 1 << 14; j++){
            printf("%d %d\n", i, j);
            unsigned long long rook_legal_moves = rook_moves_single_square(i, blockers[j]);
            printf("movement_mask\n");
            print_bitboard(movement_mask);
            printf("blockers\n");
            print_bitboard(blockers[j]);
            printf("rook_legal_moves\n");
            print_bitboard(rook_legal_moves);
            printf("%d\n", i);
        }
    }*/
    return 0;
}
//get_blockers [rook_pos] [blocker_index]
//get_num_blockers [rook_pos]
//get_rook_legal_moves [rook_pos] [blocker_index]