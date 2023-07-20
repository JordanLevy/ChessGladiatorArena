#include <limits.h>
#include <stdio.h>
#include <stdbool.h>
#include "values.h"

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

#define CAPTURE_PIECE_VALUE_MULTIPLIER 10

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

Move engine_move;

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
Move move_list[256];

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

//Move moves[256];
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

Move *game_possible_moves;
int num_game_moves;