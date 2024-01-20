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
    init_magic();
    uci_communication();

    return 0;
}
//get_blockers [rook_pos] [blocker_index]
//get_num_blockers [rook_pos]
//get_rook_legal_moves [rook_pos] [blocker_index]