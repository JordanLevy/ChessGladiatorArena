#include "values.h"
#include "piece.h"

// turns |color(1)|type(3)|spec(4)| into |0000|color(1)|type(3)| so colortype can be used as a 0-15 index
unsigned char get_type(unsigned char id){
    return id >> ROLE_BITS_OFFSET;
}

// returns the character associated with a given piece_id
char piece_id_to_notation(unsigned char id){
    unsigned char type = get_type(id);
    if(type == wP) return 'P';
    else if(type == wN) return 'N';
    else if(type == wB) return 'B';
    else if(type == wR) return 'R';
    else if(type == wQ) return 'Q';
    else if(type == wK) return 'K';
    else if(type == bP) return 'p';
    else if(type == bN) return 'n';
    else if(type == bB) return 'b';
    else if(type == bR) return 'r';
    else if(type == bQ) return 'q';
    else if(type == bK) return 'k';
    return '_';
}

void print_piece_locations(){
    for(int i = 0; i < 256; i++){
        int location = piece_location[i];
        if(location != -1){
            printf("%d %d\n", i, location);
        }
    }
}