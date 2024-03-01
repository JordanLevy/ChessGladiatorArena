#include "uci.h"
#include "values.h"
#include "board.h"
#include "testing.h"
#include "engine.h"
#include "piece.h"
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

//this is only called one time
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

void inputBlockers(char* input){
    int rook_pos;
    int blocker_index;
    sscanf(input, "get_blockers %d %d", &rook_pos, &blocker_index);
    unsigned long long movement_mask = get_rook_masks(rook_pos);
    unsigned long long* blockers = get_blockers_rook_single_square(movement_mask);
    printf("blockers %llu\n", blockers[blocker_index]);        
}

void inputRookLegalMoves(char* input){
    int rook_pos;
    int blocker_config;
    int index;
    printf("we are geting to the first\n");
    sscanf(input, "get_rook_legal_moves %d %d", &rook_pos, &blocker_config);
    printf("after the scanf\n");
    unsigned long long movement_mask = get_rook_masks(rook_pos);
    unsigned long long* blockers = get_blockers_rook_single_square(movement_mask);
    printf("too llu asining\n");
    index = get_index_from_magic(blockers[blocker_config], rook_magic_numbers[rook_pos], rook_magic_shift[rook_pos]);
    printf("index is done\n");
    unsigned long long rook_legal_moves = rook_moves_lookup[rook_pos][index];
    printf("rook_legal_moves %llu\n", rook_legal_moves);
}

void uci_communication(){
    char command[256];

    init(start_position, strlen(start_position));
    bool cancellationValue = false;
    bool* cancellationToken = &cancellationValue;
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
            } else if(startswith(command, "get_blockers")) {
                printf("%s\n", command);
                for(int i = 0; i < 64; i++){
                    //rook_masks[i] = get_rook_masks(i);
                    printf("%d %llu\n", i, rook_masks[i]);
                    //print_bitboard(rook_masks[i]);
                }
                inputBlockers(command);
            } else if(startswith(command, "get_rook_legal_moves")) {
                printf("%s\n", command);
                inputRookLegalMoves(command);
            } else if(startswith(command, "generate_magic")){
                printf("%s\n", command);
                

                printf("\n");
                unsigned long long* result_magic = (unsigned long long*)calloc(64, sizeof(unsigned long long));
                int* result_shift = (int*)calloc(64, sizeof(int));
                generate_rook_magic_numbers(48, 200, result_magic, result_shift, 5);
                for(int i = 0; i < 64; i++){
                    printf("%d magic number: %llu, shift: %d\n", i, result_magic[i], result_shift[i]);
                }
                write_rook_moves_lookup_to_file(result_magic, result_shift);
                free(result_magic);
                free(result_shift);
            } else if(startswith(command, "cancel_magic_search")){
                printf("%s\n", command);
                *cancellationToken = true;
            }
            else {
                printf("Invalid command.\n");
            }
            fflush(stdout);
        }
    }
    
}