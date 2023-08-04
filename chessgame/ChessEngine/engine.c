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

bool is_using_transposition = true;
int pv_length[64];
Move pv_table[64][64];
int ply;
int num_positions = 0;
int num_no_hash_entry = 0;


int static_eval(){
    return mat_eval + pos_eval;
}

int search_moves(int depth, int start_depth){
    if(depth == 0){
        return static_eval();
    }
    Move* moves = (Move*)malloc(80 * sizeof(Move));
    int numElems = 0;

    update_possible_moves(moves, &numElems);
    Move move;

    if(numElems == 0){
        if(white_check || black_check){
            return INT_MAX;
        }
        return 0;
    }
    int bestEvaluation = INT_MAX;

    for(int i = 0; i < numElems; i++){
        move = moves[i];
        apply_move(move.start, move.end, move.move_id);
        int evaluation = -search_moves(depth - 1, depth);
        if(evaluation < bestEvaluation){
            bestEvaluation = evaluation;
            if(depth == start_depth){

                engine_move = move;
            }
        }
        undo_move();
        decr_num_moves();
        flip_turns();
    }
    return bestEvaluation;
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
    /*printf("\n\nSorted\n\n");
    for(int i = 0; i < size; i++){
        print_move(ordered[i]);
        printf(" %d\n", move_val[i]);
    }*/
}

void game_order_moves(){
    order_moves(game_possible_moves, num_game_moves, white_turn);
}


// this is what does the pruning
int search_moves_pruning(int depth, int start_depth, int alpha, int beta, bool player, struct Move* line, struct Move* best_line){
    if(depth == 0 && !white_check && !black_check){
        return static_eval();
    }
    struct Move* moves = (struct Move*)malloc(80 * sizeof(struct Move));
    int numElems = 0;

    update_possible_moves(moves, &numElems);
    order_moves(moves, numElems, player);
    struct Move move;

    if(numElems == 0){
        free(moves);
        if(white_check){
            return INT_MIN + (start_depth - depth);
        }
        else if(black_check){
            return INT_MAX - (start_depth - depth);
        }
        return 0;
    }
    if(depth == 0){
        free(moves);
        return static_eval();
    }
    // white making a move
    if (player){
        int maxEval = INT_MIN;
        for(int i = 0; i < numElems; i++){
            move = moves[i];
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
            alpha = max(alpha, evaluation);
            if(depth <= 1){
                best_alpha = max(best_alpha, alpha);
            }
            if (beta <= alpha){
                break;
            }
        }
        free(moves);
        return maxEval;
    }

    else{
        int minEval = INT_MAX;
        for(int i = 0; i < numElems; i++){
            move = moves[i];
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
            beta = min(beta, evaluation);
            if(depth <= 1){
                best_beta = min(best_beta, beta);
            }
            if (beta <= alpha){
                break;
            }
        }
        free(moves);
        return minEval;
    }
}

int search_moves_transposition(int depth, int start_depth, int alpha, int beta, bool player, Move* line, Move* best_line){
    pv_length[ply] = ply;
    int hash_flag = ALPHA_FLAG;

    int val = ReadHash(depth, alpha, beta);
    if(val != NO_HASH_ENTRY){
        num_no_hash_entry++;
        print_table_entry();
        //printf("\n");
        return val;
    }

    //int val = 0;
    if(depth == 0){
        num_positions++;
        val = static_eval();
        if(!player){
            val = -val;
        }
        WriteHash(depth, val, EXACT_FLAG);
        return val;
    }
    Move* moves = (Move*)malloc(80 * sizeof(Move));
    int numElems = 0;

    update_possible_moves(moves, &numElems);
    order_moves(moves, numElems, player);
    Move move;

    for(int i = 0; i < numElems; i++){
        move = moves[i];
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
            return beta;
        }
        if(val > alpha){
            pv_table[ply][ply] = move;
            for (int j = ply + 1; j < pv_length[ply + 1]; j++){
                pv_table[ply][j] = pv_table[ply + 1][j]; 
            }
            pv_length[ply] = pv_length[ply + 1];
            hash_flag = EXACT_FLAG;
            alpha = val;
        }
    }
    WriteHash(depth, alpha, hash_flag);
    return alpha;
}

// copy of search moves pruning
int search_moves_with_hint(int depth, int start_depth, int alpha, int beta, bool player, Move* line, Move* best_line, int* hint_line,int hint_depth, bool* applying_hint){
    if(depth == 0 && !white_check && !black_check){
        return static_eval();
    }

    Move* moves = (Move*)malloc(80 * sizeof(Move));
    int numElems = 0;

    update_possible_moves(moves, &numElems);
    Move move;

    if(numElems == 0){
        free(moves);
        if(white_check){
            return INT_MIN + start_depth - depth;
        }
        else if(black_check){
            return INT_MAX - start_depth + depth;
        }
        return 0;
    }
    if(depth == 0){
        free(moves);
        return static_eval();
    }
    // white making a move
    int hint_location = -1;
    if (*applying_hint){
        int current_hint = depth - (start_depth - hint_depth);
        printf("%d = %d - (%d - %d)\n", current_hint, depth, start_depth, hint_depth);
        hint_location = hint_line[current_hint];
        if (current_hint == 1){
            *applying_hint = false;
        }
    }
    if (player){
        int maxEval = INT_MIN;

        for(int i = 0; i < numElems; i++){
            if (i == 0 && hint_location >= 0){
                move = moves[hint_location];
            }
            // swap the location later in the list
            else if(hint_location == i){
                move = moves[0];
            }
            else{
                move = moves[i];
            }
            apply_move(move.start, move.end, move.move_id);
            line[depth] = move;
            int evaluation = search_moves_with_hint(depth - 1, start_depth, alpha, beta, false, line, best_line, hint_line, hint_depth, applying_hint);
            undo_move();
            decr_num_moves();
            flip_turns();
            if(evaluation > maxEval){
                maxEval = evaluation;
                best_line[depth] = move;
            }
            alpha = max(alpha, evaluation);
            if(depth <= 1){
                best_alpha = max(best_alpha, alpha);
            }
            if (beta <= alpha){
                break;
            }
        }
        free(moves);
        return maxEval;
    }

    else{
        int minEval = INT_MAX;
        for(int i = 0; i < numElems; i++){
            if (i == 0 && hint_location >= 0){
                move = moves[hint_location];
            }
            // swap the location later in the list
            else if(hint_location == i){
                move = moves[0];
            }
            else{
                move = moves[i];
            }
            apply_move(move.start, move.end, move.move_id);
            line[depth] = move;
            int evaluation = search_moves_with_hint(depth - 1, start_depth, alpha, beta, true, line, best_line, hint_line, hint_depth, applying_hint);
            undo_move();
            decr_num_moves();
            flip_turns();
            if(evaluation < minEval){
                minEval = evaluation;
                best_line[depth] = move;
            }
            beta = min(beta, evaluation);
            if(depth <= 1){
                best_beta = min(best_beta, beta);
            }
            if (beta <= alpha){
                break;
            }
        }
        free(moves);
        return minEval;
    }
}

// this is the test for the depth 4
int test_depth_pruning(int depth , int start_depth, int alpha, int beta, bool player, Move* line, int* best_line, Move* best_line_actual_moves){
    if(depth == 0 && !white_check && !black_check){
        return static_eval();
    }
    Move* moves = (Move*)malloc(80 * sizeof(Move));
    int numElems = 0;

    update_possible_moves(moves, &numElems);
    Move move;

    if(numElems == 0){
        free(moves);
        if(white_check){
            return INT_MIN + start_depth - depth;
        }
        else if(black_check){
            return INT_MAX - start_depth + depth;
        }
        return 0;
    }
    if(depth == 0){
        free(moves);
        return static_eval();
    }
    // white making a move
    if (player){
        int maxEval = INT_MIN;
        for(int i = 0; i < numElems; i++){
            move = moves[i];
            apply_move(move.start, move.end, move.move_id);
            line[depth] = move;
            int evaluation = test_depth_pruning(depth - 1, start_depth, alpha, beta, false, line, best_line, best_line_actual_moves);
            undo_move();
            decr_num_moves();
            flip_turns();
            if(evaluation > maxEval){
                maxEval = evaluation;
                best_line[depth] = i;
                best_line_actual_moves[depth] = move;
            }
            alpha = max(alpha, evaluation);
            if (beta <= alpha){
                break;
            }
        }
        free(moves);
        return maxEval;
    }

    else{
        int minEval = INT_MAX;
        for(int i = 0; i < numElems; i++){
            move = moves[i];
            apply_move(move.start, move.end, move.move_id);
            line[depth] = move;
            int evaluation = test_depth_pruning(depth - 1, start_depth, alpha, beta, true, line, best_line, best_line_actual_moves);
            undo_move();
            decr_num_moves();
            flip_turns();
            if(evaluation < minEval){
                minEval = evaluation;
                best_line[depth] = i;
                best_line_actual_moves[depth] = move;
            }
            beta = min(beta, evaluation);
            if (beta <= alpha){
                break;
            }
        }
        free(moves);
        return minEval;
    }
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
        printf("pv_talbe\n");
        for (int i = 0; i <= 6; i++){
            for (int j = 0; j <= 6; j++){
                print_move(pv_table[i][j]);
            }
            printf("\n");
        }
        engine_move = pv_table[0][0];
        engine_move.eval = eval;
        printf("num_positions %d\n", num_positions);
        printf("num_no_hash_entry %d\n", num_no_hash_entry);
        return engine_move;
    }
    

    int eval = search_moves_pruning(depth, depth, INT_MIN, INT_MAX, false, line, best_line);
    engine_move = best_line[depth];
    engine_move.eval = eval;
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

Move calc_eng_move_with_test(int test_depth, int total_depth){
    Move nm;
    nm.capture = -1;
    nm.end = -1;
    nm.eval = -1;
    nm.move_id = -1;
    nm.piece_id = -1;
    nm.start = -1;

    best_alpha = INT_MIN;
    best_beta = INT_MAX;

    // initializing to a empty list
    Move* line = (Move*)malloc((total_depth + 1) * sizeof(Move));
    for(int i = 0; i <= total_depth; i++){
        line[i] = nm;
    }

    // initializing to a empty list
    Move* best_line = (Move*)malloc((total_depth + 1) * sizeof(Move));
    for(int i = 0; i <= total_depth; i++){
        best_line[i] = nm;
    }

    // initializing to a empty list
    int* best_test_line = (int*)malloc((test_depth + 1) * sizeof(int));
    for(int i = 0; i <= test_depth; i++){
        best_test_line[i] = -1;
    }

    // initializing to a empty list
    Move* best_test_line_actual = (Move*)malloc((test_depth + 1) * sizeof(Move));
    for(int i = 0; i <= test_depth; i++){
        best_test_line_actual[i] = nm;
    }

    // initializing to a empty list
    Move* best_final_line = (Move*)malloc((total_depth + 1) * sizeof(Move));
    for(int i = 0; i <= total_depth; i++){
        best_final_line[i] = nm;
    }

    // gets best line at test depth and assigns it to best_test_line
    test_depth_pruning(test_depth, test_depth, INT_MIN, INT_MAX, false, line, best_test_line, best_test_line_actual);

    for(int i = 0; i <= test_depth; i++){
        printf("%d: %d\n", i, best_test_line[i]);
    }

    bool applying_hint = true;

    int eval = search_moves_with_hint(total_depth, total_depth, INT_MIN, INT_MAX, false, line, best_final_line, best_test_line, test_depth, &applying_hint);

    // find the best line and play the first move
    engine_move = best_final_line[total_depth];
    engine_move.eval = eval;
    return engine_move;
}

int max(int a, int b){
    if (a>b){
        return a;
    }
    else{
        return b;
    }
}

int min(int a, int b){
    if (a<b){
        return a;
    }
    else{
        return b;
    }
}
