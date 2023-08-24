#ifndef BOARD_H_INCLUDED
#define BOARD_H_INCLUDED

#define CAN_CASTLE_WK 1
#define CAN_CASTLE_WQ 2
#define CAN_CASTLE_BK 4
#define CAN_CASTLE_BQ 8

unsigned char get_piece(int square);

void draw_board();

void remove_piece(unsigned char id, int square);

void destroy_piece(unsigned char id, int square);

unsigned char add_piece(unsigned char id, int square);

void revive_piece(unsigned char id, int square);

void move_piece(unsigned char id, int start, int end);

void init_masks();

void reset_board();

int notation_to_number(char c, int i);

void append_move(Move *arr, Move m, int *i);

void init_fen(char *fen, size_t fen_length);

int get_file(int n);

int get_rank(int n);

int get_r_diag(int n);

int get_l_diag(int n);

bool resolves_check(int start, int end, int move_id);

void add_moves_offset(unsigned long long mask, int start_offset, int end_offset, int min_id, int max_id, Move *moves, int *numElems);

void add_moves_position(unsigned long long mask, int start_position, int min_id, int max_id, Move *moves, int *numElems);

unsigned long long unsafe_for_white();

unsigned long long unsafe_for_black();

bool white_in_check();

bool black_in_check();

void update_unsafe();

bool white_in_checkmate(int numElems);

bool black_in_checkmate(int numElems);

void init_board(char *fen, size_t len);

bool is_legal_move(int start, int end, int promo, Move *moves, size_t n);

void incr_num_moves();

void decr_num_moves();

void flip_turns();

bool apply_move(int start, int end, int move_id);

void undo_move();

bool get_white_check();

bool get_black_check();

bool try_undo_move();

bool is_game_legal_move(int start, int end, int promo);

char piece_letter(int piece_id, bool caps);

char file_letter(int n);

void init(char *fen, int len);

#endif