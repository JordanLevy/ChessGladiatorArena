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


//#############################################################################################3




















void update_unsafe(){
    unsafe_white = unsafe_for_white();
    white_check = white_in_check();
    unsafe_black = unsafe_for_black();
    black_check = black_in_check();
}

void update_piece_masks(){
    white_pieces = bitboards[wP] | bitboards[wN] | bitboards[wB] | bitboards[wR] | bitboards[wQ];
    black_pieces = bitboards[bP] | bitboards[bN] | bitboards[bB] | bitboards[bR] | bitboards[bQ];
    not_white_pieces = ~(white_pieces | bitboards[wK] | bitboards[bK]);
    not_black_pieces = ~(black_pieces | bitboards[wK] | bitboards[bK]);
    empty = ~(white_pieces | black_pieces | bitboards[wK] | bitboards[bK]);
    occupied = ~empty;
}

void possible_moves_white(struct Move* moves, int *numElems){
    update_piece_masks();
    update_unsafe();
    (*numElems) = 0;
    possible_wP(bitboards[wP], moves, numElems);
    possible_N(bitboards[wN], not_white_pieces, WHITE, moves, numElems);
    possible_B(bitboards[wB], not_white_pieces, WHITE, moves, numElems);
    possible_R(bitboards[wR], not_white_pieces, WHITE, moves, numElems);
    possible_Q(bitboards[wQ], not_white_pieces, WHITE, moves, numElems);
    possible_K(bitboards[wK], not_white_pieces, WHITE, moves, numElems);
}

void possible_moves_black(struct Move* moves, int *numElems){
    update_piece_masks();
    update_unsafe();
    (*numElems) = 0;
    possible_bP(bitboards[bP], moves, numElems);
    possible_N(bitboards[bN], not_black_pieces, BLACK, moves, numElems);
    possible_B(bitboards[bB], not_black_pieces, BLACK, moves, numElems);
    possible_R(bitboards[bR], not_black_pieces, BLACK, moves, numElems);
    possible_Q(bitboards[bQ], not_black_pieces, BLACK, moves, numElems);
    possible_K(bitboards[bK], not_black_pieces, BLACK, moves, numElems);
}

void update_possible_moves(struct Move* moves, int *numElems){
    if(white_turn){
        possible_moves_white(moves, numElems);
    }
    else{
        possible_moves_black(moves, numElems);
    }
}

void update_game_possible_moves(){
    update_possible_moves(game_possible_moves, &num_game_moves);
}

bool white_in_checkmate(int numElems){
    // can't be in checkmate if it's not your turn
    if(!white_turn){
        return false;
    }
    // can't be in checkmate if you're not in check
    if(!white_check){
        return false;
    }
    // can't be in checkmate if you have legal moves
    if(numElems > 0){
        return false;
    }
    return true;
}

bool black_in_checkmate(int numElems){
    if(white_turn){
        return false;
    }
    if(!black_check){
        return false;
    }
    if(numElems > 0){
        return false;
    }
    return true;
}

void init_board(char* fen, size_t len){
    init_fen(fen, len);
    init_masks();
}

bool is_legal_move(int start, int end, int promo, struct Move* moves, size_t n){
    struct Move move;
    for(int i = 0; i < n; i++){
        move = moves[i];
        if(move.start == start && move.end == end && move.move_id == promo){
            return true;
        }
    }
    return false;
}

void incr_num_moves(){
    num_moves++;
}

void decr_num_moves(){
    num_moves--;
}

void flip_turns(){
    white_turn = !white_turn;
}

char piece_letter(int piece_id, bool caps){
    char letters[] = "_PNBRQK__pnbrqk";
    unsigned char type = get_type(piece_id);
    if(caps && (type > 8)){
        type -= 8;
    }
    return letters[type];
}

char file_letter(int n){
    char letter[] = "abcdefgh";
    return letter[n];
}

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

// if they moved one of the castling rooks, increment the number of moves it has made
// given the id of the piece that was moved, and whose turn it was
void apply_rook_move(unsigned char id){
    if(white_turn){
        if(id == kingside_wR){
            kingside_wR_num_moves++;
        }
        else if(id == queenside_wR){
            queenside_wR_num_moves++;
        }
    }
    else{
        if(id == kingside_bR){
            kingside_bR_num_moves++;
        }
        else if(id == queenside_bR){
            queenside_bR_num_moves++;
        }
    }
}

bool apply_castling(unsigned char id, int start, int end){
    unsigned char type = get_type(id);
    bool is_castling = false;
    if(type == wK){
        //white kingside castling
        if(end - start == -2){
            move_piece(kingside_wR, 0, 2);
            piece_location[kingside_wR] = 2;
            is_castling = true;
        }
        //white queenside castling
        if(end - start == 2){
            move_piece(queenside_wR, 7, 4);
            piece_location[queenside_wR] = 4;
            is_castling = true;
        }
        wK_num_moves++;
    }
    else if(type == bK){
        //black kingside castling
        if(end - start == -2){
            move_piece(kingside_bR, 56, 58);
            piece_location[kingside_bR] = 58;
            is_castling = true;
        }
        //black queenside castling
        if(end - start == 2){
            move_piece(queenside_bR, 63, 60);
            piece_location[queenside_bR] = 60;
            is_castling = true;
        }
        bK_num_moves++;
    }
    return is_castling;
}

bool apply_move(int start, int end, int move_id){
    unsigned char moved_piece = get_piece(start);
    unsigned char type = get_type(moved_piece);
    // not their turn to make a move
    if(white_turn != is_white_piece(moved_piece)){
        printf("\n\nNot your turn\nwhite_turn:%d\nstart:%d\nend:%d\nmove_id:%d\n\n", white_turn, start, end, move_id);
        draw_board();
        update_game_possible_moves();
        print_legal_moves(game_possible_moves, &num_game_moves);
        return false;
    }
    unsigned char captured_piece = get_piece(end);
    int new_s = start;
    int new_e = end;
    int new_m = move_id;
    unsigned char new_c = captured_piece;
    if(captured_piece > 0){
        remove_piece(captured_piece, end);
    }
    move_piece(moved_piece, start, end);
    apply_rook_move(moved_piece);
    if(apply_castling(moved_piece, start, end)){
        new_m = CASTLING;
    }
    // this is for promotion
    if(1 <= move_id && move_id <= 15){
        remove_piece(moved_piece, end);
        add_piece(move_id << ROLE_BITS_OFFSET, end);
    }
    if((type == wP || type == bP) && (abs(end - start) == 16)){
        // double pawn push
        new_m = DOUBLE_PAWN_PUSH;
        // move_list.append((start, end, 13, 0))
    }
    // if the move was castling
    else if((type == wK || type == bK) && (abs(end - start) == 2)){
    }
    else{
        // previous move start, end, and move_id
        if(num_moves > 0){
            struct Move prev_move = move_list[num_moves - 1];
            int e = prev_move.end;
            int m = prev_move.move_id;
            unsigned char ep_pawn = get_piece(e);  // pawn that was captured en passant
            unsigned char ep_pawn_type = get_type(ep_pawn);
            // white capturing en passant
            if(m == DOUBLE_PAWN_PUSH && type == wP && ep_pawn_type == bP && end - e == 8){
                remove_piece(ep_pawn, e);
                new_m = EN_PASSANT_CAPTURE;
                new_c = ep_pawn;
            }
            // black capturing en passant
            else if(m == DOUBLE_PAWN_PUSH && type == bP && ep_pawn_type == wP && end - e == -8){
                remove_piece(ep_pawn, e);
                new_m = EN_PASSANT_CAPTURE;
                new_c = ep_pawn;
            }
        }
    }
    struct Move move;
    move.start = new_s;
    move.end = new_e;
    move.move_id = new_m;
    move.piece_id = moved_piece;
    move.capture = new_c;
    move_list[num_moves] = move;
    incr_num_moves();
    flip_turns();
    return true;
}

// if they moved one of the castling rooks, decrement the number of moves it has made
// given the id of the piece that was moved, and whose turn it was
void undo_rook_move(unsigned char id){
    if(id == kingside_wR){
        kingside_wR_num_moves--;
    }
    else if(id == queenside_wR){
        queenside_wR_num_moves--;
    }
    else if(id == kingside_bR){
        kingside_bR_num_moves--;
    }
    else if(id == queenside_bR){
        queenside_bR_num_moves--;
    }
}

/*
Undoes the rook move from castling
*/
void undo_castling(unsigned char id, int start, int end){
    unsigned char type = get_type(id);
    // white king
    if(type == wK){
        //kingside
        if(end - start == -2){
            move_piece(kingside_wR, piece_location[kingside_wR], 0);
        }
        //queenside
        if(end - start == 2){
            move_piece(queenside_wR, piece_location[queenside_wR], 7);
        }
    }
    // black king
    else if(type == bK){
        //kingside
        if(end - start == -2){
            move_piece(kingside_bR, piece_location[kingside_bR], 56);
        }
        //queenside
        if(end - start == 2){
            move_piece(queenside_bR, piece_location[queenside_bR], 63);
        }
    }
}

void undo_move(){
    // can't undo if nothing has been played
    if(num_moves == 0){
        return;
    }

    //previous move (the one we're undoing)
    struct Move move = move_list[num_moves - 1];
    int start = move.start;
    int end = move.end;
    int move_id = move.move_id;
    int capture = move.capture;
    // the piece that was moved
    unsigned char moved_piece = get_piece(end);
    unsigned char type = get_type(moved_piece);
    bool is_white = is_white_piece(moved_piece);
    move_piece(moved_piece, end, start);
    undo_rook_move(moved_piece);
    // last move was a capture
    if(capture > 0){
        // last move was en passant
        if(move_id == EN_PASSANT_CAPTURE){
            // en passant is the only case where the captured piece isn't on the end square
            if(is_white){
                revive_piece(capture, end - 8);
            }
            else{
                revive_piece(capture, end + 8);
            }
        }
        else{
            revive_piece(capture, end);
        }
    }
    if(move_id == 0){
    }
    // last move was pawn promotion
    else if(1 <= move_id && move_id <= 15){
        destroy_piece(moved_piece, start);
        unsigned char promoted_pawn = move.piece_id;
        revive_piece(promoted_pawn, start);
    }
    // last move was double pawn push
    else if(move_id == DOUBLE_PAWN_PUSH){
    }
    // last move was castling
    else if(move_id == CASTLING){
        undo_castling(moved_piece, start, end);
    }
    // if we're undoing a white king move
    if(type == wK){
        wK_num_moves -= 1;
    }
    // if we're undoing a black king move
    else if(type == bK){
        bK_num_moves -= 1;
    }
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

bool get_white_check(){
    return white_check;
}

bool get_black_check(){
    return black_check;
}

bool try_undo_move(){
    if(num_moves > 0){
        undo_move();
        decr_num_moves();
        flip_turns();
        return true;
    }
    return false;
}

bool is_game_legal_move(int start, int end, int promo){
    return is_legal_move(start, end, promo, game_possible_moves, num_game_moves);
}

unsigned char* get_board_state(){
    return board;
}

void init(char* fen, int len){
    game_possible_moves = (struct Move*)malloc(80 * sizeof(struct Move));
    num_game_moves = 0;
    init_board(fen, len);
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

int static_eval(){
    return mat_eval + pos_eval;
}

// recalculate the legal moves for a given piece
void update_piece_moves(int square){
    int piece_type = get_piece(square);
    if (piece_type == 4){
        return;
    }

}

int search_moves(int depth, int start_depth){
    if(depth == 0){
        return static_eval();
    }
    struct Move* moves = (struct Move*)malloc(80 * sizeof(struct Move));
    int numElems = 0;

    update_possible_moves(moves, &numElems);
    struct Move move;

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

void print_line(struct Move* line, size_t n){
    for(int i = 1; i <= n; i++){
        print_move(line[i]);
        printf(" ");
    }
    printf("\n");
}


// this is to do stuf like chking i fyou are capteringa a good pece
int calc_static_move_eval(struct Move move, bool is_white_turn){
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
void swap(int* scores, struct Move* legal_moves, int val1, int val2){
    int temp_int = scores[val1];
    scores[val1] = scores[val2];
    scores[val2] = temp_int;

    struct Move temp_move = legal_moves[val1];
    legal_moves[val1] = legal_moves[val2];
    legal_moves[val2] = temp_move;
}

int median_of_three(int* scores, struct Move* legal_moves, int low, int hi){
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

int partition(int* scores, struct Move* legal_moves, int low, int hi){
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

void quick_sort(int* scores, struct Move* legal_moves, int low, int hi){
    if (hi <= low){
        return;
    }
    int j = partition(scores, legal_moves, low, hi);
    quick_sort(scores, legal_moves, low, j - 1);
    quick_sort(scores, legal_moves, j + 1, hi);
}

// this function should order the moves that we search
// best moves at the start of the list
void order_moves(struct Move* ordered, int size, bool is_white_turn){
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
// copy of search moves pruning
int search_moves_with_hint(int depth, int start_depth, int alpha, int beta, bool player, struct Move* line, struct Move* best_line, int* hint_line,int hint_depth, bool* applying_hint){
    if(depth == 0 && !white_check && !black_check){
        return static_eval();
    }

    struct Move* moves = (struct Move*)malloc(80 * sizeof(struct Move));
    int numElems = 0;

    update_possible_moves(moves, &numElems);
    struct Move move;

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
int test_depth_pruning(int depth , int start_depth, int alpha, int beta, bool player, struct Move* line, int* best_line, struct Move* best_line_actual_moves){
    if(depth == 0 && !white_check && !black_check){
        return static_eval();
    }
    struct Move* moves = (struct Move*)malloc(80 * sizeof(struct Move));
    int numElems = 0;

    update_possible_moves(moves, &numElems);
    struct Move move;

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

struct Move calc_eng_move(int depth){
    struct Move nm;
    nm.capture = -1;
    nm.end = -1;
    nm.eval = -1;
    nm.move_id = -1;
    nm.piece_id = -1;
    nm.start = -1;

    struct Move* line = (struct Move*)malloc((depth + 1) * sizeof(struct Move));
    for(int i = 0; i <= depth; i++){
        line[i] = nm;
    }

    struct Move* best_line = (struct Move*)malloc((depth + 1) * sizeof(struct Move));
    for(int i = 1; i <= depth; i++){
        best_line[i] = nm;
    }

    int eval = search_moves_pruning(depth, depth, INT_MIN, INT_MAX, false, line, best_line);

    engine_move = best_line[depth];
    engine_move.eval = eval;
    return engine_move;
}

bool move_equal(struct Move a, struct Move b){
    if(a.move_id != b.move_id) return false;
    if(a.capture != b.capture) return false;
    if(a.start != b.start) return false;
    if(a.end != b.end) return false;
    if(a.piece_id != b.piece_id) return false;
    return true;
}

struct Move calc_eng_move_with_test(int test_depth, int total_depth){
    struct Move nm;
    nm.capture = -1;
    nm.end = -1;
    nm.eval = -1;
    nm.move_id = -1;
    nm.piece_id = -1;
    nm.start = -1;

    best_alpha = INT_MIN;
    best_beta = INT_MAX;

    // initializing to a empty list
    struct Move* line = (struct Move*)malloc((total_depth + 1) * sizeof(struct Move));
    for(int i = 0; i <= total_depth; i++){
        line[i] = nm;
    }

    // initializing to a empty list
    struct Move* best_line = (struct Move*)malloc((total_depth + 1) * sizeof(struct Move));
    for(int i = 0; i <= total_depth; i++){
        best_line[i] = nm;
    }

    // initializing to a empty list
    int* best_test_line = (int*)malloc((test_depth + 1) * sizeof(int));
    for(int i = 0; i <= test_depth; i++){
        best_test_line[i] = -1;
    }

    // initializing to a empty list
    struct Move* best_test_line_actual = (struct Move*)malloc((test_depth + 1) * sizeof(struct Move));
    for(int i = 0; i <= test_depth; i++){
        best_test_line_actual[i] = nm;
    }

    // initializing to a empty list
    struct Move* best_final_line = (struct Move*)malloc((total_depth + 1) * sizeof(struct Move));
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
int get_eng_move_start(){
    return engine_move.start;
}
int get_eng_move_end(){
    return engine_move.end;
}
int get_eng_move_eval(){
    return engine_move.eval;
}
int get_eng_move_id(){
    return engine_move.move_id;
}

int get_wK_num_moves(){
    return wK_num_moves;
}

int get_bK_num_moves(){
    return bK_num_moves;
}

unsigned char get_kingside_wR(){
    return kingside_wR;
}

unsigned char get_queenside_wR(){
    return queenside_wR;
}

unsigned char get_kingside_bR(){
    return kingside_bR;
}

unsigned char get_queenside_bR(){
    return queenside_bR;
}

int get_kingside_wR_num_moves(){
    return kingside_wR_num_moves;
}

int get_queenside_wR_num_moves(){
    return queenside_wR_num_moves;
}

int get_kingside_bR_num_moves(){
    return kingside_bR_num_moves;
}

int get_queenside_bR_num_moves(){
    return queenside_bR_num_moves;
}
struct Move* get_possible_moves(){
    return game_possible_moves;
}

int get_num_possible_moves(){
    return num_game_moves;
}

int get_mat_eval(){
    return mat_eval;
}

int get_pos_eval(){
    return pos_eval;
}

char* move_to_string(struct Move move){
    char* result = (char*)malloc(sizeof(char) * 5);
    int s = move.start;

    /*if(s == -1){
        return;
    }*/
    int e = move.end;
    int m = move.move_id;

    char promotion = '\0';

    char start_file = file_letter(7 - get_file(s));
    int start_rank = get_rank(s) + 1;

    char end_file = file_letter(7 - get_file(e));
    int end_rank = get_rank(e) + 1;

    if(m == wN || m == bN){
        promotion = 'n';
    }
    else if(m == wB || m == bB){
        promotion = 'b';
    }
    else if(m == wR || m == bR){
        promotion = 'r';
    }
    else if(m == wQ || m == bQ){
        promotion = 'q';
    }

    if(move.start == -1){
        return "(none)";
    }

    snprintf(result, 6, "%c%d%c%d%c", start_file, start_rank, end_file, end_rank, promotion);

    return result;
}

bool str_equals(const char* a, const char* b){
    return strcmp(a, b) == 0;
}

bool startswith(const char* str, const char* prefix) {
    size_t len_str = strlen(str);
    size_t len_prefix = strlen(prefix);
    if (len_prefix > len_str) {
        return 0;
    }
    return strncmp(str, prefix, len_prefix) == 0;
}

char* substring(char* str, int start, int end) {
    int i;
    int j = 0;
    char* sub = (char*)malloc(sizeof(char) * (end - start + 1));
    for (i = start; i <= end; i++) {
        sub[j] = str[i];
        j++;
    }
    sub[j] = '\0';
    return sub;
}

void inputUCI(){
    printf("id name Odin\n");
    printf("id author Ryan Johnson and Jordan Levy\n");
    printf("uciok\n");
}

void inputSetOption(){
    printf("setOption working\n");
}

void inputIsReady(){
    printf("readyok\n");
}

void inputUCINewGame(){
    init(start_position, strlen(start_position));
}

void inputPosition(char* input){

    char* cmd = input;
    cmd += 9;
    strcat(cmd, " ");
    if(startswith(cmd, "startpos ")){
        cmd += 9;
        init(start_position, strlen(start_position));
    }
    else if(startswith(cmd, "fen ")){
        cmd += 4;
        init(cmd, strlen(cmd));
    }
    if(startswith(cmd, "moves ")){
        cmd += 6;
        //TODO make moves accordingly
    }
}

void inputGo(char* input){
    char* cmd = input;
    cmd += 3;
    if(startswith(cmd, "depth ")){
        cmd += 6;
        int depth = atoi(cmd);
        struct Move result = calc_eng_move(depth);
        char* move_string = move_to_string(result);
        printf("bestmove %s\n", move_string);
    }
    else if(startswith(cmd, "perft ")){
        cmd += 6;
        int depth = atoi(cmd);
        unsigned long long a = detailed_perft(depth);
        printf("perft %llu\n", a);
    }
}

void uci_communication(){
    char command[256];

    init(start_position, strlen(start_position));

    while (fgets(command, sizeof(command), stdin)) {
        // remove newline character from the command
        //printf("info received command%s\n", command);
        command[strcspn(command, "\n")] = 0;

        if(str_equals(command, "uci")) {
            inputUCI();
        } else if(startswith(command, "setoption")) {
            inputSetOption();
        } else if(str_equals(command, "isready")) {
            inputIsReady();
        } else if(str_equals(command, "ucinewgame")) {
            inputUCINewGame();
        } else if(startswith(command, "position")) {
            printf("%s\n", command);
            inputPosition(command);
        } else if(startswith(command, "go")) {
            printf("%s\n", command);
            inputGo(command);
        } else if(startswith(command, "quit")) {
            break;
        } else {
            printf("Invalid command.\n");
        }
        fflush(stdout);
    }
}

int main(){
    uci_communication();
    /*init(start_position, strlen(start_position));
    draw_board();
    printf("\n");
    apply_move(11, 27, 0);
    draw_board();
    struct Move m = calc_eng_move(6);
    print_move(m);*/
    //printf("%llu", detailed_perft(4));
    //detailed_perft(4);
    return 0;
}
