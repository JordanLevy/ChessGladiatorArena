#ifndef ENGINE_H_INCLUDED
#define ENGINE_H_INCLUDED

#include <stdbool.h>
#include "values.h"

int static_eval();

int search_moves(int depth, int start_depth);

int calc_static_move_eval(struct Move move, bool is_white_turn);

void swap(int *scores, struct Move *legal_moves, int val1, int val2);

int median_of_three(int *scores, struct Move *legal_moves, int low, int hi);

int partition(int *scores, struct Move *legal_moves, int low, int hi);

void quick_sort(int *scores, struct Move *legal_moves, int low, int hi);

void order_moves(struct Move *ordered, int size, bool is_white_turn);

void game_order_moves();

int search_moves_pruning(int depth, int start_depth, int alpha, int beta, bool player, struct Move *line, struct Move *best_line);

int search_moves_with_hint(int depth, int start_depth, int alpha, int beta, bool player, struct Move *line, struct Move *best_line, int *hint_line, int hint_depth, bool *applying_hint);

int test_depth_pruning(int depth, int start_depth, int alpha, int beta, bool player, struct Move *line, int *best_line, struct Move *best_line_actual_moves);

struct Move calc_eng_move(int depth);

bool move_equal(struct Move a, struct Move b);

struct Move calc_eng_move_with_test(int test_depth, int total_depth);

int max(int a, int b);

int min(int a, int b);

#endif
