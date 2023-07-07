#include "testing.h"
#include "piece.h"
#include "values.h"
#include "board.h"
#include <stdlib.h>
#include <string.h>

struct Move;

void print_legal_moves(struct Move* moves, int *numElems){
    for(int i = 0; i < (*numElems); i++){
        struct Move move = moves[i];
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

void print_move(struct Move move){
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

    struct Move* moves = (struct Move*)malloc(80 * sizeof(struct Move));
    int numElems = 0;

    update_possible_moves(moves, &numElems);

    unsigned long long num_positions = 0ULL;
    struct Move move;

    for(int i = 0; i < numElems; i++){
        move = moves[i];
        apply_move(move.start, move.end, move.move_id);
        num_positions += perft_test(depth - 1);
        undo_move();
        decr_num_moves();
        flip_turns();
        update_piece_masks();
    }

    free(moves);

    return num_positions;
}

unsigned long long detailed_perft(int depth){
    struct Move* moves = (struct Move*)malloc(80 * sizeof(struct Move));
    int numElems = 0;

    update_possible_moves(moves, &numElems);

    unsigned long long num_positions = 0ULL;
    struct Move move;
    int n;
    int s;
    int e;
    int m;
    char file;
    int rank;

    for(int i = 0; i < numElems; i++){

        move = moves[i];
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

    free(moves);

    return num_positions;
}

void run_game(){
    char str[] = "";
    int x;
    int z;
    char w;
    char y;
    int start;
    int end;
    int promo;
    bool undo_successful;


    //update_possible_moves(moves, &numElems);
    update_game_possible_moves();
    draw_board();


    while(true){
        if(white_turn){
            printf("white's turn\n");
        }
        else{
            printf("black's turn\n");
        }
        print_legal_moves(game_possible_moves, &num_game_moves);

        //get input from user in notation (e.g. b1c3)
        printf("Make a move: ");
        gets(str);
        //if empty input, set start to -1 to indicate undo move
        if(strlen(str) == 0){
            start = -1;
            end = -1;
        }
        //otherwise, set start and end square (0 to 63) to files and ranks
        else{
            w = str[0];
            x = (int)str[1] - 48;
            y = str[2];
            z = (int)str[3] - 48;
            //printf("%c %d %c %d\n", w, x, y, z);
            printf("\n");
            start = notation_to_number(w, x);
            end = notation_to_number(y, z);
        }
        //promotion is unsupported at the moment
        promo = 0;

        //if start and end squares within bounds
        if(0 <= start && start < 64 && 0 <= end && end < 64){
            //if the move is in the list of legal moves
            if(is_legal_move(start, end, promo, game_possible_moves, num_game_moves)){
                //make the move on the board
                apply_move(start, end, 0);
                printf("%d\n" ,mat_eval);
                update_possible_moves(game_possible_moves, &num_game_moves);
            }
            //otherwise the move is illegal
            else{
                printf("Illegal\n\n");
            }
        }
        //if we are undoing a move
        else{
            undo_successful = try_undo_move();
            //if there is a move to undo
            if(undo_successful){
                printf("Undo\n\n");
                update_possible_moves(game_possible_moves, &num_game_moves);
            }
            //there are no moves to undo
            else{
                printf("Nothing to undo\n\n");
            }
        }
        draw_board();
    }
}

void print_line(struct Move* line, size_t n){
    for(int i = 1; i <= n; i++){
        print_move(line[i]);
        printf(" ");
    }
    printf("\n");
}