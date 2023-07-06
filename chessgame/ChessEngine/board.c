#include <ctype.h>
#include "values.h"
#include "board.h"
#include "piece.h"
#include "bitwise.h"

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
    pos_eval += square_incentive[type][end];
    pos_eval -= square_incentive[type][start];
}

void init_masks(){
    all_squares = ~0ULL;
    square_a8 = 1ULL << 63;

    int ns[] = {1, 8, 24, 33, 35, 28, 12, 3};
    knight_span = generate_bitboard(ns, 8);

    int ks[] = {0, 1, 2, 8, 10, 16, 17, 18};
    king_span = generate_bitboard(ks, 8);

    int f[] = {7, 15, 23, 31, 39, 47, 55, 63};
    file[1] = generate_bitboard(f, 8);
    for(int i = 1; i < 8; i++){
        file[i + 1] = file[i] >> 1;
    }

    rank[1] = generate_bitboard_from_range(0, 7);
    for(int i = 1; i < 8; i++){
        rank[i + 1] = rank[i] << 8;
    }

    int left = 0;
    int right = 0;
    for(int i = 0; i < 64; i++){
        left = get_l_diag(i);
        right = get_r_diag(i);
        l_diag[left] |= 1ULL << i;
        r_diag[right] |= 1ULL << i;
    }

    file_ab = file[1] | file[2];
    file_gh = file[7] | file[8];
}

void reset_board(){
    best_alpha = INT_MIN;
    best_beta = INT_MAX;

    mat_eval = 0;
    pos_eval = 0;
    num_moves = 0;
    white_turn = true;

    for(int i = 0; i < 15; i++){
        bitboards[i] = 0ULL;
    }

    not_black_pieces = 0ULL;
    not_white_pieces = 0ULL;
    all_squares = 0ULL;

    white_pieces = 0ULL;
    black_pieces = 0ULL;

    empty = 0ULL;
    occupied = 0ULL;
    for(int i = 0; i < 9; i++){
        file[i] = 0ULL;
        rank[i] = 0ULL;
    }
    for(int i = 0; i < 15; i++){
        l_diag[i] = 0ULL;
        r_diag[i] = 0ULL;
    }

    square_a8 = 0ULL;

    knight_span = 0ULL;
    king_span = 0ULL;

    file_ab = 0ULL;
    file_gh = 0ULL;

    unsafe_white = 0ULL;
    unsafe_black = 0ULL;

    white_check = false;
    black_check = false;

    num_pieces_delivering_check = 0;
    blocking_squares = 0ULL;
    for(int i = 0; i < 64; i++){
        pinning_squares[i] = 0ULL;
    }
    en_passant_pinned = -1;

    // piece id of the 4 rooks you can castle with
    kingside_wR = EMPTY_SQUARE;
    queenside_wR = EMPTY_SQUARE;
    kingside_bR = EMPTY_SQUARE;
    queenside_bR = EMPTY_SQUARE;

    // number of times each of the 4 castling rooks has moved
    kingside_wR_num_moves = 0;
    queenside_wR_num_moves = 0;
    kingside_bR_num_moves = 0;
    queenside_bR_num_moves = 0;

    // number of times each king has moved
    wK_num_moves = 0;
    bK_num_moves = 0;

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

void append_move(struct Move* arr, struct Move m, int *i){
    arr[*i] = m;
    (*i)++;
}

void init_fen(char *fen, size_t fen_length){
    reset_board();
    int square = 63;
    char current = '_';
    int fen_section = 0;
    kingside_wR_num_moves = 1;
    queenside_wR_num_moves = 1;
    kingside_bR_num_moves = 1;
    queenside_bR_num_moves = 1;
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
                kingside_wR_num_moves = 0;
            }
            else if (current == 'Q'){
                queenside_wR_num_moves = 0;
            }
            else if (current == 'k'){
                kingside_bR_num_moves = 0;
            }
            else if (current == 'q'){
                queenside_bR_num_moves = 0;
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

                struct Move double_pawn;
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

bool resolves_check(int start, int end, int move_id){
    unsigned char moved_piece = get_piece(start);
    unsigned char type = get_type(moved_piece);
    if(white_turn){
        // trying to move a pinned piece
        if(pinning_squares[start]){
            unsigned long long pinning_line = pinning_squares[start];
            if(!((1ULL << end) & pinning_line)){
                return false;
            }
        }
        // if white pawn trying to capture en passant but it's en passant pinned
        if((type == wP) && (end == en_passant_pinned)){
            return false;
        }
        // if it's a king move
        if(type == wK){
            // and the king ends up on an unsafe square
            if((1ULL << end) & unsafe_white){
                return false;
            }
        }
        // if white is currently in check
        else if(white_check){
            // if it's a double check
            if(num_pieces_delivering_check > 1){
                // king must move
                if(type != wK){
                    return false;
                }
                // moving to an unsafe square
                if((1ULL << end) & unsafe_white){
                    return false;
                }
            }
            // and checking piece isn't being blocked or captured
            if(!((1ULL << end) & blocking_squares)){
                return false;
            }
        }
    }
    else{
        if(pinning_squares[start]){
            unsigned long long pinning_line = pinning_squares[start];
            if(!((1ULL << end) & pinning_line)){
                return false;
            }
        }
        if((type == bP) && (end == en_passant_pinned)){
            return false;
        }
        if(type == bK){
            if((1ULL << end) & unsafe_black){
                return false;
            }
        }
        else if(black_check){
            if(num_pieces_delivering_check > 1){
                if(type != bK){
                    return false;
                }
                // moving to an unsafe square
                if((1ULL << end) & unsafe_black){
                    return false;
                }
            }
            if(!((1ULL << end) & blocking_squares)){
                return false;
            }
        }
    }
    return true;
}

void add_moves_offset(unsigned long long mask, int start_offset, int end_offset, int min_id, int max_id, struct Move* moves, int *numElems){
    struct Move move;
    for(int i = 0; i < 64; i++){
        if((1ULL << i) & mask){
            for(int j = min_id; j <= max_id; j++){
                if(resolves_check(i + start_offset, i + end_offset, j)){
                    move.start = i + start_offset;
                    move.end = i + end_offset;
                    move.move_id = j;
                    move.piece_id = get_piece(move.start);
                    move.capture = get_piece(move.end);
                    append_move(moves, move, numElems);
                }
            }
        }
    }
}

void add_moves_position(unsigned long long mask, int start_position, int min_id, int max_id, struct Move* moves, int *numElems){
    struct Move move;
    for(int i = 0; i < 64; i++){
        if((1ULL << i) & mask){
            for(int j = min_id; j <= max_id; j++){
                if(resolves_check(start_position, i, j)){
                    move.start = start_position;
                    move.end = i;
                    move.move_id = j;
                    move.piece_id = get_piece(move.start);
                    move.capture = get_piece(move.end);
                    append_move(moves, move, numElems);
                }
            }
        }
    }
}

unsigned long long unsafe_for_white(){
    num_pieces_delivering_check = 0;
    blocking_squares = 0ULL;
    for(int i = 0; i < 64; i++){
        pinning_squares[i] = 0ULL;
    }
    en_passant_pinned = -1;

    unsigned long long unsafe = 0ULL;
    unsigned long long mask = 0ULL;

    unsigned long long king = bitboards[wK];
    unsigned long long occupied_no_king = occupied & ~king;

    unsigned long long checking_pawn = 0ULL;

    // pawns
    // threaten to capture right
    mask = l_shift(bitboards[bP], -8 - 1) & ~file[1];
    unsafe |= mask;
    // the pawn that's delivering check
    checking_pawn = r_shift((king & mask), -8 - 1);
    if(checking_pawn != 0ULL){
        num_pieces_delivering_check++;
        blocking_squares |= checking_pawn;
    }


    // threaten to capture left
    mask = l_shift(bitboards[bP], -8 + 1) & ~file[8];
    unsafe |= mask;
    checking_pawn = r_shift((king & mask), -8 + 1);
    if(checking_pawn != 0ULL){
        num_pieces_delivering_check++;
        blocking_squares |= checking_pawn;
    }

    // knight
    for(int i = 0; i < next_spec[bN]; i++){
        unsigned char id = BLACK | KNIGHT | i;
        int location = piece_location[id];
        if(location == -1) continue;
        mask = span_piece(all_squares, location, knight_span, 18, king);
        unsafe |= mask;
    }

    // king
    for(int i = 0; i < next_spec[bK]; i++){
        unsigned char id = BLACK | KING | i;
        int location = piece_location[id];
        if(location == -1) continue;
        mask = span_piece(all_squares, location, king_span, 9, 0ULL);
        unsafe |= mask;
    }

    // queen
    for(int i = 0; i < next_spec[bQ]; i++){
        unsigned char id = BLACK | QUEEN | i;
        int location = piece_location[id];
        if(location == -1) continue;
        mask = sliding_piece(all_squares, location, occupied_no_king, true, true, king);
        unsafe |= mask;
    }

    // rook
    for(int i = 0; i < next_spec[bR]; i++){
        unsigned char id = BLACK | ROOK | i;
        int location = piece_location[id];
        if(location == -1) continue;
        mask = sliding_piece(all_squares, location, occupied_no_king, true, false, king);
        unsafe |= mask;
    }

    // bishop
    for(int i = 0; i < next_spec[bB]; i++){
        unsigned char id = BLACK | BISHOP | i;
        int location = piece_location[id];
        if(location == -1) continue;
        mask = sliding_piece(all_squares, location, occupied_no_king, false, true, king);
        unsafe |= mask;
    }

    return unsafe;
}

unsigned long long unsafe_for_black(){
    unsigned long long unsafe = 0ULL;
    unsigned long long mask = 0ULL;

    unsigned long long king = bitboards[bK];
    unsigned long long occupied_no_king = occupied & ~king;

    // pawns
    // threaten to capture right
    mask = l_shift(bitboards[wP], 8 - 1) & ~file[1];
    unsafe |= mask;
    blocking_squares |= r_shift((king & mask), 8 - 1);

    // threaten to capture left
    mask = l_shift(bitboards[wP], 8 + 1) & ~file[8];
    unsafe |= mask;
    blocking_squares |= r_shift((king & mask), 8 + 1);

    // knight
    for(int i = 0; i < next_spec[wN]; i++){
        unsigned char id = WHITE | KNIGHT | i;
        int location = piece_location[id];
        if(location == -1) continue;
        mask = span_piece(all_squares, location, knight_span, 18, king);
        unsafe |= mask;
    }

    // king
    for(int i = 0; i < next_spec[wK]; i++){
        unsigned char id = WHITE | KING | i;
        int location = piece_location[id];
        if(location == -1) continue;
        mask = span_piece(all_squares, location, king_span, 9, 0ULL);
        unsafe |= mask;
    }

    // queens
    for(int i = 0; i < next_spec[wQ]; i++){
        unsigned char id = WHITE | QUEEN | i;
        int location = piece_location[id];
        if(location == -1) continue;
        mask = sliding_piece(all_squares, location, occupied_no_king, true, true, king);
        unsafe |= mask;
    }

    // rook
    for(int i = 0; i < next_spec[wR]; i++){
        unsigned char id = WHITE | ROOK | i;
        int location = piece_location[id];
        if(location == -1) continue;
        mask = sliding_piece(all_squares, location, occupied_no_king, true, false, king);
        unsafe |= mask;
    }

    // bishop
    for(int i = 0; i < next_spec[wB]; i++){
        unsigned char id = WHITE | BISHOP | i;
        int location = piece_location[id];
        if(location == -1) continue;
        mask = sliding_piece(all_squares, location, occupied_no_king, false, true, king);
        unsafe |= mask;
    }

    return unsafe;
}

bool white_in_check(){
    return (bitboards[wK] & unsafe_white) > 0ULL;
}

bool black_in_check(){
    return (bitboards[bK] & unsafe_black) > 0ULL;
}