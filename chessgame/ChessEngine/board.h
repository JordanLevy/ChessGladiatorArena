#ifndef BOARD_H_INCLUDED
#define BOARD_H_INCLUDED

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

void append_move(struct Move *arr, struct Move m, int *i);

void init_fen(char *fen, size_t fen_length);

int get_file(int n);

int get_rank(int n);

int get_r_diag(int n);

int get_l_diag(int n);

bool resolves_check(int start, int end, int move_id);

void add_moves_offset(unsigned long long mask, int start_offset, int end_offset, int min_id, int max_id, struct Move *moves, int *numElems);

void add_moves_position(unsigned long long mask, int start_position, int min_id, int max_id, struct Move *moves, int *numElems);

unsigned long long unsafe_for_white();

unsigned long long unsafe_for_black();

bool white_in_check();

bool black_in_check();

#endif