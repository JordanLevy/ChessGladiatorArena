#include <stdbool.h>
#include <stdlib.h>
#include <ctype.h>
#include "values.h"
#include "piece.h"
#include "bitwise.h"
#include "board.h"
#include "testing.h"
#include <stdio.h>
#include <unistd.h>

unsigned long long** rook_moves_lookup;
unsigned long long* rook_magic_numbers;
unsigned long long rook_masks[64] = {282578800148862,565157600297596,1130315200595066,2260630401190006,4521260802379886,9042521604759646,18085043209519166,36170086419038334,282578800180736,565157600328704,1130315200625152,2260630401218048,4521260802403840,9042521604775424,18085043209518592,36170086419037696,282578808340736,565157608292864,1130315208328192,2260630408398848,4521260808540160,9042521608822784,18085043209388032,36170086418907136,282580897300736,565159647117824,1130317180306432,2260632246683648,4521262379438080,9042522644946944,18085043175964672,36170086385483776,283115671060736,565681586307584,1130822006735872,2261102847592448,4521664529305600,9042787892731904,18085034619584512,36170077829103616,420017753620736,699298018886144,1260057572672512,2381576680245248,4624614895390720,9110691325681664,18082844186263552,36167887395782656,35466950888980736,34905104758997504,34344362452452352,33222877839362048,30979908613181440,26493970160820224,17522093256097792,35607136465616896,9079539427579068672,8935706818303361536,8792156787827803136,8505056726876686336,7930856604974452736,6782456361169985536,4485655873561051136,9115426935197958144};

int* rook_magic_shift;

unsigned long long** bishop_moves_lookup;
unsigned long long* bishop_magic_numbers;
unsigned long long bishop_masks[64] = {18049651735527936,70506452091904,275415828992,1075975168,38021120,8657588224,2216338399232,567382630219776,9024825867763712,18049651735527424,70506452221952,275449643008,9733406720,2216342585344,567382630203392,1134765260406784,4512412933816832,9024825867633664,18049651768822272,70515108615168,2491752130560,567383701868544,1134765256220672,2269530512441344,2256206450263040,4512412900526080,9024834391117824,18051867805491712,637888545440768,1135039602493440,2269529440784384,4539058881568768,1128098963916800,2256197927833600,4514594912477184,9592139778506752,19184279556981248,2339762086609920,4538784537380864,9077569074761728,562958610993152,1125917221986304,2814792987328512,5629586008178688,11259172008099840,22518341868716544,9007336962655232,18014673925310464,2216338399232,4432676798464,11064376819712,22137335185408,44272556441600,87995357200384,35253226045952,70506452091904,567382630219776,1134765260406784,2832480465846272,5667157807464448,11333774449049600,22526811443298304,9024825867763712,18049651735527936};

int* bishop_magic_shift;

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

unsigned char letter_to_piece_type(char letter){
    char lower = tolower(letter);
    if(lower == 'p') return PAWN;
    if(lower == 'n') return KNIGHT;
    if(lower == 'b') return BISHOP;
    if(lower == 'r') return ROOK;
    if(lower == 'q') return QUEEN;
    if(lower == 'k') return KING;
    return EMPTY_SQUARE;
}

// returns the number of pieces with the same colortype, ignores spec
// e.g. how many white knights are on the board? num_identical_pieces(wK)
int get_num_identical_pieces(unsigned char id){
    return next_spec[get_type(id)];
}

unsigned long long span_piece(unsigned long long mask, int i, unsigned long long span, int origin, unsigned long long king_bb){
    unsigned long long squares = offset_span(span, origin, i);
    squares = remove_span_warps(squares, i);
    squares = squares & mask;
    if(squares & king_bb){
        num_pieces_delivering_check++;
        blocking_squares |= 1ULL << i;
    }
    return squares;
}

unsigned long long line_between_pieces(unsigned long long direction, int piece_1, int piece_2){
    unsigned long long mask_1;
    unsigned long long mask_2;
    if(!((1ULL << piece_1) & direction)){
        return 0ULL;
    }
    if(!((1ULL << piece_2) & direction)){
        return 0ULL;
    }
    if(piece_1 < piece_2){
        mask_1 = (all_squares >> (piece_1 + 1) << (piece_1 + 1));
        mask_2 = (all_squares << (64 - piece_2) >> (64 - piece_2));
    }
    else{
        mask_1 = (all_squares << (64 - piece_1) >> (64 - piece_1));
        mask_2 = (all_squares >> (piece_2 + 1) << (piece_2 + 1));
    }

    unsigned long long mask = mask_1 & mask_2;

    return direction & mask;
}

unsigned long long line_attack(unsigned long long o, unsigned long long m, unsigned long long s){
    unsigned long long o_and_m = o & m;

    unsigned long long rev_o_and_m = reverse(o_and_m);
    unsigned long long rev_s = reverse(s);

    unsigned long long two_s = 2*s;
    unsigned long long rev_two_s = 2*rev_s;

    unsigned long long left = o_and_m - two_s;
    unsigned long long right = reverse(rev_o_and_m - rev_two_s);

    unsigned long long left_xor_right = left ^ right;
    unsigned long long ans = left_xor_right & m;
    return ans;
}

bool is_white_piece(int id){
    return id & WHITE;
}

bool is_black_piece(int id){
    return id & BLACK;
}

void init_rook_magic(){
    //inishalis the magic nnumber array and the shif for a perticular magic number array
    rook_magic_numbers = malloc(64 * sizeof(unsigned long long));
    rook_magic_shift = malloc(64 * sizeof(int));
    //this is to get the rooks (where the blockers could be)

    char cwd[1024];

    // Get the current working directory
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("Current working directory: %s\n", cwd);
    } else {
        perror("getcwd() error");
    }


    // Open the file in read mode
    FILE *file = openFileInProjectFolder("./magic_rook_nums.txt", "r");

    // Check if the file was opened successfully
    if (file == NULL) {
        printf("Error opening the file.\n");
        return;
    }
    // Variables to store the values read from the file
    int pos, index, shift;
    unsigned long long magic, moves;
    rook_moves_lookup = malloc(64 * sizeof(unsigned long long*));

    if(rook_moves_lookup == NULL){
        printf("Error allocating rook_moves_lookup.\n");
        return;
    }

    int last_pos = -1;
    // Read each line from the file until the end
    // Use fscanf to parse values from the buffer
    while(fscanf(file,"%d %llu %d %d %llu", &pos, &magic, &shift, &index, &moves) == 5){
        if(pos != last_pos){
            rook_moves_lookup[pos] = malloc((1 << (64-shift)) * sizeof(unsigned long long));
            if(rook_moves_lookup[pos] == NULL){
                printf("Error allocating rook_moves_lookup[%d].\n", pos);
                return;
            }
        }
        //printf("%d %llu %d %d\n", pos, magic, shift, index);
        //print_bitboard(moves);
        rook_moves_lookup[pos][index] = moves;
        rook_magic_numbers[pos] = magic;
        rook_magic_shift[pos] = shift;
        last_pos = pos;
    }
    // Close the file
    fclose(file);
}

void init_bishop_magic(){
    //inishalis the magic nnumber array and the shif for a perticular magic number array
    bishop_magic_numbers = malloc(64 * sizeof(unsigned long long));
    bishop_magic_shift = malloc(64 * sizeof(int));
    //this is to get the rooks (where the blockers could be)

    // Open the file in read mode
    FILE *file = openFileInProjectFolder("./magic_bishop_nums.txt", "r");

    // Check if the file was opened successfully
    if (file == NULL) {
        printf("Error opening the file.\n");
        return;
    }
    // Variables to store the values read from the file
    int pos, index, shift;
    unsigned long long magic, moves;
    bishop_moves_lookup = malloc(64 * sizeof(unsigned long long*));

    if(bishop_moves_lookup == NULL){
        printf("Error allocating bishop_moves_lookup.\n");
        return;
    }

    int last_pos = -1;
    // Read each line from the file until the end
    // Use fscanf to parse values from the buffer
    while(fscanf(file,"%d %llu %d %d %llu", &pos, &magic, &shift, &index, &moves) == 5){
        if(pos != last_pos){
            bishop_moves_lookup[pos] = malloc((1 << (64-shift)) * sizeof(unsigned long long));
            if(bishop_moves_lookup[pos] == NULL){
                printf("Error allocating bishop_moves_lookup[%d].\n", pos);
                return;
            }
        }
        //printf("%d %llu %d %d\n", pos, magic, shift, index);
        //print_bitboard(moves);
        bishop_moves_lookup[pos][index] = moves;
        bishop_magic_numbers[pos] = magic;
        bishop_magic_shift[pos] = shift;
        last_pos = pos;
    }
    // Close the file
    fclose(file);
}

void init_magic(){
    init_rook_magic();
    init_bishop_magic();
}

unsigned long long sliding_piece(unsigned long long mask, int location, unsigned long long blockers, bool rook_moves, bool bishop_moves, unsigned long long king_bb){

    // squares this piece threatens, as a bitboard
    unsigned long long squares = 0ULL;
    // bitboard representing this piece's location
    unsigned long long slider = 1ULL << location;
    // int 0-63 of which square the enemy king is on
    int king_square = 64 - leading_zeros(king_bb) - 1;
    // whether or not the king is white
    bool king_color = is_white_piece(get_piece(king_square));

    // array of bitboards, each representing a file/rank/diagonal this piece can move along
    unsigned long long directions[4] = { 0ULL };
    // if it moves like a rook, it can move along the rank and file it's on
    if(rook_moves){
        directions[0] = rank[get_rank(location) + 1];
        directions[1] = file[8 - get_file(location)];
    }
    // if it moves like a bishop, it can move along the left and right diagonals it's on
    if(bishop_moves){
        directions[2] = l_diag[get_l_diag(location)];
        directions[3] = r_diag[get_r_diag(location)];
    }

    unsigned long long d = 0ULL;
    unsigned long long new_squares = 0ULL;
    // bitboard with ones on a straight line from this piece to the enemy king
    unsigned long long king_line = 0ULL;
    // bitboard possible pinning line
    unsigned long long pos_pin = 0ULL;
    // number of pieces along the possible pinning line
    int counter = 0;
    // location of up to 2 pieces on the possible pinning line
    // only one piece can be pinned, but 2 pawns can be pinned w/ en passant
    int pin_loc[] = {-1, -1};

    // for each direction this piece can move along
    for(int k = 0; k < 4; k++){
        // set d to this current direction
        d = directions[k];
        // if it can't move in this direction, continue to the next direction
        if(d == 0ULL){
            continue;
        }
        // get a bitboard of squares threatened in this direction
        new_squares = line_attack(blockers, d, slider) & mask;
        // add them to the bitboard of squares this piece threatens
        squares |= new_squares;
        // if the enemy king is in the line of sight of this piece in this direction
        if(new_squares & king_bb){
            //printf("%d looking at king in direction %d\n", i, k);
            num_pieces_delivering_check++;
            blocking_squares |= line_between_pieces(d, location, king_square);
            blocking_squares |= slider;
        }
        // if this function was called with the king_bb parameter (checking for pins)
        else if(king_square >= 0){
            // draw a line from this piece to the enemy king
            king_line = line_between_pieces(d, location, king_square);
            // if there is no such line, continue. Piece doesn't pin in this direction
            if(!king_line){
                continue;
            }
            // pieces on the line between the piece and the king could possibly be pinned
            pos_pin = blockers & king_line;

            // count and store the first two pieces on the king line
            counter = 0;
            pin_loc[0] = -1;
            pin_loc[1] = -1;
            for(int i = 0; i < 64; i++){
                if((1ULL << i) & pos_pin){
                    counter += 1;
                    if(counter > 2){
                        break;
                    }
                    pin_loc[counter - 1] = i;
                }
            }
            // if there is only one piece on the pinning line
            if(counter == 1){
                bool pinned_piece_color = is_white_piece(get_piece(pin_loc[0]));
                // if the piece is the same color as the king, consider it pinned
                if(pinned_piece_color == king_color){
                    // pinned piece may capture the piece that delivers the pin
                    king_line = king_line | slider;
                    // pinned piece is restricted to only move along the pinning line
                    pinning_squares[pin_loc[0]] = king_line;
                }
            }
            // if there are two pieces on this pinning line
            else if(counter == 2){
                int p1 = get_piece(pin_loc[0]);
                int p2 = get_piece(pin_loc[1]);
                // and they are opposite colored pawns
                if((p1 == 1 && p2 == 7) || (p1 == 7 && p2 == 1)){
                    // and this isn't the first move of the game
                    if(num_moves > 0){
                        // get the previous move
                        Move move = move_list[num_moves - 1];
                        // and the previous move was a double pawn push
                        if(move.move_id == 13){
                            // we only care about this en passant pinning situation horizontally
                            // everything else is handled via regular pins
                            if(get_rank(pin_loc[0]) == get_rank(pin_loc[1])){
                                // if it's a white pawn
                                if(is_white_piece(get_piece(move.end))){
                                    // mark that it is illegal for white to capture en passant on that square
                                    en_passant_pinned = move.end - 8;
                                }
                                else{
                                    // mark that it is illegal for black to capture en passant on that square
                                    en_passant_pinned = move.end + 8;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return squares;
}

void possible_P(unsigned long long bb, unsigned long long can_capture, unsigned long long promo_rank, unsigned long long enemy_pawns, unsigned long long double_push_rank, int fwd, unsigned char color, MoveList* move_lists){
    bool is_white = (color == WHITE);
    int e = 0;
    int m = 0;
    Move prev_move;
    if(num_moves > 0){
        prev_move = move_list[num_moves - 1];
        e = prev_move.end;
        m = prev_move.move_id;
    }

    int promo_min = bN;
    int promo_max = bQ;

    if(is_white){
        promo_min = wN;
        promo_max = wQ;
    }

    unsigned long long not_promo_rank = ~promo_rank;

    // capture right
    unsigned long long mask = l_shift(bb, fwd * 8 - 1) & can_capture & not_promo_rank & ~file[1];
    add_moves_offset(mask, -(fwd * 8 - 1), 0, 0, 0, move_lists);

    // capture left
    mask = l_shift(bb, fwd * 8 + 1) & can_capture & not_promo_rank & ~file[8];
    add_moves_offset(mask, -(fwd * 8 + 1), 0, 0, 0, move_lists);

    // one forward
    mask = l_shift(bb, fwd * 8) & empty & not_promo_rank;
    add_moves_offset(mask, -fwd * 8, 0, 0, 0, move_lists);

    // two forward
    mask = l_shift(bb, 2 * fwd * 8) & empty & l_shift(empty, fwd * 8) & double_push_rank;
    add_moves_offset(mask, -(2 * fwd * 8), 0, 0, 0, move_lists);

    // promotion by capture right
    mask = l_shift(bb, fwd * 8 - 1) & can_capture & promo_rank & ~file[1];
    add_moves_offset(mask, -(fwd * 8 - 1), 0, promo_min, promo_max, move_lists);

    // promotion by capture left
    mask = l_shift(bb, fwd * 8 + 1) & can_capture & promo_rank & ~file[8];
    add_moves_offset(mask, -(fwd * 8 + 1), 0, promo_min, promo_max, move_lists);

    // promotion by one forward
    mask = l_shift(bb, fwd * 8) & empty & promo_rank;
    add_moves_offset(mask, -fwd * 8, 0, promo_min, promo_max, move_lists);

    // if the previous move was a double pawn push, en passant might be possible
    if(m == DOUBLE_PAWN_PUSH){
        unsigned long long pushed_pawn_location = 1ULL << e;

        // left en passant
        mask = l_shift(bb, 1) & enemy_pawns & not_promo_rank & ~file[8] & pushed_pawn_location;
        add_moves_offset(mask, -1, fwd * 8, 0, 0, move_lists);

        // right en passant
        mask = l_shift(bb, -1) & enemy_pawns & not_promo_rank & ~file[1] & pushed_pawn_location;
        add_moves_offset(mask, 1, fwd * 8, 0, 0, move_lists);
    }
}

void possible_wP(unsigned long long bb, MoveList* move_lists){
    possible_P(bb, black_pieces, rank[8], bitboards[bP], rank[4], 1, WHITE, move_lists);
}

void possible_bP(unsigned long long bb, MoveList* move_lists){
    possible_P(bb, white_pieces, rank[1], bitboards[wP], rank[5], -1, BLACK, move_lists);
}

void possible_N(unsigned long long bb, unsigned long long mask, unsigned char color, MoveList* move_lists){
    unsigned char type = get_type(color | KNIGHT);
    for(int i = 0; i < next_spec[type]; i++){
        unsigned char id = color | KNIGHT | i;
        int location = piece_location[id];
        if(location == -1) continue;
        add_moves_position(span_piece(mask, location, knight_span, 18, 0ULL), location, 0, 0, move_lists);
    }
}

void possible_B(unsigned long long bb, unsigned long long mask, unsigned char color, MoveList* move_lists){
    unsigned char type = get_type(color | BISHOP);
    for(int i = 0; i < next_spec[type]; i++){
        unsigned char id = color | BISHOP | i;
        int location = piece_location[id];
        if(location == -1) continue;
        if(bishop_magic_enabled){
            unsigned long long blockers = occupied & bishop_masks[location];
            int index = get_index_from_magic(blockers, bishop_magic_numbers[location], bishop_magic_shift[location]);
            add_moves_position(bishop_moves_lookup[location][index], location, 0, 0, move_lists);  
        }
        else{
            add_moves_position(sliding_piece(mask, location, occupied, false, true, 0ULL), location, 0, 0, move_lists);
        }
    }
}

void possible_R(unsigned long long bb, unsigned long long mask, unsigned char color, MoveList* move_lists){
    unsigned char type = get_type(color | ROOK);
    //for all the rooks 4 ish times
    for(int i = 0; i < next_spec[type]; i++){
        unsigned char id = color | ROOK | i;
        int location = piece_location[id];
        if(location == -1) continue;
        if(rook_magic_enabled){
            unsigned long long blockers = occupied & rook_masks[location];
            int index = get_index_from_magic(blockers, rook_magic_numbers[location], rook_magic_shift[location]);
            add_moves_position(rook_moves_lookup[location][index], location, 0, 0, move_lists);  
        }
        else{
            add_moves_position(sliding_piece(mask, location, occupied, true, false, 0ULL), location, 0, 0, move_lists);
        }
    }
}

void possible_Q(unsigned long long bb, unsigned long long mask, unsigned char color, MoveList* move_lists){
    unsigned char type = get_type(color | QUEEN);
    for(int i = 0; i < next_spec[type]; i++){
        unsigned char id = color | QUEEN | i;
        int location = piece_location[id];
        if(location == -1) continue;
        if(queen_magic_enabled){
            unsigned long long legal_moves = 0;

            unsigned long long movement_mask = rook_masks[location];
            unsigned long long rook_blockers = occupied & movement_mask;
            int index = get_index_from_magic(rook_blockers, rook_magic_numbers[location], rook_magic_shift[location]);
            legal_moves |= rook_moves_lookup[location][index];

            unsigned long long movement_mask = bishop_masks[location];
            unsigned long long bishop_blockers = occupied & movement_mask;
            index = get_index_from_magic(bishop_blockers, bishop_magic_numbers[location], bishop_magic_shift[location]);
            legal_moves |= bishop_moves_lookup[location][index];

            add_moves_position(legal_moves, location, 0, 0, move_lists);
        }
        else{
            add_moves_position(sliding_piece(mask, location, occupied, true, true, 0ULL), location, 0, 0, move_lists);
        }
    }
}

void possible_K(unsigned long long bb, unsigned long long mask, unsigned char color, MoveList* move_lists){
    unsigned long long squares = 0ULL;
    unsigned long long safe = ~unsafe_white;
    bool is_white = color == WHITE;
    if(!is_white){
        safe = ~unsafe_black;
    }
    unsigned long long empty_and_safe = empty & safe;

    unsigned char type = get_type(color | KING);
    for(int i = 0; i < next_spec[type]; i++){
        unsigned char id = color | KING | i;
        int location = piece_location[id];
        if(location == -1) continue;
        add_moves_position(span_piece((mask & safe), location, king_span, 9, 0ULL), location, 0, 0, move_lists);
    }

    // if the king is in check, king cannot castle
    if(!(bb & safe)){
        return;
    }
    // this is white king, hasn't moved yet
    if(is_white && wK_num_moves == 0){
        // white queenside castle
        if(piece_location[queenside_wR] == 7 && queenside_wR_num_moves == 0){
            squares = l_shift(bb, 2) & l_shift(empty_and_safe, 1) & empty_and_safe & l_shift(empty, -1);
            add_moves_offset(squares, -2, 0, 0, 0, move_lists);
        }
        // white kingside castle
        if(piece_location[kingside_wR] == 0 && kingside_wR_num_moves == 0){
            squares = l_shift(bb, -2) & l_shift(empty_and_safe, -1) & empty_and_safe;
            add_moves_offset(squares, 2, 0, 0, 0, move_lists);
        }
    }
    // this is black king, hasn't moved yet
    else if(!is_white && bK_num_moves == 0){
        // black queenside castle
        if(piece_location[queenside_bR] == 63 && queenside_bR_num_moves == 0){
            squares = l_shift(bb, 2) & l_shift(empty_and_safe, 1) & empty_and_safe & l_shift(empty, -1);
            add_moves_offset(squares, -2, 0, 0, 0, move_lists);
        }
        // black kingside castle
        if(piece_location[kingside_bR] == 56 && kingside_bR_num_moves == 0){
            squares = l_shift(bb, -2) & l_shift(empty_and_safe, -1) & empty_and_safe;
            add_moves_offset(squares, 2, 0, 0, 0, move_lists);
        }
    }
}

void update_piece_masks(){
    white_pieces = bitboards[wP] | bitboards[wN] | bitboards[wB] | bitboards[wR] | bitboards[wQ];
    black_pieces = bitboards[bP] | bitboards[bN] | bitboards[bB] | bitboards[bR] | bitboards[bQ];
    not_white_pieces = ~(white_pieces | bitboards[wK] | bitboards[bK]);
    not_black_pieces = ~(black_pieces | bitboards[wK] | bitboards[bK]);
    empty = ~(white_pieces | black_pieces | bitboards[wK] | bitboards[bK]);
    occupied = ~empty;
}

void possible_moves_white(MoveList* move_lists){
    update_piece_masks();
    update_unsafe();
    //2 is jenarick because we have 2 lists right now 
    //this could change in the futcher
    move_lists[ALL].size = 0;
    possible_wP(bitboards[wP], move_lists);
    possible_N(bitboards[wN], not_white_pieces, WHITE, move_lists);
    possible_B(bitboards[wB], not_white_pieces, WHITE, move_lists);
    possible_R(bitboards[wR], not_white_pieces, WHITE, move_lists);
    possible_Q(bitboards[wQ], not_white_pieces, WHITE, move_lists);
    possible_K(bitboards[wK], not_white_pieces, WHITE, move_lists);
}

void possible_moves_black(MoveList* move_lists){
    update_piece_masks();
    update_unsafe();
    //2 is jenarick because we have 2 lists right now 
    //this could change in the futcher
    move_lists[ALL].size = 0;
    possible_bP(bitboards[bP], move_lists);
    possible_N(bitboards[bN], not_black_pieces, BLACK, move_lists);
    possible_B(bitboards[bB], not_black_pieces, BLACK, move_lists);
    possible_R(bitboards[bR], not_black_pieces, BLACK, move_lists);
    possible_Q(bitboards[bQ], not_black_pieces, BLACK, move_lists);
    possible_K(bitboards[bK], not_black_pieces, BLACK, move_lists);
}

void update_possible_moves(MoveList* move_lists){
    if(white_turn){
        possible_moves_white(move_lists);
    }
    else{
        possible_moves_black(move_lists);
    }
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