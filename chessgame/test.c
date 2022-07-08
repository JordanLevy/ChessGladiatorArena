#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

int board[64] = {0};

void hello_world(){
    printf("hello world\n");
}

int num_test(){
    return 5;
}

bool is_even(int a){
    return ((a % 2) == 0);
}

int* get_list(){
    board[3]++;
    return board;
}