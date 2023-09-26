#include "testing.h"
#include "piece.h"
#include "values.h"
#include "board.h"
#include <stdlib.h>
#include <string.h>

void print_legal_moves(Move* moves, int *numMoves){
    for(int i = 0; i < (*numMoves); i++){
        Move move = moves[i];
        int s = move.start;
        int e = move.end;
        int m = move.move_id;
        printf("%d:\t", i);

        char file = file_letter(7 - get_file(s));
        int rank = get_rank(s) + 1;
        printf("%c%c%d", piece_letter(get_piece(s), true), file, rank);

        file = file_letter(7 - get_file(e));
        rank = get_rank(e) + 1;
        printf("%c%d\t%d\n", file, rank, m);
    }
}

void print_move(Move move){
    int s = move.start;
    /*if(s == -1){
        return;
    }*/
    int e = move.end;
    //int m = move.id;

    char file = file_letter(7 - get_file(s));
    int rank = get_rank(s) + 1;
    printf("%c%c%d", piece_letter(move.piece_id, true), file, rank);

    file = file_letter(7 - get_file(e));
    rank = get_rank(e) + 1;
    printf("%c%d", file, rank);
}

unsigned long long perft_test(int depth){
    if(depth == 0){
        return 1ULL;
    }

    MoveList* move_lists = (MoveList*)malloc(1 * sizeof(MoveList));
    move_lists[ALL].size = 0; 
    move_lists[ALL].moves = (Move*)malloc(80 * sizeof(Move));

    update_possible_moves(move_lists);

    unsigned long long num_positions = 0ULL;
    Move move;

    for(int i = 0; i < move_lists[ALL].size; i++){
        move = move_lists[ALL].moves[i];
        apply_move(move.start, move.end, move.move_id);
        num_positions += perft_test(depth - 1);
        undo_move();
        decr_num_moves();
        flip_turns();
        update_piece_masks();
    }

        free(move_lists[ALL].moves);
        free(move_lists);

    return num_positions;
}

unsigned long long detailed_perft(int depth){

    MoveList* move_lists = (MoveList*)malloc(1 * sizeof(MoveList));
    move_lists[ALL].size = 0; 
    move_lists[ALL].moves = (Move*)malloc(80 * sizeof(Move));

    update_possible_moves(move_lists);

    unsigned long long num_positions = 0ULL;
    Move move;
    int n;
    int s;
    int e;
    int m;
    char file;
    int rank;

    for(int i = 0; i < move_lists[ALL].size; i++){

        move = move_lists[ALL].moves[i];
        apply_move(move.start, move.end, move.move_id);
        n = perft_test(depth - 1);
        num_positions += n;
        s = move.start;
        e = move.end;
        m = move.move_id;

        file = file_letter(7 - get_file(s));
        rank = get_rank(s) + 1;
        printf("%c%c%d", piece_letter(move.piece_id, true), file, rank);

        file = file_letter(7 - get_file(e));
        rank = get_rank(e) + 1;
        printf("%c%d\t%d\t%d\n", file, rank, m, n);
        undo_move();
        decr_num_moves();
        flip_turns();
    }

        free(move_lists[ALL].moves);
        free(move_lists);

    return num_positions;
}


void print_line(Move* line, size_t n){
    for(int i = 1; i <= n; i++){
        print_move(line[i]);
        printf(" ");
    }
    printf("\n");
}

unsigned long long rook_moves_single_square(int square, unsigned long long blockers){
    return sliding_piece(~0ULL, square, blockers, true, false, 0ULL);
}

unsigned long long* get_blockers_rook_single_square(unsigned long long movement){
    int* moveSquares = (int*)calloc(14, sizeof(int));
    int numMoveSquares = 0;
    for(int i = 0; i < 64; i++){
        if(((movement >> i) & 1) == 1){
            moveSquares[numMoveSquares] = i;
            numMoveSquares++;
        }
    }

    int numPatterns = 1 << numMoveSquares;
    unsigned long long* blockerBitboards = (unsigned long long*)calloc(numPatterns, sizeof(unsigned long long));

    for(int i = 0; i < numPatterns; i++){
        for(int j = 0; j < numMoveSquares; j++){
            unsigned long long bit = (i >> j) & 1ULL;
            blockerBitboards[i] |= bit << moveSquares[j];
        }
    }

    return blockerBitboards;
}