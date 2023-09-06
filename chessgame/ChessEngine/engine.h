#ifndef ENGINE_H_INCLUDED
#define ENGINE_H_INCLUDED

#include <stdbool.h>
#include "values.h"

int static_eval();

int calc_static_move_eval(Move move, bool is_white_turn);

void swap(int *scores, Move *legal_moves, int val1, int val2);

int median_of_three(int *scores, Move *legal_moves, int low, int hi);

int partition(int *scores, Move *legal_moves, int low, int hi);

void quick_sort(int *scores, Move *legal_moves, int low, int hi);

void order_moves(Move *ordered, int size, bool is_white_turn);

void game_order_moves();

int search_moves_transposition(int depth, int start_depth, int alpha, int beta, bool player, Move *line, Move *best_line);

int search_moves_captures(int alpha, int beta, bool player);

Move calc_eng_move(int depth);

bool move_equal(Move a, Move b);

int max_val(int a, int b);

int min_val(int a, int b);

#endif
