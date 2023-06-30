#include "values.h"
#include "board.h"
#include "piece.h"

// returns the piece residing on a square (0-63)
unsigned char get_piece(int square){
    return board[square];
}

void draw_board(){
    for(int i = 63; i >= 0; i--){
        unsigned char id = get_piece(i);
        printf("|");
        printf("%c", piece_id_to_notation(id));
        if(i % 8 == 0){
            printf("|\n");
        }
    }
}

