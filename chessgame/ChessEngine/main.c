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
    return 0;
}
