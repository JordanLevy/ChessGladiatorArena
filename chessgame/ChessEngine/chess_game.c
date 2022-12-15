#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <limits.h>
#include <time.h>
#include <math.h>
#include <ctype.h>

#define len(x)  (sizeof(x) / sizeof((x)[0]))

#define NUM_COLOR_BITS 1
#define NUM_ROLE_BITS 3
#define NUM_SPEC_BITS 4

#define SPEC_BITS_OFFSET 0
#define ROLE_BITS_OFFSET (SPEC_BITS_OFFSET + NUM_SPEC_BITS)
#define COLOR_BITS_OFFSET (ROLE_BITS_OFFSET + NUM_ROLE_BITS)

#define EMPTY_SQUARE (0 << ROLE_BITS_OFFSET)
#define PAWN (1 << ROLE_BITS_OFFSET)
#define KNIGHT (2 << ROLE_BITS_OFFSET)
#define BISHOP (3 << ROLE_BITS_OFFSET)
#define ROOK (4 << ROLE_BITS_OFFSET)
#define QUEEN (5 << ROLE_BITS_OFFSET)
#define KING (6 << ROLE_BITS_OFFSET)

#define WHITE (1 << COLOR_BITS_OFFSET)
#define BLACK (0 << COLOR_BITS_OFFSET)

#define COLOR_MASK (((int)pow(2, NUM_COLOR_BITS) - 1) << COLOR_BITS_OFFSET)
#define ROLE_MASK (((int)pow(2, NUM_ROLE_BITS) - 1) << ROLE_BITS_OFFSET)
#define SPEC_MASK (((int)pow(2, NUM_SPEC_BITS) - 1) << SPEC_BITS_OFFSET)

#define wP ((WHITE | PAWN) >> ROLE_BITS_OFFSET)
#define wN ((WHITE | KNIGHT) >> ROLE_BITS_OFFSET)
#define wB ((WHITE | BISHOP) >> ROLE_BITS_OFFSET)
#define wR ((WHITE | ROOK) >> ROLE_BITS_OFFSET)
#define wQ ((WHITE | QUEEN) >> ROLE_BITS_OFFSET)
#define wK ((WHITE | KING) >> ROLE_BITS_OFFSET)

#define bP ((BLACK | PAWN) >> ROLE_BITS_OFFSET)
#define bN ((BLACK | KNIGHT) >> ROLE_BITS_OFFSET)
#define bB ((BLACK | BISHOP) >> ROLE_BITS_OFFSET)
#define bR ((BLACK | ROOK) >> ROLE_BITS_OFFSET)
#define bQ ((BLACK | QUEEN) >> ROLE_BITS_OFFSET)
#define bK ((BLACK | KING) >> ROLE_BITS_OFFSET)

#define DOUBLE_PAWN_PUSH 16
#define EN_PASSANT_CAPTURE 17
#define CASTLING 18

/* move_ids are as follows:
0: normal move
1-15 (wP-bK): promotion to the specified type
16: Double pawn push
17: En-passant capture
18: Castling
*/

/* each piece is represented by an 8-bit piece_id (char): Color (1 bit) | Role (3 bits) | Specifier (4 bits)

Type refers to 0000 | Color | Role, which is a low 0-15 value to be used as an array index
Type = piece_id >> ROLE_BITS_OFFSET
These types are wP, wN, etc.

Color, denotes piece color:
    0=black piece
    1=white piece
Role, denotes how the piece behaves:
    0=000=empty square
    1=001=pawn
    2=010=knight
    3=011=bishop
    4=100=rook
    5=101=queen
    6=110=king
Specifier, makes the ID unique, even if Type is identical:
    Technically need 4 bits because you could have 9 queens of the same color
    Pieces that start on "lower" squares (h1 lowest, a8 highest) are given lower specifiers
    0000=0th piece of that type
    0001=1st piece of that type
    ...
e.g. white knight that started on g1 (0th white knight): 1 010 0000
e.g. empty square: 0 000 0000
e.g. black pawn that started on e7 (3rd black pawn): 0 001 0011
0=0000=empty square
1=0001=black pawn
2=0010=black knight
3=0011=black bishop
4=0100=black rook
5=0101=black queen
6=0110=black king

7=nothing
8=nothing

9 =1001=white pawn
10=1010=white knight
11=1011=white bishop
12=1100=white rook
13=1101=white queen
14=1110=white king
*/

/*
Things to optimize:
unsafe_for_white/black have to recalculate most of the same stuff
remove_piece has nested for loops (pieces array refactor?)
only recalculating the legal moves that actually are affected


store the legal moves per piece indexed by square as a board

list of 64 move list


Engine debugging tool
*/

struct Move {
    int start;
    int end;
    int move_id;
    unsigned char capture;
    unsigned char piece_id;
    int eval;
};

struct Move engine_move;

// best test alpha and beta
int best_alpha = INT_MIN;
int best_beta = INT_MAX;


// grid for deciding positional value of a move, for pos_eval
int square_incentive[15][64] =
{
//Empty
{ 0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0},

 //bP
 { 0,   0,   0,   0,   0,   0,   0,   0,
 -50, -50, -50, -50, -50, -50, -50, -50,
 -10, -10, -20, -30, -30, -20, -10, -10,
  -5,  -5, -10, -25, -25, -10,  -5,  -5,
   0,   0,   0, -20, -20,   0,   0,   0,
  -5,   5,  10,   0,   0,  10,   5,  -5,
  -5, -10, -10,  20,  20, -10, -10,  -5,
   0,   0,   0,   0,   0,   0,   0,   0},

//bN
{ 50,  40,  30,  30,  30,  30,  40,  50,
  40,  20,   0,   0,   0,   0,  20,  40,
  30,   0, -10, -15, -15, -10,   0,  30,
  30,  -5, -15, -20, -20, -15,  -5,  30,
  30,   0, -15, -20, -20, -15,   0,  30,
  30,  -5, -10, -15, -15, -10,  -5,  30,
  40,  20,   0,  -5,  -5,   0,  20,  40,
  50,  40,  30,  30,  30,  30,  40,  50},

//bB
{ 20,  10,  10,  10,  10,  10,  10,  20,
  10,   0,   0,   0,   0,   0,   0,  10,
  10,   0,  -5, -10, -10,  -5,   0,  10,
  10,  -5,  -5, -10, -10,  -5,  -5,  10,
  10,   0, -10, -10, -10, -10,   0,  10,
  10, -10, -10, -10, -10, -10, -10,  10,
  10,  -5,   0,   0,   0,   0,  -5,  10,
  20,  10,  10,  10,  10,  10,  10,  20},

//bR
{  0,   0,   0,   0,   0,   0,   0,   0,
  -5, -10, -10, -10, -10, -10, -10,  -5,
   5,   0,   0,   0,   0,   0,   0,   5,
   5,   0,   0,   0,   0,   0,   0,   5,
   5,   0,   0,   0,   0,   0,   0,   5,
   5,   0,   0,   0,   0,   0,   0,   5,
   5,   0,   0,   0,   0,   0,   0,   5,
   0,   0,   0,  -5,  -5,   0,   0,   0},

//bQ
{ 20,  10,  10,   5,   5,  10,  10,  20,
  10,   0,   0,   0,   0,   0,   0,  10,
  10,   0,  -5,  -5,  -5,  -5,   0,  10,
   5,   0,  -5,  -5,  -5,  -5,   0,   5,
   0,   0,  -5,  -5,  -5,  -5,   0,   5,
  10,  -5,  -5,  -5,  -5,  -5,   0,  10,
  10,   0,  -5,   0,   0,   0,   0,  10,
  20,  10,  10,   5,   5,  10,  10,  20},

//bK
{ 30,  40,  40,  50,  50,  40,  40,  30,
  30,  40,  40,  50,  50,  40,  40,  30,
  30,  40,  40,  50,  50,  40,  40,  30,
  30,  40,  40,  50,  50,  40,  40,  30,
  20,  30,  30,  40,  40,  30,  30,  20,
  10,  20,  20,  20,  20,  20,  20,  10,
 -20, -20,   0,   0,   0,   0, -20, -20,
 -20, -30, -10,   0,   0, -10, -30, -20},

//7. Empty
{ 0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0},

//8. Empty
{ 0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0},

//wP
{ 0,   0,   0,   0,   0,   0,   0,   0,
   5,  10,  10, -20, -20,  10,  10,   5,
   5,  -5, -10,   0,   0, -10,  -5,   5,
   0,   0,   0,  20,  20,   0,   0,   0,
   5,   5,  10,  25,  25,  10,   5,   5,
  10,  10,  20,  30,  30,  20,  10,  10,
  50,  50,  50,  50,  50,  50,  50,  50,
   0,   0,   0,   0,   0,   0,   0,   0},

//wN
{ -50, -40, -30, -30, -30, -30, -40, -50,
 -40, -20,   0,   5,   5,   0, -20, -40,
 -30,   5,  10,  15,  15,  10,   5, -30,
 -30,   0,  15,  20,  20,  15,   0, -30,
 -30,   5,  15,  20,  20,  15,   5, -30,
 -30,   0,  10,  15,  15,  10,   0, -30,
 -40, -20,   0,   0,   0,   0, -20, -40,
 -50, -40, -30, -30, -30, -30, -40, -50},

//wB
{ -20, -10, -10, -10, -10, -10, -10, -20,
 -10,   5,   0,   0,   0,   0,   5, -10,
 -10,  10,  10,  10,  10,  10,  10, -10,
 -10,   0,  10,  10,  10,  10,   0, -10,
 -10,   5,   5,  10,  10,   5,   5, -10,
 -10,   0,   5,  10,  10,   5,   0, -10,
 -10,   0,   0,   0,   0,   0,   0, -10,
 -20, -10, -10, -10, -10, -10, -10, -20},

//wR
{  0,   0,   0,   5,   5,   0,   0,   0,
  -5,   0,   0,   0,   0,   0,   0,  -5,
  -5,   0,   0,   0,   0,   0,   0,  -5,
  -5,   0,   0,   0,   0,   0,   0,  -5,
  -5,   0,   0,   0,   0,   0,   0,  -5,
  -5,   0,   0,   0,   0,   0,   0,  -5,
   5,  10,  10,  10,  10,  10,  10,   5,
   0,   0,   0,   0,   0,   0,   0,   0},

//wQ
{ -20, -10, -10,  -5,  -5, -10, -10, -20,
 -10,   0,   0,   0,   0,   5,   0, -10,
 -10,   0,   5,   5,   5,   5,   5, -10,
  -5,   0,   5,   5,   5,   5,   0,   0,
  -5,   0,   5,   5,   5,   5,   0,  -5,
 -10,   0,   5,   5,   5,   5,   0, -10,
 -10,   0,   0,   0,   0,   0,   0, -10,
 -20, -10, -10,  -5,  -5, -10, -10, -20},

//wK
{  20,  30,  10,   0,   0,  10,  30,  20,
  20,  20,   0,   0,   0,   0,  20,  20,
 -10, -20, -20, -20, -20, -20, -20, -10,
 -20, -30, -30, -40, -40, -30, -30, -20,
 -30, -40, -40, -50, -50, -40, -40, -30,
 -30, -40, -40, -50, -50, -40, -40, -30,
 -30, -40, -40, -50, -50, -40, -40, -30,
 -30, -40, -40, -50, -50, -40, -40, -30}
};

int values[15] = {0, -100, -300, -330, -500, -900, -9000, 0, 0, 100, 300, 330, 500, 900, 9000};
int mat_eval = 0;
int pos_eval = 0;
int num_moves = 0;
bool white_turn = true;

unsigned long long bitboards[15] = {0ULL};
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

// piece id of the 4 rooks you can castle with
unsigned char kingside_wR = EMPTY_SQUARE;
unsigned char queenside_wR = EMPTY_SQUARE;
unsigned char kingside_bR = EMPTY_SQUARE;
unsigned char queenside_bR = EMPTY_SQUARE;

// number of times each of the 4 castling rooks has moved
int kingside_wR_num_moves = 0;
int queenside_wR_num_moves = 0;
int kingside_bR_num_moves = 0;
int queenside_bR_num_moves = 0;

// number of times each king has moved
int wK_num_moves = 0;
int bK_num_moves = 0;

// given a square 0-63, get the piece_id of the piece on that square
unsigned char board[64] = {EMPTY_SQUARE};

// given a piece_id, get the square 0-63 that piece is on
int piece_location[256] = {-1};

// next available spec to assign to a new piece of a given type
//e.g. If I create a new white knight, what will its spec be? next_spec[wN]
int next_spec[15] = {0};

char *start_position = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq";

struct Move *game_possible_moves;
int num_game_moves;

// returns the piece residing on a square (0-63)
unsigned char get_piece(int square){
    return board[square];
}

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

void print_piece_locations(){
    for(int i = 0; i < 256; i++){
        int location = piece_location[i];
        if(location != -1){
            printf("%d %d\n", i, location);
        }
    }
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

unsigned long long generate_bitboard(int squares[], int num_squares){
    unsigned long long a = 0ULL;
    for(int i = 0; i < num_squares; i++){
        a |= 1ULL << squares[i];
    }
    return a;
}

unsigned long long generate_bitboard_from_range(int a, int b){
    unsigned long long ans = ~(0ULL);
    ans = r_shift(ans, 64 - b + a - 1) << a;
    return ans;
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

void init_fen(char *fen, size_t fen_length){
    mat_eval = 0;
    pos_eval = 0;

    int square = 63;
    char current = '_';
    int i = 0;
    int a = 36;
    for(int i = 0; i < 256; i++){
        piece_location[i] = -1;
    }
    for(; i < fen_length; i++){
        current = fen[i];
        //found a number, representing empty squares
        if(current >= '0' && current <= '9'){
            square -= (current - '0');
        }
        // found a slash, next row
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
    current = fen[i];
    if(current == 'b'){
        white_turn = false;
    }
    else{
        white_turn = true;
    }
    i+= 2;

    kingside_wR_num_moves = 1;
    queenside_wR_num_moves = 1;
    kingside_bR_num_moves = 1;
    queenside_bR_num_moves = 1;

    for(; i < fen_length; i++){
        current = fen[i];
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

void print_bitboard(unsigned long long bitboard){
    for(int i = 63; i >= 0; i--){
        printf(" %d ", (bitboard & (1ULL << i)) ? 1 : 0);
        if(i % 8 == 0){
            printf("\n");
        }
    }
    printf("\n");
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
                        struct Move move = move_list[num_moves - 1];
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

void possible_P(unsigned long long bb, unsigned long long can_capture, unsigned long long promo_rank, unsigned long long enemy_pawns, unsigned long long double_push_rank, int fwd, unsigned char color, struct Move* moves, int *numElems){
    bool is_white = (color == WHITE);
    int e = 0;
    int m = 0;
    struct Move prev_move;
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
    add_moves_offset(mask, -(fwd * 8 - 1), 0, 0, 0, moves, numElems);

    // capture left
    mask = l_shift(bb, fwd * 8 + 1) & can_capture & not_promo_rank & ~file[8];
    add_moves_offset(mask, -(fwd * 8 + 1), 0, 0, 0, moves, numElems);

    // one forward
    mask = l_shift(bb, fwd * 8) & empty & not_promo_rank;
    add_moves_offset(mask, -fwd * 8, 0, 0, 0, moves, numElems);

    // two forward
    mask = l_shift(bb, 2 * fwd * 8) & empty & l_shift(empty, fwd * 8) & double_push_rank;
    add_moves_offset(mask, -(2 * fwd * 8), 0, 0, 0, moves, numElems);

    // promotion by capture right
    mask = l_shift(bb, fwd * 8 - 1) & can_capture & promo_rank & ~file[1];
    add_moves_offset(mask, -(fwd * 8 - 1), 0, promo_min, promo_max, moves, numElems);

    // promotion by capture left
    mask = l_shift(bb, fwd * 8 + 1) & can_capture & promo_rank & ~file[8];
    add_moves_offset(mask, -(fwd * 8 + 1), 0, promo_min, promo_max, moves, numElems);

    // promotion by one forward
    mask = l_shift(bb, fwd * 8) & empty & promo_rank;
    add_moves_offset(mask, -fwd * 8, 0, promo_min, promo_max, moves, numElems);

    // if the previous move was a double pawn push, en passant might be possible
    if(m == DOUBLE_PAWN_PUSH){
        unsigned long long pushed_pawn_location = 1ULL << e;

        // left en passant
        mask = l_shift(bb, 1) & enemy_pawns & not_promo_rank & ~file[8] & pushed_pawn_location;
        add_moves_offset(mask, -1, fwd * 8, 0, 0, moves, numElems);

        // right en passant
        mask = l_shift(bb, -1) & enemy_pawns & not_promo_rank & ~file[1] & pushed_pawn_location;
        add_moves_offset(mask, 1, fwd * 8, 0, 0, moves, numElems);
    }
}

void possible_wP(unsigned long long bb, struct Move* moves, int *numElems){
    possible_P(bb, black_pieces, rank[8], bitboards[bP], rank[4], 1, WHITE, moves, numElems);
}

void possible_bP(unsigned long long bb, struct Move* moves, int *numElems){
    possible_P(bb, white_pieces, rank[1], bitboards[wP], rank[5], -1, BLACK, moves, numElems);
}

void possible_N(unsigned long long bb, unsigned long long mask, unsigned char color, struct Move* moves, int *numElems){
    unsigned char type = get_type(color | KNIGHT);
    for(int i = 0; i < next_spec[type]; i++){
        unsigned char id = color | KNIGHT | i;
        int location = piece_location[id];
        if(location == -1) continue;
        add_moves_position(span_piece(mask, location, knight_span, 18, 0ULL), location, 0, 0, moves, numElems);
    }
}

void possible_B(unsigned long long bb, unsigned long long mask, unsigned char color, struct Move* moves, int *numElems){
    unsigned char type = get_type(color | BISHOP);
    for(int i = 0; i < next_spec[type]; i++){
        unsigned char id = color | BISHOP | i;
        int location = piece_location[id];
        if(location == -1) continue;
        add_moves_position(sliding_piece(mask, location, occupied, false, true, 0ULL), location, 0, 0, moves, numElems);
    }
}

void possible_R(unsigned long long bb, unsigned long long mask, unsigned char color, struct Move* moves, int *numElems){
    unsigned char type = get_type(color | ROOK);
    for(int i = 0; i < next_spec[type]; i++){
        unsigned char id = color | ROOK | i;
        int location = piece_location[id];
        if(location == -1) continue;
        add_moves_position(sliding_piece(mask, location, occupied, true, false, 0ULL), location, 0, 0, moves, numElems);
    }
}

void possible_Q(unsigned long long bb, unsigned long long mask, unsigned char color, struct Move* moves, int *numElems){
    unsigned char type = get_type(color | QUEEN);
    for(int i = 0; i < next_spec[type]; i++){
        unsigned char id = color | QUEEN | i;
        int location = piece_location[id];
        if(location == -1) continue;
        add_moves_position(sliding_piece(mask, location, occupied, true, true, 0ULL), location, 0, 0, moves, numElems);
    }
}

void possible_K(unsigned long long bb, unsigned long long mask, unsigned char color, struct Move* moves, int *numElems){
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
        add_moves_position(span_piece((mask & safe), location, king_span, 9, 0ULL), location, 0, 0, moves, numElems);
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
            add_moves_offset(squares, -2, 0, 0, 0, moves, numElems);
        }
        // white kingside castle
        if(piece_location[kingside_wR] == 0 && kingside_wR_num_moves == 0){
            squares = l_shift(bb, -2) & l_shift(empty_and_safe, -1) & empty_and_safe;
            add_moves_offset(squares, 2, 0, 0, 0, moves, numElems);
        }
    }
    // this is black king, hasn't moved yet
    else if(!is_white && bK_num_moves == 0){
        // black queenside castle
        if(piece_location[queenside_bR] == 63 && queenside_bR_num_moves == 0){
            squares = l_shift(bb, 2) & l_shift(empty_and_safe, 1) & empty_and_safe & l_shift(empty, -1);
            add_moves_offset(squares, -2, 0, 0, 0, moves, numElems);
        }
        // black kingside castle
        if(piece_location[kingside_bR] == 56 && kingside_bR_num_moves == 0){
            squares = l_shift(bb, -2) & l_shift(empty_and_safe, -1) & empty_and_safe;
            add_moves_offset(squares, 2, 0, 0, 0, moves, numElems);
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
        printf("Not your turn\nwhite_turn:%d\nstart:%d\nend:%d\nmove_id:%d\n", white_turn, start, end, move_id);
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

// changes a letter num into a 0 to 63 num
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


// this is what does the pruning
int search_moves_pruning(int depth, int start_depth, int alpha, int beta, bool player, struct Move* line, struct Move* best_line){
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
            int evaluation = search_moves_pruning(depth - 1, depth, alpha, beta, false, line, best_line);
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
            int evaluation = search_moves_pruning(depth - 1, depth, alpha, beta, true, line, best_line);
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

int calc_eng_move(int depth){
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

    return eval;
}

bool move_equal(struct Move a, struct Move b){
    if(a.move_id != b.move_id) return false;
    if(a.capture != b.capture) return false;
    if(a.start != b.start) return false;
    if(a.end != b.end) return false;
    if(a.piece_id != b.piece_id) return false;
    return true;
}

int calc_eng_move_with_test(int test_depth, int total_depth){
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

    print_line(best_test_line_actual, test_depth);

    bool applying_hint = true;

    int eval = search_moves_with_hint(total_depth, total_depth, INT_MIN, INT_MAX, false, line, best_final_line, best_test_line, test_depth, &applying_hint);

    // find the best line and play the first move
    engine_move = best_final_line[total_depth];

    return eval;
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

int main(){
    printf("%d %d %d\n", NUM_COLOR_BITS, NUM_ROLE_BITS, NUM_SPEC_BITS);
    printf("%d %d %d\n", COLOR_BITS_OFFSET, ROLE_BITS_OFFSET, SPEC_BITS_OFFSET);
    printf("%d %d %d %d %d %d %d\n", EMPTY_SQUARE, PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING);
    printf("%d %d\n", WHITE, BLACK);
    printf("%d %d %d\n", COLOR_MASK, ROLE_MASK, SPEC_MASK);

    printf("\n\n");
    char* fen = start_position;
    init(fen, strlen(fen));
    //run_game();
    //printf("Perft: %llu\n", perft_test(6));*/


    return 0;
}
