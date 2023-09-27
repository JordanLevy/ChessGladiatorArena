#include "uci.h"
#include "values.h"
#include "board.h"
#include "testing.h"
#include "engine.h"
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

char* move_to_string(Move move){
    char* result = (char*)malloc(sizeof(char) * 5);
    int s = move.start;

    int e = move.end;
    int m = move.move_id;

    char promotion = '\0';

    char start_file = file_letter(7 - get_file(s));
    int start_rank = get_rank(s) + 1;

    char end_file = file_letter(7 - get_file(e));
    int end_rank = get_rank(e) + 1;

    if(m == wN || m == bN){
        promotion = 'n';
    }
    else if(m == wB || m == bB){
        promotion = 'b';
    }
    else if(m == wR || m == bR){
        promotion = 'r';
    }
    else if(m == wQ || m == bQ){
        promotion = 'q';
    }

    if(move.start == -1){
        free(result);
        return "(none)";
    }

    snprintf(result, 6, "%c%d%c%d%c", start_file, start_rank, end_file, end_rank, promotion);

    return result;
}

bool str_equals(const char* a, const char* b){
    return strcmp(a, b) == 0;
}

bool startswith(const char* str, const char* prefix) {
    size_t len_str = strlen(str);
    size_t len_prefix = strlen(prefix);
    if (len_prefix > len_str) {
        return 0;
    }
    return strncmp(str, prefix, len_prefix) == 0;
}

char* substring(char* str, int start, int end) {
    int i;
    int j = 0;
    char* sub = (char*)malloc(sizeof(char) * (end - start + 1));
    for (i = start; i <= end; i++) {
        sub[j] = str[i];
        j++;
    }
    sub[j] = '\0';
    return sub;
}

void inputUCI(){
    printf("id name Odin\n");
    printf("id author Ryan Johnson and Jordan Levy\n");
    printf("uciok\n");
}

void inputSetOption(){
    printf("setOption working\n");
}

void inputIsReady(){
    printf("readyok\n");
}

void inputUCINewGame(){
    init(start_position, strlen(start_position));
}

void inputPosition(char* input){

    char* cmd = input;
    cmd += 9;
    strcat(cmd, " ");
    if(startswith(cmd, "startpos ")){
        cmd += 9;
        init(start_position, strlen(start_position));
    }
    else if(startswith(cmd, "fen ")){
        cmd += 4;
        init(cmd, strlen(cmd));
    }
    if(startswith(cmd, "moves ")){
        cmd += 6;
        //TODO make moves accordingly
    }
}

void inputGo(char* input){
    char* cmd = input;
    cmd += 3;
    if(startswith(cmd, "depth ")){
        cmd += 6;
        int depth = atoi(cmd);
        Move result = calc_eng_move(depth);
        char* move_string = move_to_string(result);
        printf("bestmove %s\n", move_string);
    }
    else if(startswith(cmd, "perft ")){
        cmd += 6;
        int depth = atoi(cmd);
        unsigned long long a = detailed_perft(depth);
        printf("perft %llu\n", a);
    }
}

void uci_communication(){
    char command[256];

    init(start_position, strlen(start_position));
    if(uci_enabled){
        while (fgets(command, sizeof(command), stdin)) {
            // remove newline character from the command
            //printf("info received command%s\n", command);
            command[strcspn(command, "\n")] = 0;

            if(str_equals(command, "uci")) {
                inputUCI();
            } else if(startswith(command, "setoption")) {
                inputSetOption();
            } else if(str_equals(command, "isready")) {
                inputIsReady();
            } else if(str_equals(command, "ucinewgame")) {
                inputUCINewGame();
            } else if(startswith(command, "position")) {
                printf("%s\n", command);
                inputPosition(command);
            } else if(startswith(command, "go")) {
                printf("%s\n", command);
                inputGo(command);
            } else if(startswith(command, "quit")) {
                break;
            } else {
                printf("Invalid command.\n");
            }
            fflush(stdout);
        }
    }
    
}