#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <limits.h>

#define len(x)  (sizeof(x) / sizeof((x)[0]))

/*
Things to optimize:
unsafe_for_white/black have to recalculate most of the same stuff
remove_piece has nested for loops (pieces array refactor?)
only recalculating the legal moves that actually are affected


store the legal moves per piece indexed by square as a board

list of 64 move list
*/

struct Move {
    int start;
    int end;
    int id;
    int capture;
    int piece;
    int eval;
};

struct Move engine_move;

// this is the pos_eval gride

int square_incentive[13][64] =
{
{ 0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0},

{ 0,   0,   0,   0,   0,   0,   0,   0,
   5,  10,  10, -20, -20,  10,  10,   5,
   5,  -5, -10,   0,   0, -10,  -5,   5,
   0,   0,   0,  20,  20,   0,   0,   0,
   5,   5,  10,  25,  25,  10,   5,   5,
  10,  10,  20,  30,  30,  20,  10,  10,
  50,  50,  50,  50,  50,  50,  50,  50,
   0,   0,   0,   0,   0,   0,   0,   0},

{ -50, -40, -30, -30, -30, -30, -40, -50,
 -40, -20,   0,   5,   5,   0, -20, -40,
 -30,   5,  10,  15,  15,  10,   5, -30,
 -30,   0,  15,  20,  20,  15,   0, -30,
 -30,   5,  15,  20,  20,  15,   5, -30,
 -30,   0,  10,  15,  15,  10,   0, -30,
 -40, -20,   0,   0,   0,   0, -20, -40,
 -50, -40, -30, -30, -30, -30, -40, -50},

{ -20, -10, -10, -10, -10, -10, -10, -20,
 -10,   5,   0,   0,   0,   0,   5, -10,
 -10,  10,  10,  10,  10,  10,  10, -10,
 -10,   0,  10,  10,  10,  10,   0, -10,
 -10,   5,   5,  10,  10,   5,   5, -10,
 -10,   0,   5,  10,  10,   5,   0, -10,
 -10,   0,   0,   0,   0,   0,   0, -10,
 -20, -10, -10, -10, -10, -10, -10, -20},

{  0,   0,   0,   5,   5,   0,   0,   0,
  -5,   0,   0,   0,   0,   0,   0,  -5,
  -5,   0,   0,   0,   0,   0,   0,  -5,
  -5,   0,   0,   0,   0,   0,   0,  -5,
  -5,   0,   0,   0,   0,   0,   0,  -5,
  -5,   0,   0,   0,   0,   0,   0,  -5,
   5,  10,  10,  10,  10,  10,  10,   5,
   0,   0,   0,   0,   0,   0,   0,   0},

{ -20, -10, -10,  -5,  -5, -10, -10, -20,
 -10,   0,   0,   0,   0,   5,   0, -10,
 -10,   0,   5,   5,   5,   5,   5, -10,
  -5,   0,   5,   5,   5,   5,   0,   0,
  -5,   0,   5,   5,   5,   5,   0,  -5,
 -10,   0,   5,   5,   5,   5,   0, -10,
 -10,   0,   0,   0,   0,   0,   0, -10,
 -20, -10, -10,  -5,  -5, -10, -10, -20},

{  20,  30,  10,   0,   0,  10,  30,  20,
  20,  20,   0,   0,   0,   0,  20,  20,
 -10, -20, -20, -20, -20, -20, -20, -10,
 -20, -30, -30, -40, -40, -30, -30, -20,
 -30, -40, -40, -50, -50, -40, -40, -30,
 -30, -40, -40, -50, -50, -40, -40, -30,
 -30, -40, -40, -50, -50, -40, -40, -30,
 -30, -40, -40, -50, -50, -40, -40, -30},

 { 0,   0,   0,   0,   0,   0,   0,   0,
 -50, -50, -50, -50, -50, -50, -50, -50,
 -10, -10, -20, -30, -30, -20, -10, -10,
  -5,  -5, -10, -25, -25, -10,  -5,  -5,
   0,   0,   0, -20, -20,   0,   0,   0,
  -5,   5,  10,   0,   0,  10,   5,  -5,
  -5, -10, -10,  20,  20, -10, -10,  -5,
   0,   0,   0,   0,   0,   0,   0,   0},

{ 50,  40,  30,  30,  30,  30,  40,  50,
  40,  20,   0,   0,   0,   0,  20,  40,
  30,   0, -10, -15, -15, -10,   0,  30,
  30,  -5, -15, -20, -20, -15,  -5,  30,
  30,   0, -15, -20, -20, -15,   0,  30,
  30,  -5, -10, -15, -15, -10,  -5,  30,
  40,  20,   0,  -5,  -5,   0,  20,  40,
  50,  40,  30,  30,  30,  30,  40,  50},

{ 20,  10,  10,  10,  10,  10,  10,  20,
  10,   0,   0,   0,   0,   0,   0,  10,
  10,   0,  -5, -10, -10,  -5,   0,  10,
  10,  -5,  -5, -10, -10,  -5,  -5,  10,
  10,   0, -10, -10, -10, -10,   0,  10,
  10, -10, -10, -10, -10, -10, -10,  10,
  10,  -5,   0,   0,   0,   0,  -5,  10,
  20,  10,  10,  10,  10,  10,  10,  20},

{  0,   0,   0,   0,   0,   0,   0,   0,
  -5, -10, -10, -10, -10, -10, -10,  -5,
   5,   0,   0,   0,   0,   0,   0,   5,
   5,   0,   0,   0,   0,   0,   0,   5,
   5,   0,   0,   0,   0,   0,   0,   5,
   5,   0,   0,   0,   0,   0,   0,   5,
   5,   0,   0,   0,   0,   0,   0,   5,
   0,   0,   0,  -5,  -5,   0,   0,   0},

{ 20,  10,  10,   5,   5,  10,  10,  20,
  10,   0,   0,   0,   0,   0,   0,  10,
  10,   0,  -5,  -5,  -5,  -5,   0,  10,
   5,   0,  -5,  -5,  -5,  -5,   0,   5,
   0,   0,  -5,  -5,  -5,  -5,   0,   5,
  10,  -5,  -5,  -5,  -5,  -5,   0,  10,
  10,   0,  -5,   0,   0,   0,   0,  10,
  20,  10,  10,   5,   5,  10,  10,  20},

{ 30,  40,  40,  50,  50,  40,  40,  30,
  30,  40,  40,  50,  50,  40,  40,  30,
  30,  40,  40,  50,  50,  40,  40,  30,
  30,  40,  40,  50,  50,  40,  40,  30,
  20,  30,  30,  40,  40,  30,  30,  20,
  10,  20,  20,  20,  20,  20,  20,  10,
 -20, -20,   0,   0,   0,   0, -20, -20,
 -20, -30, -10,   0,   0, -10, -30, -20}
 };



int values[] = {0, 100, 300, 330, 500, 900, 9000, -100, -300, -330, -500, -900, -9000};
int mat_eval = 0;
int pos_eval = 0;
int num_moves = 0;
bool white_turn = true;

unsigned long long bitboards[13] = {0ULL};
struct Move move_list[256];

unsigned long long not_black_pieces = 0ULL;
unsigned long long not_white_pieces = 0ULL;
unsigned long long all_squares = 0ULL;

unsigned long long white_pieces = 0ULL;
unsigned long long black_pieces = 0ULL;

unsigned long long empty = 0ULL;
unsigned long long occupied = 0ULL;
unsigned long long file[9] = {0ULL};
unsigned long long rank[9] = {0ULL};
unsigned long long l_diag[15] = {0ULL};
unsigned long long r_diag[15] = {0ULL};

unsigned long long square_a8 = 0ULL;

unsigned long long knight_span = 0ULL;
unsigned long long king_span = 0ULL;

unsigned long long file_ab = 0ULL;
unsigned long long file_gh = 0ULL;

//struct Move moves[256];
//int num_legal_moves = 0;

unsigned long long unsafe_white = 0ULL;
unsigned long long unsafe_black = 0ULL;

bool white_check = false;
bool black_check = false;

int num_pieces_delivering_check = 0;
unsigned long long blocking_squares = 0ULL;
unsigned long long pinning_squares[64] = {0ULL};
int en_passant_pinned = -1;

int rook_pos[] = {7, 0, 63, 56};
int rook_num_moves[] = {0, 0, 0, 0};

int king_num_moves[] = {0, 0};

int board[64] = {0};
int pieces[13][9];
int num_pieces_of_type[13] = {0};
int piece_letter_to_num[127] = {0};

char *start_position = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq"; //"rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ";

struct Move *game_possible_moves;
int num_game_moves;

unsigned long long generate_bitboard(int squares[], int num_squares){
    unsigned long long a = 0ULL;
    for(int i = 0; i < num_squares; i++){
        a |= 1ULL << squares[i];
    }
    return a;
}

unsigned long long generate_bitboard_from_range(int min_r, int max_r){
    unsigned long long a = 0ULL;
    for(int i = min_r; i <= max_r; i++){
        a |= 1ULL << i;
    }
    return a;
}

void init_bitboards(){
    board[0] = 4;
    board[1] = 2;
    board[2] = 3;
    board[3] = 6;
    board[4] = 5;
    board[5] = 3;
    board[6] = 2;
    board[7] = 4;
    for(int i = 8; i <= 15; i++){
        board[i] = 1;
    }
    for(int i = 48; i <= 55; i++){
        board[i] = board[i - 40] + 6;
    }
    for(int i = 56; i <= 63; i++){
        board[i] = board[i - 56] + 6;
    }

    for(int i = 0; i < 64; i++){
        int p = board[i];
        if(p > 0){
            pieces[p][num_pieces_of_type[p]] = i;
            num_pieces_of_type[p]++;for(int i = 0; i < 4; i++){
        printf("f %d\n", rook_pos[i]);
    }

    for(int i = 0; i < 4; i++){
        printf("g %d\n", rook_num_moves[i]);
    }

    for(int i = 0; i < 2; i++){
        printf("h %d\n", king_num_moves[i]);
    }
        }
    }

    bitboards[1] = generate_bitboard_from_range(8, 15);
    bitboards[2] = (1ULL << 1) | (1ULL << 6);
    bitboards[3] = (1ULL << 2) | (1ULL << 5);
    bitboards[4] = (1ULL << 0) | (1ULL << 7);
    bitboards[5] = (1ULL << 4);
    bitboards[6] = (1ULL << 3);

    int diff = 8 * 5;
    bitboards[7] = bitboards[1] << diff;

    diff = 8 * 7;
    for(int i = 8; i <= 12; i++){
        bitboards[i] = (bitboards[i - 6] << diff);
    }
}


void init_fen(char *fen, size_t fen_length){
    piece_letter_to_num['P'] = 1;
    piece_letter_to_num['N'] = 2;
    piece_letter_to_num['B'] = 3;
    piece_letter_to_num['R'] = 4;
    piece_letter_to_num['Q'] = 5;
    piece_letter_to_num['K'] = 6;
    piece_letter_to_num['p'] = 7;
    piece_letter_to_num['n'] = 8;
    piece_letter_to_num['b'] = 9;
    piece_letter_to_num['r'] = 10;
    piece_letter_to_num['q'] = 11;
    piece_letter_to_num['k'] = 12;
    int square = 63;
    char current = '_';
    int p = 0;
    int i = 0;
    for(; i < fen_length; i++){
        current = fen[i];
        //this is a number
        if(current >= 48 && current <= 57){
            square -= (current - 48);
        }
        else if(current == '/'){
            continue;
        }
        //found a space, done placing pieces
        else if(current == ' '){
            i++;
            break;
        }
        //placing a piece
        else{
            p = piece_letter_to_num[(int)current];
            board[square] = p;
            bitboards[p] |= (1ULL << square);
            pieces[p][num_pieces_of_type[p]] = square;
            num_pieces_of_type[p]++;
            square -= 1;
        }
    }
    current = fen[i];
    if(current == 'b'){
        white_turn = false;
    }
    else{
        white_turn = true;
    }
    i+= 2;

    for (int j = 0; j < 4; j++){
        rook_num_moves[j] = 1;
    }

    for(; i < fen_length; i++){
        current = fen[i];
        if (current == 'K'){
            rook_num_moves[1] = 0;
        }
        else if (current == 'Q'){
            rook_num_moves[0] = 0;
        }
        else if (current == 'k'){
            rook_num_moves[3] = 0;
        }
        else if (current == 'q'){
            rook_num_moves[2] = 0;
        }
        if(current == ' '){
            i++;
            break;
        }
    }
}


void append_move(struct Move* arr, struct Move m, int *i){
    arr[*i] = m;
    (*i)++;
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

unsigned long long r_shift(unsigned long long x, int n){
    if(n <= 0){
        return x << abs(n);
    }
    if(x > 0){
        return x >> n;
    }
    unsigned long long y = x >> 1;
    y &= ~square_a8;
    y = (y >> (n - 1));
    return y;
}

unsigned long long l_shift(unsigned long long x, int n){
    if(n <= 0){
        return r_shift(x, abs(n));
    }
    return x << n;
}

int get_piece(int square){
    return board[square];
}

unsigned long long reverse(unsigned long long i){
    i = ((i & 0x5555555555555555) << 1) | ((i >> 1) & 0x5555555555555555);
    i = ((i & 0x3333333333333333) << 2) | ((i >> 2) & 0x3333333333333333);
    i = ((i & 0x0f0f0f0f0f0f0f0f) << 4) | ((i >> 4) & 0x0f0f0f0f0f0f0f0f);
    i = ((i & 0x00ff00ff00ff00ff) << 8) | ((i >> 8) & 0x00ff00ff00ff00ff);
    i = (i << 48) | ((i & 0xffff0000) << 16) | ((i >> 16) & 0xffff0000) | (i >> 48);
    return i;
}

int leading_zeros(unsigned long long i){
    if(i == 0){
        return 64;
    }
    if(i < 0){
        return 0;
    }
    int n = 1;
    unsigned long long x = i >> 32;
    if(x == 0){
        n += 32;
        x = i;
    }
    if((x >> 16) == 0){
        n += 16;
        x = x << 16;
    }
    if((x >> 24) == 0){
        n += 8;
        x = x << 8;
    }
    if((x >> 28) == 0){
        n += 4;
        x = x << 4;
    }
    if((x >> 30) == 0){
        n += 2;
        x = x << 2;
    }
    n -= x >> 31;
    return n;
}

bool resolves_check(int start, int end, int move_id){
    int moved_piece = get_piece(start);
    if(white_turn){
        // trying to move a pinned piece
        if(pinning_squares[start]){
            unsigned long long pinning_line = pinning_squares[start];
            if(!((1ULL << end) & pinning_line)){
                return false;
            }
        }
        // if white pawn trying to capture en passant but it's en passant pinned
        if((moved_piece == 1) && (end == en_passant_pinned)){
            return false;
        }
        // if it's a king move
        if(moved_piece == 6){
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
                if(moved_piece != 6){
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
        if((moved_piece == 7) && (end == en_passant_pinned)){
            return false;
        }
        if(moved_piece == 12){
            if((1ULL << end) & unsafe_black){
                return false;
            }
        }
        else if(black_check){
            if(num_pieces_delivering_check > 1){
                if(moved_piece != 12){
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


void add_moves_offset(unsigned long long mask, int start_offset, int end_offset, int min_id, int max_id, struct Move* moves, int *numElems, int piece_type){
    struct Move mov;
    for(int i = 0; i < 64; i++){
        if((1ULL << i) & mask){
            for(int j = min_id; j <= max_id; j++){
                if(resolves_check(i + start_offset, i + end_offset, j)){
                    mov.start = i + start_offset;
                    mov.end = i + end_offset;
                    mov.id = j;
                    mov.piece = piece_type;
                    append_move(moves, mov, numElems);
                }
            }
        }
    }
}


void add_moves_position(unsigned long long mask, int start_position, int min_id, int max_id, struct Move* moves, int *numElems, int piece_type){
    struct Move mov;
    for(int i = 0; i < 64; i++){
        if((1ULL << i) & mask){
            for(int j = min_id; j <= max_id; j++){
                if(resolves_check(start_position, i, j)){
                    mov.start = start_position;
                    mov.end = i;
                    mov.id = j;
                    mov.piece = piece_type;
                    append_move(moves, mov, numElems);
                }
            }
        }
    }
}

unsigned long long offset_span(unsigned long long span, int origin, int i){
    if(i > origin){
        return l_shift(span, i - origin);
    }
    return r_shift(span, origin - i);
}

unsigned long long remove_span_warps(unsigned long long span, int i){
    if(i % 8 < 4){
        return span & ~file_ab;
    }
    return span & ~file_gh;
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
    unsigned long long mask = 0ULL;
    if(!((1ULL << piece_1) & direction)){
        return 0ULL;
    }
    if(!((1ULL << piece_2) & direction)){
        return 0ULL;
    }
    if(piece_1 < piece_2){
        mask = generate_bitboard_from_range(piece_1+1, piece_2-1);
    }
    else{
        mask = generate_bitboard_from_range(piece_2 + 1, piece_1-1);
    }
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

bool is_white_piece(int piece){
    return (1 <= piece) && (piece <= 6);
}

bool is_black_piece(int piece){
    return (7 <= piece) && (piece <= 12);
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

unsigned long long sliding_piece(unsigned long long mask, int i, unsigned long long blockers, bool rook_moves, bool bishop_moves, unsigned long long king_bb){
    // squares this piece threatens, as a bitboard
    unsigned long long squares = 0ULL;
    // bitboard representing this piece's location
    unsigned long long slider = 1ULL << i;
    // int 0-63 of which square the enemy king is on
    int king_square = 64 - leading_zeros(king_bb) - 1;
    // whether or not the king is white
    bool king_color = is_white_piece(get_piece(king_square));

    // array of bitboards, each representing a file/rank/diagonal this piece can move along
    unsigned long long directions[4] = { 0ULL };
    // if it moves like a rook, it can move along the rank and file it's on
    if(rook_moves){
        directions[0] = rank[get_rank(i) + 1];
        directions[1] = file[8 - get_file(i)];
    }
    // if it moves like a bishop, it can move along the left and right diagonals it's on
    if(bishop_moves){
        directions[2] = l_diag[get_l_diag(i)];
        directions[3] = r_diag[get_r_diag(i)];
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
            blocking_squares |= line_between_pieces(d, i, king_square);
            blocking_squares |= slider;
        }
        // if this function was called with the king_bb parameter (checking for pins)
        else if(king_square >= 0){
            // draw a line from this piece to the enemy king
            king_line = line_between_pieces(d, i, king_square);
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
                        struct Move mov = move_list[num_moves - 1];
                        // and the previous move was a double pawn push
                        if(mov.id == 13){
                            // we only care about this en passant pinning situation horizontally
                            // everything else is handled via regular pins
                            if(get_rank(pin_loc[0]) == get_rank(pin_loc[1])){
                                // if it's a white pawn
                                if(is_white_piece(get_piece(mov.end))){
                                    // mark that it is illegal for white to capture en passant on that square
                                    en_passant_pinned = mov.end - 8;
                                }
                                else{
                                    // mark that it is illegal for black to capture en passant on that square
                                    en_passant_pinned = mov.end + 8;
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

void possible_P(unsigned long long bb, unsigned long long can_capture, unsigned long long promo_rank, unsigned long long enemy_pawns, unsigned long long double_push_rank, int fwd, bool is_white, struct Move* moves, int *numElems){
    int e = 0;
    int m = 0;
    struct Move prev_move;
    if(num_moves > 0){
        prev_move = move_list[num_moves - 1];
        e = prev_move.end;
        m = prev_move.id;
    }

    int promo_min = 8;
    int promo_max = 11;

    if(is_white){
        promo_min = 2;
        promo_max = 5;
    }

    unsigned long long not_promo_rank = ~promo_rank;
    int piece_type = 1;
    if(!is_white){
        piece_type = 7;
    }

    // capture right
    unsigned long long mask = l_shift(bb, fwd * 8 - 1) & can_capture & not_promo_rank & ~file[1];
    add_moves_offset(mask, -(fwd * 8 - 1), 0, 0, 0, moves, numElems, piece_type);

    // capture left
    mask = l_shift(bb, fwd * 8 + 1) & can_capture & not_promo_rank & ~file[8];
    add_moves_offset(mask, -(fwd * 8 + 1), 0, 0, 0, moves, numElems, piece_type);

    // one forward
    mask = l_shift(bb, fwd * 8) & empty & not_promo_rank;
    add_moves_offset(mask, -fwd * 8, 0, 0, 0, moves, numElems, piece_type);

    // two forward
    mask = l_shift(bb, 2 * fwd * 8) & empty & l_shift(empty, fwd * 8) & double_push_rank;
    add_moves_offset(mask, -(2 * fwd * 8), 0, 0, 0, moves, numElems, piece_type);

    // promotion by capture right
    mask = l_shift(bb, fwd * 8 - 1) & can_capture & promo_rank & ~file[1];
    add_moves_offset(mask, -(fwd * 8 - 1), 0, promo_min, promo_max, moves, numElems, piece_type);

    // promotion by capture left
    mask = l_shift(bb, fwd * 8 + 1) & can_capture & promo_rank & ~file[8];
    add_moves_offset(mask, -(fwd * 8 + 1), 0, promo_min, promo_max, moves, numElems, piece_type);

    // promotion by one forward
    mask = l_shift(bb, fwd * 8) & empty & promo_rank;
    add_moves_offset(mask, -fwd * 8, 0, promo_min, promo_max, moves, numElems, piece_type);

    // if the previous move was a double pawn push, en passant might be possible
    if(m == 13){
        unsigned long long pushed_pawn_location = 1ULL << e;

        // left en passant
        mask = l_shift(bb, 1) & enemy_pawns & not_promo_rank & ~file[8] & pushed_pawn_location;
        add_moves_offset(mask, -1, fwd * 8, 0, 0, moves, numElems, piece_type);

        // right en passant
        mask = l_shift(bb, -1) & enemy_pawns & not_promo_rank & ~file[1] & pushed_pawn_location;
        add_moves_offset(mask, 1, fwd * 8, 0, 0, moves, numElems, piece_type);
    }
}

void possible_wP(unsigned long long bb, struct Move* moves, int *numElems){
    possible_P(bb, black_pieces, rank[8], bitboards[7], rank[4], 1, true, moves, numElems);
}

void possible_bP(unsigned long long bb, struct Move* moves, int *numElems){
    possible_P(bb, white_pieces, rank[1], bitboards[1], rank[5], -1, false, moves, numElems);
}

void possible_N(unsigned long long bb, unsigned long long mask, bool is_white, struct Move* moves, int *numElems){
    int p = 8;
    if(is_white){
        p = 2;
    }
    for(int i = 0; i < num_pieces_of_type[p]; i++){
        add_moves_position(span_piece(mask, pieces[p][i], knight_span, 18, 0ULL), pieces[p][i], 0, 0, moves, numElems, p);
    }
}

void possible_B(unsigned long long bb, unsigned long long mask, bool is_white, struct Move* moves, int *numElems){
    int p = 9;
    if(is_white){
        p = 3;
    }
    for(int i = 0; i < num_pieces_of_type[p]; i++){
        add_moves_position(sliding_piece(mask, pieces[p][i], occupied, false, true, 0ULL), pieces[p][i], 0, 0, moves, numElems, p);
    }
}

void possible_R(unsigned long long bb, unsigned long long mask, bool is_white, struct Move* moves, int *numElems){
    int p = 10;
    if(is_white){
        p = 4;
    }
    for(int i = 0; i < num_pieces_of_type[p]; i++){
        add_moves_position(sliding_piece(mask, pieces[p][i], occupied, true, false, 0ULL), pieces[p][i], 0, 0, moves, numElems, p);
    }
}

void possible_Q(unsigned long long bb, unsigned long long mask, bool is_white, struct Move* moves, int *numElems){
    int p = 11;
    if(is_white){
        p = 5;
    }
    for(int i = 0; i < num_pieces_of_type[p]; i++){
        add_moves_position(sliding_piece(mask, pieces[p][i], occupied, true, true, 0ULL), pieces[p][i], 0, 0, moves, numElems, p);
    }
}

void possible_K(unsigned long long bb, unsigned long long mask, bool is_white, struct Move* moves, int *numElems){
    unsigned long long squares = 0ULL;
    unsigned long long safe = ~unsafe_white;
    if(!is_white){
        safe = ~unsafe_black;
    }
    unsigned long long empty_and_safe = empty & safe;

    int p = 12;
    if(is_white){
        p = 6;
    }
    for(int i = 0; i < num_pieces_of_type[p]; i++){
        add_moves_position(span_piece((mask & safe), pieces[p][i], king_span, 9, 0ULL), pieces[p][i], 0, 0, moves, numElems, p);
    }

    // if the king is in check, king cannot castle
    if(!(bb & safe)){
        return;
    }
    // this is white king, hasn't moved yet
    if(is_white && king_num_moves[0] == 0){
        // white queenside castle
        if(get_piece(7) == 4 && rook_num_moves[0] == 0){
            squares = l_shift(bb, 2) & l_shift(empty_and_safe, 1) & empty_and_safe & l_shift(empty, -1);
            add_moves_offset(squares, -2, 0, 0, 0, moves, numElems, p);
        }
        // white kingside castle
        if(get_piece(0) == 4 && rook_num_moves[1] == 0){
            squares = l_shift(bb, -2) & l_shift(empty_and_safe, -1) & empty_and_safe;
            add_moves_offset(squares, 2, 0, 0, 0, moves, numElems, p);
        }
    }
    // this is black king, hasn't moved yet
    else if(!is_white && king_num_moves[1] == 0){
        // black queenside castle
        if(get_piece(63) == 10 && rook_num_moves[2] == 0){
            squares = l_shift(bb, 2) & l_shift(empty_and_safe, 1) & empty_and_safe & l_shift(empty, -1);
            add_moves_offset(squares, -2, 0, 0, 0, moves, numElems, p);
        }
        // black kingside castle
        if(get_piece(56) == 10 && rook_num_moves[3] == 0){
            squares = l_shift(bb, -2) & l_shift(empty_and_safe, -1) & empty_and_safe;
            add_moves_offset(squares, 2, 0, 0, 0, moves, numElems, p);
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

    unsigned long long king = bitboards[6];
    unsigned long long occupied_no_king = occupied & ~king;

    unsigned long long checking_pawn = 0ULL;

    // pawns
    // threaten to capture right
    mask = l_shift(bitboards[7], -8 - 1) & ~file[1];
    unsafe |= mask;
    // the pawn that's delivering check
    checking_pawn = r_shift((king & mask), -8 - 1);
    if(checking_pawn != 0ULL){
        num_pieces_delivering_check++;
        blocking_squares |= checking_pawn;
    }


    // threaten to capture left
    mask = l_shift(bitboards[7], -8 + 1) & ~file[8];
    unsafe |= mask;
    checking_pawn = r_shift((king & mask), -8 + 1);
    if(checking_pawn != 0ULL){
        num_pieces_delivering_check++;
        blocking_squares |= checking_pawn;
    }

    // knight
    for(int i = 0; i < num_pieces_of_type[8]; i++){
        mask = span_piece(all_squares, pieces[8][i], knight_span, 18, king);
        unsafe |= mask;
    }

    // king
    for(int i = 0; i < num_pieces_of_type[12]; i++){
        mask = span_piece(all_squares, pieces[12][i], king_span, 9, 0ULL);
        unsafe |= mask;
    }

    // queen
    for(int i = 0; i < num_pieces_of_type[11]; i++){
        mask = sliding_piece(all_squares, pieces[11][i], occupied_no_king, true, true, king);
        unsafe |= mask;
    }

    // rook
    for(int i = 0; i < num_pieces_of_type[10]; i++){
        mask = sliding_piece(all_squares, pieces[10][i], occupied_no_king, true, false, king);
        unsafe |= mask;
    }

    // bishop
    for(int i = 0; i < num_pieces_of_type[9]; i++){
        mask = sliding_piece(all_squares, pieces[9][i], occupied_no_king, false, true, king);
        unsafe |= mask;
    }

    return unsafe;
}

unsigned long long unsafe_for_black(){
    unsigned long long unsafe = 0ULL;
    unsigned long long mask = 0ULL;

    unsigned long long king = bitboards[12];
    unsigned long long occupied_no_king = occupied & ~king;

    // pawns
    // threaten to capture right
    mask = l_shift(bitboards[1], 8 - 1) & ~file[1];
    unsafe |= mask;
    blocking_squares |= r_shift((king & mask), 8 - 1);

    // threaten to capture left
    mask = l_shift(bitboards[1], 8 + 1) & ~file[8];
    unsafe |= mask;
    blocking_squares |= r_shift((king & mask), 8 + 1);

    // knight
    for(int i = 0; i < num_pieces_of_type[2]; i++){
        mask = span_piece(all_squares, pieces[2][i], knight_span, 18, king);
        unsafe |= mask;
    }

    // king
    for(int i = 0; i < num_pieces_of_type[6]; i++){
        mask = span_piece(all_squares, pieces[6][i], king_span, 9, 0ULL);
        unsafe |= mask;
    }

    // queens
    for(int i = 0; i < num_pieces_of_type[5]; i++){
        mask = sliding_piece(all_squares, pieces[5][i], occupied_no_king, true, true, king);
        unsafe |= mask;
    }

    // rook
    for(int i = 0; i < num_pieces_of_type[4]; i++){
        mask = sliding_piece(all_squares, pieces[4][i], occupied_no_king, true, false, king);
        unsafe |= mask;
    }

    // bishop
    for(int i = 0; i < num_pieces_of_type[3]; i++){
        mask = sliding_piece(all_squares, pieces[3][i], occupied_no_king, false, true, king);
        unsafe |= mask;
    }

    return unsafe;
}

bool white_in_check(){
    return (bitboards[6] & unsafe_white) > 0ULL;
}

bool black_in_check(){
    return (bitboards[12] & unsafe_black) > 0ULL;
}

void update_unsafe(){
    unsafe_white = unsafe_for_white();
    white_check = white_in_check();
    unsafe_black = unsafe_for_black();
    black_check = black_in_check();
}

void update_piece_masks(){
    white_pieces = bitboards[1] | bitboards[2] | bitboards[3] | bitboards[4] | bitboards[5];
    black_pieces = bitboards[7] | bitboards[8] | bitboards[9] | bitboards[10] | bitboards[11];
    not_white_pieces = ~(white_pieces | bitboards[6] | bitboards[12]);
    not_black_pieces = ~(black_pieces | bitboards[6] | bitboards[12]);
    empty = ~(white_pieces | black_pieces | bitboards[6] | bitboards[12]);
    occupied = ~empty;
}

void possible_moves_white(struct Move* moves, int *numElems){
    update_piece_masks();
    update_unsafe();
    (*numElems) = 0;
    possible_wP(bitboards[1], moves, numElems);
    possible_N(bitboards[2], not_white_pieces, true, moves, numElems);
    possible_B(bitboards[3], not_white_pieces, true, moves, numElems);
    possible_R(bitboards[4], not_white_pieces, true, moves, numElems);
    possible_Q(bitboards[5], not_white_pieces, true, moves, numElems);
    possible_K(bitboards[6], not_white_pieces, true, moves, numElems);
}

void possible_moves_black(struct Move* moves, int *numElems){
    update_piece_masks();
    update_unsafe();
    (*numElems) = 0;
    possible_bP(bitboards[7], moves, numElems);
    possible_N(bitboards[8], not_black_pieces, false, moves, numElems);
    possible_B(bitboards[9], not_black_pieces, false, moves, numElems);
    possible_R(bitboards[10], not_black_pieces, false, moves, numElems);
    possible_Q(bitboards[11], not_black_pieces, false, moves, numElems);
    possible_K(bitboards[12], not_black_pieces, false, moves, numElems);
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

void print_bitboard(unsigned long long bitboard){
    for(int i = 63; i >= 0; i--){
        printf(" %d ", (bitboard & (1ULL << i)) ? 1 : 0);
        if(i % 8 == 0){
            printf("\n");
        }
    }
    printf("\n");
}

void init_board(){
    //init_bitboards();
    int a = strlen(start_position);
    init_fen(start_position, a);
    init_masks();
}

bool is_legal_move(int start, int end, int promo, struct Move* moves, size_t n){
    struct Move mov;
    for(int i = 0; i < n; i++){
        mov = moves[i];
        if(mov.start == start && mov.end == end && mov.id == promo){
            return true;
        }
    }
    return false;
}

void remove_piece(int piece, int square){
    board[square] = 0;
    for(int i = 0; i < num_pieces_of_type[piece]; i++){
        if(pieces[piece][i] == square){
            int n = num_pieces_of_type[piece] - 1;
            pieces[piece][i] = pieces[piece][n];
            num_pieces_of_type[piece]--;
            break;
        }
    }
    unsigned long long remove_mask = ~(1ULL << square);
    bitboards[piece] = bitboards[piece] & remove_mask;
    mat_eval -= values[piece];
    pos_eval -= square_incentive[piece][square];
}

void add_piece(int piece, int square){
    board[square] = piece;
    pieces[piece][num_pieces_of_type[piece]] = square;
    num_pieces_of_type[piece]++;
    unsigned long long add_mask = 1ULL << square;
    bitboards[piece] = bitboards[piece] | add_mask;
    mat_eval += values[piece];
    pos_eval += square_incentive[piece][square];
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

char piece_letter(int p, bool caps){
    char letters[] = "_PNBRQKpnbrqk";
    if(caps && (p > 6)){
        p -= 6;
    }
    return letters[p];
}

void draw_board(){
    bool has_piece = false;
    for(int i = 63; i >= 0; i--){
        has_piece = false;
        for(int b = 1; b <= 12; b++){
            if(bitboards[b] & (1ULL << i)){
                printf("|%c", piece_letter(b, false));
                has_piece = true;
                break;
            }
        }
        if(!has_piece){
            printf("|_");
        }
        if(i % 8 == 0){
            printf("|\n");
        }
    }
}

bool apply_move(int start, int end, int move_id){
    int moved_piece = get_piece(start);
    //this is the ternd
    if(white_turn != is_white_piece(moved_piece)){
        printf("Not your turn %d %d %d %d\n", white_turn, start, end, move_id);
        draw_board();
        return false;
    }
    int captured_piece = get_piece(end);
    int new_s = start;
    int new_e = end;
    int new_m = move_id;
    int new_c = captured_piece;
    remove_piece(moved_piece, start);
    if(moved_piece == 4){
        for(int i = 0; i < 2; i++){
            if(start == rook_pos[i]){
                rook_pos[i] = end;
                rook_num_moves[i]++;
                break;
            }
        }
    }
    else if(moved_piece == 10){
        for(int i = 2; i < 4; i++){
            if(start == rook_pos[i]){
                rook_pos[i] = end;
                rook_num_moves[i]++;
                break;
            }
        }
    }
    //white king move
    if(moved_piece == 6){
        //kingside castling
        if(end - start == -2){
            remove_piece(4, rook_pos[1]);
            add_piece(4, 2);
            rook_pos[1] = 2;
            new_m = 15;
            // move_list.append((start, end, 15, 0))
        }
        //queenside castling
        if(end - start == 2){
            remove_piece(4, rook_pos[0]);
            add_piece(4, 4);
            rook_pos[0] = 4;
            new_m = 15;
            // move_list.append((start, end, 15, 0))
        }
        king_num_moves[0]++;
    }
    //black king move
    else if(moved_piece == 12){
        //kingside castling
        if(end - start == -2){
            remove_piece(10, rook_pos[3]);
            add_piece(10, 58);
            rook_pos[3] = 58;
            new_m = 15;
            // move_list.append((start, end, 15, 0))
        }
        //queenside castling
        if(end - start == 2){
            remove_piece(10, rook_pos[2]);
            add_piece(10, 60);
            rook_pos[2] = 60;
            new_m = 15;
            // move_list.append((start, end, 15, 0))
        }
        king_num_moves[1]++;
    }
    if(captured_piece > 0){
        remove_piece(captured_piece, end);
        //check the value of the peace
    }
    // this is for promotion
    if(move_id > 0){
        add_piece(move_id, end);
    }
    else{
        add_piece(moved_piece, end);
    }
    if((moved_piece == 1 || moved_piece == 7) && (abs(end - start) == 16)){
        // double pawn push
        new_m = 13;
        // move_list.append((start, end, 13, 0))
    }
    // if the move was castling
    else if((moved_piece == 6 || moved_piece == 12) && (abs(end - start) == 2)){
    }
    else{
        // previous move start, end, and move_id
        if(num_moves > 0){
            struct Move prev_move = move_list[num_moves - 1];
            int e = prev_move.end;
            int m = prev_move.id;
            int ep_pawn = get_piece(e);  // pawn that was captured en passant
            // white capturing en passant
            if(m == 13 && moved_piece == 1 && ep_pawn == 7 && end - e == 8){
                remove_piece(ep_pawn, e);
                new_m = 14;
            }
            // black capturing en passant
            else if(m == 13 && moved_piece == 7 && ep_pawn == 1 && end - e == -8){
                remove_piece(ep_pawn, e);
                new_m = 14;
            }
        }
    }
    struct Move mov;
    mov.start = new_s;
    mov.end = new_e;
    mov.id = new_m;
    mov.capture = new_c;
    move_list[num_moves] = mov;
    incr_num_moves();
    flip_turns();
    return true;
}

void undo_move(){
    // can't undo if nothing has been played
    if(num_moves == 0){
        return;
    }
    //previous move (the one we're undoing)
    struct Move mov = move_list[num_moves - 1];
    int start = mov.start;
    int end = mov.end;
    int move_id = mov.id;
    int capture = mov.capture;
    // the piece that was moved
    int moved_piece = get_piece(end);
    bool is_white = is_white_piece(moved_piece);
    remove_piece(moved_piece, end);
    add_piece(moved_piece, start);
    // last move was a capture
    if(capture > 0){
        add_piece(capture, end);
    }
    if(move_id == 0){
    }
    // last move was pawn promotion
    else if(1 <= move_id && move_id <= 12){
        remove_piece(move_id, start);
        if(is_white){
            add_piece(1, start);
        }
        else{
            add_piece(7, start);
        }
    }
    // last move was double pawn push
    else if(move_id == 13){
    }
    // last move was en passant
    else if(move_id == 14){
        if(is_white){
            add_piece(7, end - 8);
        }
        else{
            add_piece(1, end + 8);
        }
    }
    // last move was castling
    else if(move_id == 15){
        // white king
        if(moved_piece == 6){
            //kingside
            if(end - start == -2){
                remove_piece(4, rook_pos[1]);
                add_piece(4, 0);
                rook_pos[1] = 0;
            }
            //queenside
            if(end - start == 2){
                remove_piece(4, rook_pos[0]);
                add_piece(4, 7);
                rook_pos[0] = 7;
            }
        }
        // black king
        else if(moved_piece == 12){
            //kingside
            if(end - start == -2){
                remove_piece(10, rook_pos[3]);
                add_piece(10, 56);
                rook_pos[3] = 56;
            }
            //queenside
            if(end - start == 2){
                remove_piece(10, rook_pos[2]);
                add_piece(10, 63);
                rook_pos[2] = 63;
            }
        }
    }
    //if we're undoing a white rook move
    if(moved_piece == 4){
        for(int i = 0; i < 2; i++){
            if(end == rook_pos[i]){
                rook_pos[i] = start;
                rook_num_moves[i] -= 1;
                break;
            }
        }
    }
    // if we're undoing a black rook move
    else if(moved_piece == 10){
        for(int i = 2; i < 4; i++){
            if(end == rook_pos[i]){
                rook_pos[i] = start;
                rook_num_moves[i] -= 1;
                break;
            }
        }
    }
    // if we're undoing a white king move
    if(moved_piece == 6){
        king_num_moves[0] -= 1;
    }
    // if we're undoing a black king move
    else if(moved_piece == 12){
        king_num_moves[1] -= 1;
    }
}

void update_possible_moves(struct Move* moves, int *numElems){
    if(white_turn){
        possible_moves_white(moves, numElems);
    }
    else{
        possible_moves_black(moves, numElems);
    }
}

char file_letter(int n){
    char letter[] = "abcdefgh";
    return letter[n];
}


unsigned long long perft_test(int depth){
    if(depth == 0){
        return 1ULL;
    }

    struct Move* moves = (struct Move*)calloc(256, sizeof(struct Move));
    int numElems = 0;

    update_possible_moves(moves, &numElems);

    unsigned long long num_positions = 0ULL;
    struct Move move;

    for(int i = 0; i < numElems; i++){
        move = moves[i];
        apply_move(move.start, move.end, move.id);
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
    struct Move* moves = (struct Move*)calloc(256, sizeof(struct Move));
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
        apply_move(move.start, move.end, move.id);
        n = perft_test(depth - 1);
        num_positions += n;
        s = move.start;
        e = move.end;
        m = move.id;

        file = file_letter(7 - get_file(s));
        rank = get_rank(s) + 1;
        printf("%c%c%d", piece_letter(get_piece(s), true), file, rank);

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

void print_legal_moves(struct Move* moves, int *numElems){
    for(int i = 0; i < (*numElems); i++){
        struct Move move = moves[i];
        int s = move.start;
        int e = move.end;
        int m = move.id;
        printf("%d:\t", i);

        char file = file_letter(7 - get_file(s));
        int rank = get_rank(s) + 1;
        printf("%c%c%d", piece_letter(get_piece(s), true), file, rank);

        file = file_letter(7 - get_file(e));
        rank = get_rank(e) + 1;
        printf("%c%d\t%d\n", file, rank, m);
    }
}

// this changes a leter num into a 0 to 63 num
int nota_to_numb(char c, int i){
    int file_num = c;
    if (c >= 'A' && c <= 'H'){
        file_num += 32;
    }
    file_num -= 97;

    int square;

    square = (i-1)*8+(7-file_num);

    return square;

}

bool get_white_check(){
    return white_check;
}

bool get_black_check(){
    return black_check;
}

void update_game_possible_moves(){
    update_possible_moves(game_possible_moves, &num_game_moves);
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

int* get_board_state(){
    return board;
}

void init(){
    game_possible_moves = (struct Move*)calloc(256, sizeof(struct Move));
    num_game_moves = 0;
    init_board();
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
            start = nota_to_numb(w, x);
            end = nota_to_numb(y, z);
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

// this is going to recalulate the legale moves for a given piece
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
    struct Move* moves = (struct Move*)calloc(256, sizeof(struct Move));
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
        apply_move(move.start, move.end, move.id);
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

int search_moves_pruning(int depth, int start_depth, int alpha, int beta, bool player){
    if(depth == 0){
        return static_eval();
    }
    struct Move* moves = (struct Move*)calloc(256, sizeof(struct Move));
    int numElems = 0;

    update_possible_moves(moves, &numElems);
    struct Move move;

    if(numElems == 0){
        if(white_check){
            return INT_MIN;
        }
        else if(black_check){
            return INT_MAX;
        }
        return 0;
    }
    // this is refering to the white making a move
    if (player){
        int maxEval = INT_MIN;
        for(int i = 0; i < numElems; i++){
            move = moves[i];
            apply_move(move.start, move.end, move.id);
            int evaluation = search_moves_pruning(depth - 1, depth, alpha, beta, false);
            if(evaluation > maxEval){
                maxEval = evaluation;
                if(depth == start_depth){
                    engine_move = move;
                }
            }
            alpha = max(alpha, evaluation);
            if (beta <= alpha){
                break;
            }
            undo_move();
            decr_num_moves();
            flip_turns();
        }
        return maxEval;
    }

    else{
        int minEval = INT_MAX;
        for(int i = 0; i < numElems; i++){
            move = moves[i];
            apply_move(move.start, move.end, move.id);
            int evaluation = search_moves_pruning(depth - 1, depth, alpha, beta, true);
            if(evaluation < minEval){
                minEval = evaluation;
                if(depth == start_depth){
                    engine_move = move;
                }
            }
            beta = min(beta, evaluation);
            if (beta <= alpha){
                break;
            }
            undo_move();
            decr_num_moves();
            flip_turns();
        }
        return minEval;
    }
}

int calc_eng_move(int depth){
    return search_moves_pruning(depth, depth, INT_MIN, INT_MAX, false);

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
    return engine_move.id;
}

int* get_king_num_moves(){
    return king_num_moves;
}

int* get_rook_pos(){
    return rook_pos;
}

int* get_rook_num_moves(){
    return rook_num_moves;
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


int main(){
    init();
    //update_game_possible_moves();
    //apply_move(nota_to_numb('g', 2), nota_to_numb('g', 3), 0);
    //update_game_possible_moves();
    run_game();
    //struct Move test1 = seach_moves(5);
    //printf("%d %d %d", test1.start, test1.end, test1.eval);
    //h3_perft(7);
    //run_game();
    //printf("Perft: %llu\n", detailed_perft(7));
    //printf("%llu", perft_test(5));
    /*apply_move(nota_to_numb('g', 2), nota_to_numb('g', 3), 0);
    apply_move(nota_to_numb('h', 7), nota_to_numb('h', 6), 0);
    apply_move(nota_to_numb('f', 1), nota_to_numb('g', 2), 0);
    apply_move(nota_to_numb('h', 8), nota_to_numb('h', 7), 0);
    apply_move(nota_to_numb('g', 1), nota_to_numb('h', 3), 0);
    apply_move(nota_to_numb('h', 6), nota_to_numb('h', 5), 0);
    printf("Perft: %llu\n", detailed_perft(1));

    for(int i = 0; i < 4; i++){
        printf("f %d\n", rook_pos[i]);
    }

    for(int i = 0; i < 4; i++){
        printf("g %d\n", rook_num_moves[i]);
    }

    for(int i = 0; i < 2; i++){
        printf("h %d\n", king_num_moves[i]);
    }*/
    printf("Perft: %llu\n", detailed_perft(5));
    return 0;
}
