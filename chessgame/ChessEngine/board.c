#include <ctype.h>
#include <stdlib.h>
#include "values.h"
#include "board.h"
#include "piece.h"
#include "bitwise.h"
#include "testing.h"
#include "transposition.h"

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

// remove a piece from the board, don't change next_spec
void remove_piece(unsigned char id, int square){
    unsigned char type = get_type(id);
    board[square] = EMPTY_SQUARE;
    piece_location[id] = -1;
    unsigned long long remove_mask = ~(1ULL << square);
    bitboards[type] &= remove_mask;
    mat_eval -= values[type];
    zobrist_hash ^= piece_keys[square][type];
    pos_eval -= square_incentive[type][square];

}

// remove a piece from the board, and unassign its spec
// this is only used for undoing promotion,
// otherwise unused specs would build up with add_piece
void destroy_piece(unsigned char id, int square){
    unsigned char type = get_type(id);
    next_spec[type]--;
    board[square] = EMPTY_SQUARE;
    piece_location[id] = -1;
    unsigned long long remove_mask = ~(1ULL << square);
    bitboards[type] &= remove_mask;
    mat_eval -= values[type];
    zobrist_hash ^= piece_keys[square][type];
    pos_eval -= square_incentive[type][square];
}

// add a new piece to the board, assigning it a new spec
unsigned char add_piece(unsigned char id, int square){
    unsigned char type = get_type(id);
    unsigned char spec = next_spec[type];
    next_spec[type]++;
    unsigned char new_id = id | spec;
    board[square] = new_id;
    piece_location[new_id] = square;
    unsigned long long add_mask = 1ULL << square;
    bitboards[type] |= add_mask;
    mat_eval += values[type];
    zobrist_hash ^= piece_keys[square][type];
    pos_eval += square_incentive[type][square];
    return new_id;
}

// place a previously-captured piece back on the board, don't change next_spec
void revive_piece(unsigned char id, int square){
    unsigned char type = get_type(id);
    board[square] = id;
    piece_location[id] = square;
    unsigned long long add_mask = 1ULL << square;
    bitboards[type] |= add_mask;
    mat_eval += values[type];
    zobrist_hash ^= piece_keys[square][type];
    pos_eval += square_incentive[type][square];
}

// moves an existing piece from start square to end square, don't change next_spec
void move_piece(unsigned char id, int start, int end){
    unsigned char type = get_type(id);
    board[start] = EMPTY_SQUARE;
    board[end] = id;
    piece_location[id] = end;
    unsigned long long remove_mask = ~(1ULL << start);
    bitboards[type] &= remove_mask;
    unsigned long long add_mask = 1ULL << end;
    bitboards[type] |= add_mask;
    zobrist_hash ^= piece_keys[start][type];
    zobrist_hash ^= piece_keys[end][type];
    pos_eval += square_incentive[type][end];
    pos_eval -= square_incentive[type][start];
}

void reset_board(){
    best_alpha = INT_MIN;
    best_beta = INT_MAX;

    mat_eval = 0;
    pos_eval = 0;
    zobrist_hash = 0;
    num_moves = 0;
    white_turn = true;


    for(int i = 0; i < 15; i++){
        bitboards[i] = 0ULL;
    }

    not_black_pieces = 0ULL;
    not_white_pieces = 0ULL;

    white_pieces = 0ULL;
    black_pieces = 0ULL;

    empty = 0ULL;
    occupied = 0ULL;

    unsafe_white = 0ULL;
    unsafe_black = 0ULL;

    white_check = false;
    black_check = false;

    num_pieces_delivering_check = 0;
    blocking_squares = 0ULL;

    // piece id of the 4 rooks you can castle with
    kingside_wR = EMPTY_SQUARE;
    queenside_wR = EMPTY_SQUARE;
    kingside_bR = EMPTY_SQUARE;
    queenside_bR = EMPTY_SQUARE;

    for(int i = 0; i < 64; i++){
        board[i] = EMPTY_SQUARE;
    }

    for(int i = 0; i < 256; i++){
        piece_location[i] = -1;
    }

    for(int i = 0; i < 15; i++){
        next_spec[i] = 0;
    }
}

// changes a letter num into a 0 to 63 num
int notation_to_number(char c, int i){
    int file_num = c;
    if (c >= 'A' && c <= 'H'){
        file_num += 32;
    }
    file_num -= 97;

    int square;

    square = (i-1)*8+(7-file_num);

    return square;

}

void append_move(Move* arr, Move m, int *i){
    arr[*i] = m;
    (*i)++;
}

void init_fen(char *fen, size_t fen_length){
    reset_board();
    int square = 63;
    char current = '_';
    int fen_section = 0;
    castling_rights = 0;
    for(int i = 0; i < fen_length; i++){
        current = fen[i];
        if (current == '\0'){
            printf("info error we reched the null caractor\n");
        }
        //found a space, done section
        if(current == ' '){
            fen_section += 1;
            continue;
        }
        if (fen_section == 0){
            //found a number, representing empty squares
            if(current >= '0' && current <= '9'){
                square -= (current - '0');
            }
            // found a slash, next row
            else if(current == '/'){
                continue;
            }
            //placing a piece
            else{
                char lower = tolower(current);
                unsigned char color = (current == lower)?BLACK:WHITE;
                unsigned char role = letter_to_piece_type(lower);
                unsigned char id = add_piece(color | role, square);
                if(role == ROOK){
                    if(color == WHITE){
                        if(square == 0){
                            kingside_wR = id;
                        }
                        else if(square == 7){
                            queenside_wR = id;
                        }
                    }
                    else{
                        if(square == 56){
                            kingside_bR = id;
                        }
                        else if(square == 63){
                            queenside_bR = id;
                        }
                    }
                }
                square -= 1;
            }
        }
        else if(fen_section == 1){
            //this is section 1
            if(current == 'b'){
                white_turn = false;
            }
            else{
                white_turn = true;
            }
            // this is the end of secton 1

        }
        //this is the start of section 2
        //this section handals casaling rights
        else if (fen_section == 2){
            if (current == 'K'){
                castling_rights |= CAN_CASTLE_WK;
            }
            else if (current == 'Q'){
                castling_rights |= CAN_CASTLE_WQ;
            }
            else if (current == 'k'){
                castling_rights |= CAN_CASTLE_BK;
            }
            else if (current == 'q'){
                castling_rights |= CAN_CASTLE_BQ;
            }
        }
        //end of section 2
        else if (fen_section == 3){
            //section 3 code
            //this handels in enpasont casle
           if(current != '-'){
                char en_passant_file = current;
                i += 1;
                current = fen[i];
                char en_passant_rank_char = current;

                int en_passant_rank = (int)en_passant_rank_char - 48;

                Move double_pawn;
                //this is notation to num eg. a1 to 7
                int square_num = notation_to_number(en_passant_file, en_passant_rank);
                double_pawn.move_id = DOUBLE_PAWN_PUSH;
                if(en_passant_rank == 6){
                    double_pawn.start = square_num + 8;
                    double_pawn.end = square_num - 8;
                }
                else if(en_passant_rank == 3){
                    double_pawn.start = square_num - 8;
                    double_pawn.end = square_num + 8;
                }
                double_pawn.piece_id = get_piece(double_pawn.end);
                append_move(move_list, double_pawn, &num_moves);
            }
        }
    }
}

int get_file(int n){
    return n % 8;
}

int get_rank(int n){
    return n / 8;
}

int get_r_diag(int n){
    return get_rank(n) + get_file(n);
}

int get_l_diag(int n){
    return get_rank(n) + 7 - get_file(n);
}

void add_moves_offset(unsigned long long mask, int start_offset, int end_offset, int min_id, int max_id, MoveList* move_lists){
    Move move;
    for(int i = 0; i < 64; i++){
        if((1ULL << i) & mask){
            for(int j = min_id; j <= max_id; j++){
                move.start = i + start_offset;
                move.end = i + end_offset;
                move.move_id = j;
                move.piece_id = get_piece(move.start);
                move.capture = get_piece(move.end);
                append_move(move_lists[ALL].moves, move, &move_lists[ALL].size);
            }
        }
    }
}

void add_moves_position(unsigned long long mask, int start_position, int min_id, int max_id, MoveList* move_lists){
    Move move;
    for(int i = 0; i < 64; i++){
        if((1ULL << i) & mask){
            for(int j = min_id; j <= max_id; j++){
                move.start = start_position;
                move.end = i;
                move.move_id = j;
                move.piece_id = get_piece(move.start);
                move.capture = get_piece(move.end);
                append_move(move_lists[ALL].moves, move, &move_lists[ALL].size);
            }
        }
    }
}

bool white_in_check(){
    return (bitboards[wK] & unsafe_white) > 0ULL;
}

bool black_in_check(){
    return (bitboards[bK] & unsafe_black) > 0ULL;
}

bool white_in_checkmate(int numMoves){
    // can't be in checkmate if it's not your turn
    if(!white_turn){
        return false;
    }
    // can't be in checkmate if you're not in check
    if(!white_check){
        return false;
    }
    // can't be in checkmate if you have legal moves
    if(numMoves > 0){
        return false;
    }
    return true;
}

bool black_in_checkmate(int numMoves){
    if(white_turn){
        return false;
    }
    if(!black_check){
        return false;
    }
    if(numMoves > 0){
        return false;
    }
    return true;
}

void init_board(char* fen, size_t len){
    init_fen(fen, len);
}

bool is_legal_move(int start, int end, int promo, Move* moves, size_t n){
    Move move;
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

bool apply_move(int start, int end, int move_id){

    if (white_turn){
        zobrist_hash ^= side_key;
    }
    unsigned char moved_piece = get_piece(start);
    unsigned char type = get_type(moved_piece);
    // not their turn to make a move
    if(white_turn != is_white_piece(moved_piece)){
        printf("\n\nNot your turn\nwhite_turn:%d\nstart:%d\nend:%d\nmove_id:%d\n\n", white_turn, start, end, move_id);
        draw_board();
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
    }
    // if the move was castling
    else if((type == wK || type == bK) && (abs(end - start) == 2)){
    }
    else{
        // previous move start, end, and move_id
       if(num_moves > 0){
            Move prev_move = move_list[num_moves - 1];
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
    Move move;
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

bool get_white_check(){
    return white_check;
}

bool get_black_check(){
    return black_check;
}

void undo_move(){
    // can't undo if nothing has been played
    if(num_moves == 0){
        return;
    }

    if (!white_turn){
        zobrist_hash ^= side_key;
    }

    //previous move (the one we're undoing)
    Move move = move_list[num_moves - 1];
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

bool try_undo_move(){
    if(num_moves > 0){
        undo_move();
        decr_num_moves();
        flip_turns();
        return true;
    }
    return false;
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

void init(char* fen, int len){
    init_board(fen, len);
}