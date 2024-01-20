#include "testing.h"
#include "piece.h"
#include "values.h"
#include "board.h"
#include "bitwise.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "transposition.h"

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
    //the number of squares that a bloker could take up
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
    free(moveSquares);
    return blockerBitboards;
}

unsigned long long get_rook_masks(int square){
    unsigned long long rook_file = file[8 - get_file(square)];
    unsigned long long rook_rank = rank[get_rank(square) + 1];
    rook_file = rook_file & ~(rank[1] | rank[8]);
    rook_rank = rook_rank & ~(file[1] | file[8]);
    unsigned long long result = rook_file ^ rook_rank;
    result &= ~(1ULL << square);
    return result;
}

void write_rook_moves_lookup_to_file(unsigned long long* magic, int* shift){
    FILE *file = fopen("magic_rook_nums.txt", "w");

    // Check if the file was opened successfully
    if (file == NULL) {
        printf("Error opening the file.\n");
        return;
    }

    for(int i = 0; i <= 63; i++){//64; i++){

        unsigned long long movement_mask = get_rook_masks(i);
        unsigned long long* blockers = get_blockers_rook_single_square(movement_mask);
        int index = 0;
        //fprintf(file, "%d,", 1 << 14);
        //print_bitboard(movement_mask);
        int blocker_max = 10;
        if(get_file(i) == 0 || get_file(i) == 7){
            blocker_max += 1;
        }
        if(get_rank(i) == 0 || get_rank(i) == 7){
            blocker_max += 1;
        }

        for(int j = 0; j < 1 << blocker_max; j++){
            unsigned long long rook_legal_moves = rook_moves_single_square(i, blockers[j]);
            index = get_index_from_magic(blockers[j], magic[i], shift[i]);
            fprintf(file, "%d %llu %d %d %llu\n",i, magic[i], shift[i], index, rook_legal_moves);
        }
        //this is for get_blockers_rook_single_square
        free(blockers);
    }
    printf("Done\n");

    fclose(file);
}

int get_index_from_magic(unsigned long long blocker, unsigned long long magic_number, int shift){
    return (blocker * magic_number) >> shift;
}

bool is_valid_rook_magic_number(int square, unsigned long long magic_number, int shift){
    unsigned long long T_shift = (1 << (64-shift));
    bool *indices_seen = (bool *)calloc(T_shift, sizeof(bool));
    unsigned long long movement_mask = get_rook_masks(square);
    unsigned long long* blockers = get_blockers_rook_single_square(movement_mask);
    int blocker_max = 10;
    if(get_file(square) == 0 || get_file(square) == 7){
        blocker_max += 1;
    }
    if(get_rank(square) == 0 || get_rank(square) == 7){
        blocker_max += 1;
    }
    for(int j = 0; j < 1 << blocker_max; j++){
        int index = get_index_from_magic(blockers[j], magic_number, shift);
        if(indices_seen[index]){
            return false;
        }
        indices_seen[index] = true;
        //unsigned long long rook_legal_moves = rook_moves_single_square(square, blockers[j]);
    }
    free(blockers);
    return true;
}

unsigned long long find_single_rook_magic_number(int square, int shift, int num_iterations){
    //printf("Magic index: %d\n", get_index_from_magic(4503599728033792ULL, 10414575345181196316ULL, 54));
    unsigned long long r_num;
    for(int i = 0; i < num_iterations; i++){
        r_num = get_random_U64_number();
        if (is_valid_rook_magic_number(square, r_num, shift)){
            return r_num;
        }
    }
    return 0ULL;
}

void generate_rook_magic_numbers(int min_shift, int num_iterations, unsigned long long* result_magic, int* result_shift, int amount_run){
    for(int i = 0; i < 64; i++){
        result_shift[i] = min_shift;
    }
    int shift = min_shift;
    for(int j = 0;j < amount_run; j++){
        for(int i = 0; i < 64; i++){
            shift = result_shift[i] + 1;
            unsigned long long magic = find_single_rook_magic_number(i, shift, num_iterations);
            if(magic != 0ULL){
                result_magic[i] = magic;
                result_shift[i] = shift;
            }
        }
    }
}