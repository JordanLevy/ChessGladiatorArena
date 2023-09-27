#ifndef VALUES_H_INCLUDED
#define VALUES_H_INCLUDED

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

#define ALL 0

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

typedef struct Move {
    int start;
    int end;
    int move_id;
    unsigned char capture;
    unsigned char piece_id;
    int eval;
} Move;

typedef struct MoveList{
    Move* moves;
    int size;
} MoveList;

extern Move engine_move;

// best test alpha and beta
extern int best_alpha;
extern int best_beta;


// grid for deciding positional value of a move, for pos_eval
extern int square_incentive[15][64];

extern int values[15];
extern int mat_eval;
extern int pos_eval;
extern int castling_rights;
extern int num_moves;
extern bool white_turn;

extern unsigned long long bitboards[15];
extern Move move_list[256];

extern unsigned long long not_black_pieces;
extern unsigned long long not_white_pieces;
extern unsigned long long all_squares;

extern unsigned long long white_pieces;
extern unsigned long long black_pieces;

extern unsigned long long empty;
extern unsigned long long occupied;
extern unsigned long long file[9];
extern unsigned long long rank[9];
extern unsigned long long l_diag[15];
extern unsigned long long r_diag[15];

extern unsigned long long square_a8;

extern unsigned long long knight_span;
extern unsigned long long king_span;

extern unsigned long long file_ab;
extern unsigned long long file_gh;

//Move moves[256];
//int num_legal_moves;

extern unsigned long long unsafe_white;
extern unsigned long long unsafe_black;

extern bool white_check;
extern bool black_check;

extern int num_pieces_delivering_check;
extern unsigned long long blocking_squares;
extern unsigned long long pinning_squares[64];
extern int en_passant_pinned;

// piece id of the 4 rooks you can castle with
extern unsigned char kingside_wR;
extern unsigned char queenside_wR;
extern unsigned char kingside_bR;
extern unsigned char queenside_bR;

// number of times each of the 4 castling rooks has moved
extern int kingside_wR_num_moves;
extern int queenside_wR_num_moves;
extern int kingside_bR_num_moves;
extern int queenside_bR_num_moves;

// number of times each king has moved
extern int wK_num_moves;
extern int bK_num_moves;

// given a square 0-63, get the piece_id of the piece on that square
extern unsigned char board[64];

// given a piece_id, get the square 0-63 that piece is on
extern int piece_location[256];

// next available spec to assign to a new piece of a given type
//e.g. If I create a new white knight, what will its spec be? next_spec[wN]
extern int next_spec[15];

extern char *start_position;

//this is for UCI COMAND TO TERN ON AND OFF THE COMUNICATION
extern bool uci_enabled;




#endif