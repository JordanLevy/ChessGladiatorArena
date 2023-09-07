#include "engine.h"
#include "values.h"
#include "piece.h"
#include "board.h"
#include "testing.h"
#include "transposition.h"
#include <stdbool.h>
#include <stdlib.h>

#define START_ALPHA -1000001
#define START_BETA 1000000
#define MATE_SCORE 1000000

bool is_using_transposition = false;
int pv_length[64];
Move pv_table[64][64];
int ply;
int num_positions = 0;
int num_with_hash_entry = 0;


int static_eval(){
    return mat_eval + pos_eval;
}

// this is to do stuf like chking i fyou are capteringa a good pece
int calc_static_move_eval(Move move, bool is_white_turn){
    int value = 0;
    unsigned char moved_piece = get_type(move.piece_id);
    unsigned char capture = get_type(move.capture);
    if (move.capture > 0){
        //this will give you the value of what is being captered
        value += CAPTURE_PIECE_VALUE_MULTIPLIER * abs(values[capture]) - abs(values[moved_piece]);
    }
    int a = square_incentive[moved_piece][move.end] - square_incentive[moved_piece][move.start];
    if(is_white_turn){
        value += a;
    }
    else{
        value -= a;
    }
    return value;
}

// swaps the vals at the indexis
void swap(int* scores, Move* legal_moves, int val1, int val2){
    int temp_int = scores[val1];
    scores[val1] = scores[val2];
    scores[val2] = temp_int;

    Move temp_move = legal_moves[val1];
    legal_moves[val1] = legal_moves[val2];
    legal_moves[val2] = temp_move;
}

int median_of_three(int* scores, Move* legal_moves, int low, int hi){
    int mid = (low + hi) / 2;
    if(scores[mid] < scores[low]){
        swap(scores, legal_moves, low, mid);
    }
    if(scores[hi] < scores[low]){
        swap(scores, legal_moves, low, hi);
    }
    if(scores[hi] < scores[mid]){
        swap(scores, legal_moves, mid, hi);
    }
    return mid;
}

int partition(int* scores, Move* legal_moves, int low, int hi){
    int pivotIndex = median_of_three(scores, legal_moves, low, hi);
    int pivot = -scores[pivotIndex];
    swap(scores, legal_moves, low, pivotIndex);
    int i = low;
    int j = hi + 1;
    while(true){
        while(-scores[++i] < pivot){
            if(i == hi){
                break;
            }
        }
        while(pivot < -scores[--j]){
            if(j == low){
                break;
            }
        }
        if(i >= j){
            break;
        }
        swap(scores, legal_moves, i, j);
    }
    swap(scores, legal_moves, low, j);
    return j;
}

void quick_sort(int* scores, Move* legal_moves, int low, int hi){
    if (hi <= low){
        return;
    }
    int j = partition(scores, legal_moves, low, hi);
    quick_sort(scores, legal_moves, low, j - 1);
    quick_sort(scores, legal_moves, j + 1, hi);
}

// this function should order the moves that we search
// best moves at the start of the list
void order_moves(Move* ordered, int size, bool is_white_turn){
    int* move_val = (int*)malloc(size * sizeof(int));
    for(int i = 0; i < size; i++){
        move_val[i] = calc_static_move_eval(ordered[i], is_white_turn);
        //print_move(ordered[i]);
        //printf(" %d\n", move_val[i]);
    }
    quick_sort(move_val, ordered, 0, size - 1);
    free(move_val);
    /*printf("\n\nSorted\n\n");
    for(int i = 0; i < size; i++){
        print_move(ordered[i]);
        printf(" %d\n", move_val[i]);
    }*/
}

// this is what does the pruning
int search_moves_pruning(int depth, int start_depth, int alpha, int beta, bool player, struct Move* line, struct Move* best_line){
    if(depth == 0 && !white_check && !black_check){
        return static_eval();
    }

    //makes the move_list
    MoveList* move_lists = (MoveList*)malloc(1 * sizeof(MoveList));
    move_lists[ALL].size = 0; 
    move_lists[ALL].moves = (Move*)malloc(80 * sizeof(Move));

    update_possible_moves(move_lists);
    order_moves(move_lists[ALL].moves, move_lists[ALL].size, player);
    Move move;

    if(move_lists[ALL].size == 0){

        free(move_lists[ALL].moves);
        free(move_lists);

        if(white_check){
            return INT_MIN + (start_depth - depth);
        }
        else if(black_check){
            return INT_MAX - (start_depth - depth);
        }
        return 0;
    }
    if(depth == 0){
        free(move_lists[ALL].moves);
        free(move_lists);

        return static_eval();
    }
    // white making a move
    if (player){
        int maxEval = INT_MIN;
        for(int i = 0; i < move_lists[ALL].size; i++){
            move = move_lists[ALL].moves[i];
            apply_move(move.start, move.end, move.move_id);
            line[depth] = move;
            int evaluation = search_moves_pruning(depth - 1, start_depth, alpha, beta, false, line, best_line);
            undo_move();
            decr_num_moves();
            flip_turns();
            if(evaluation > maxEval){
                maxEval = evaluation;
                best_line[depth] = move;
            }
            alpha = max_val(alpha, evaluation);
            if(depth <= 1){
                best_alpha = max_val(best_alpha, alpha);
            }
            if (beta <= alpha){
                break;
            }
        }
        free(move_lists[ALL].moves);
        free(move_lists);
        return maxEval;
    }

    else{
        int minEval = INT_MAX;
        for(int i = 0; i < move_lists[ALL].size; i++){
            move = move_lists[ALL].moves[i];
            apply_move(move.start, move.end, move.move_id);
            line[depth] = move;
            int evaluation = search_moves_pruning(depth - 1, start_depth, alpha, beta, true, line, best_line);
            undo_move();
            decr_num_moves();
            flip_turns();
            if(evaluation < minEval){
                minEval = evaluation;
                best_line[depth] = move;
            }
            beta = min_val(beta, evaluation);
            if(depth <= 1){
                best_beta = min_val(best_beta, beta);
            }
            if (beta <= alpha){
                break;
            }
        }
        free(move_lists[ALL].moves);
        free(move_lists);
        return minEval;
    }
}

int search_moves_transposition(int depth, int start_depth, int alpha, int beta, bool player, Move* line, Move* best_line){
    pv_length[ply] = ply;
    int hash_flag = ALPHA_FLAG;

    int val = ReadHash(depth, alpha, beta);
    if(val != NO_HASH_ENTRY){
        num_with_hash_entry++;
        return val;
    }

    //int val = 0;
    if(depth == 0){
        return search_moves_captures(alpha, beta, player);
    }
    MoveList* move_lists = (MoveList*)malloc(1 * sizeof(MoveList));
    move_lists[ALL].size = 0; 
    move_lists[ALL].moves = (Move*)malloc(80 * sizeof(Move));

    update_possible_moves(move_lists);
    order_moves(move_lists[ALL].moves, move_lists[ALL].size, player);
    Move move;

    for(int i = 0; i < move_lists[ALL].size; i++){
        move = move_lists[ALL].moves[i];
        apply_move(move.start, move.end, move.move_id);
        line[depth] = move;
        ply++;
        val = -search_moves_transposition(depth - 1, start_depth, -beta, -alpha, !player, line, best_line);
        ply--;
        undo_move();
        decr_num_moves();
        flip_turns();
        if(val >= beta){
            WriteHash(depth, beta, BETA_FLAG);
            free(move_lists[ALL].moves);
            free(move_lists);
            return beta;
        }
        if(val > alpha){
            move.eval = val;
            pv_table[ply][ply] = move;
            for (int j = ply + 1; j < pv_length[ply + 1]; j++){
                pv_table[ply][j] = pv_table[ply + 1][j]; 
            }
            pv_length[ply] = pv_length[ply + 1];
            hash_flag = EXACT_FLAG;
            alpha = val;
        }
    }
    if(move_lists[ALL].size == 0){
        free(move_lists[ALL].moves);
        free(move_lists);
        if(white_in_check || black_in_check){
            return -MATE_SCORE + ply;
        }
        return 0;
    }
    WriteHash(depth, alpha, hash_flag);
        free(move_lists[ALL].moves);
        free(move_lists);
    return alpha;
}

int search_moves_captures(int alpha, int beta, bool player){

    num_positions++;
    int val = static_eval();
    if(!player){
        val = -val;
    }
    if(val >= beta){
        return beta;
    }
    if(val > alpha){
        alpha = val;
    }
    MoveList* move_lists = (MoveList*)malloc(1 * sizeof(MoveList));
    move_lists[ALL].size = 0; 
    move_lists[ALL].moves = (Move*)malloc(80 * sizeof(Move));

    update_possible_moves(move_lists);

    Move move;

    for(int i = 0; i < move_lists[ALL].size; i++){
        move = move_lists[ALL].moves[i];
        if(move.capture == EMPTY_SQUARE){
            continue;
        }
        apply_move(move.start, move.end, move.move_id);
        ply++;
        val = -search_moves_captures(-beta, -alpha, !player);
        ply--;
        undo_move();
        decr_num_moves();
        flip_turns();
        if(val >= beta){
            free(move_lists[ALL].moves);
            free(move_lists);
            return beta;
        }
        if(val > alpha){
            move.eval = val;
            alpha = val;
        }
    }
    if(move_lists[ALL].size == 0){
        free(move_lists[ALL].moves);
        free(move_lists);
        if(white_in_check || black_in_check){
            return -MATE_SCORE + ply;
        }
        return 0;
    }
        free(move_lists[ALL].moves);
        free(move_lists);
    return alpha;
}

Move calc_eng_move(int depth){
    Move nm;
    nm.capture = -1;
    nm.end = -1;
    nm.eval = -1;
    nm.move_id = -1;
    nm.piece_id = -1;
    nm.start = -1;

    Move* line = (Move*)malloc((depth + 1) * sizeof(Move));
    for(int i = 0; i <= depth; i++){
        line[i] = nm;
    }

    Move* best_line = (Move*)malloc((depth + 1) * sizeof(Move));
    for(int i = 1; i <= depth; i++){
        best_line[i] = nm;
    }

    if(is_using_transposition){
        int eval = search_moves_transposition(depth, depth, START_ALPHA, START_BETA, false, line, best_line);
        engine_move = pv_table[0][0];
        engine_move.eval = eval;
        printf("pv_table\n");
        for (int i = 0; i < pv_length[0]; i++){
            print_move(pv_table[0][i]);
            printf(" %d\n", pv_table[0][i].eval);
        }
        printf("\n");
        printf("num_positions %d\n", num_positions);
        printf("num_with_hash_entry %d\n", num_with_hash_entry);
        
        free(line);
        free(best_line);
        return engine_move;
    }
    

    int eval = search_moves_pruning(depth, depth, INT_MIN, INT_MAX, false, line, best_line);
    engine_move = best_line[depth];
    engine_move.eval = eval;
    free(line);
    free(best_line);
    return engine_move;
}

bool move_equal(Move a, Move b){
    if(a.move_id != b.move_id) return false;
    if(a.capture != b.capture) return false;
    if(a.start != b.start) return false;
    if(a.end != b.end) return false;
    if(a.piece_id != b.piece_id) return false;
    return true;
}

int max_val(int a, int b){
    if (a>b){
        return a;
    }
    else{
        return b;
    }
}

int min_val(int a, int b){
    if (a<b){
        return a;
    }
    else{
        return b;
    }
}
