#ifndef UCI_H_INCLUDED
#define UCI_H_INCLUDED

#include <stdbool.h>
#include "values.h"

char *move_to_string(struct Move move);

bool str_equals(const char *a, const char *b);

bool startswith(const char *str, const char *prefix);

char *substring(char *str, int start, int end);

void inputUCI();

void inputSetOption();

void inputIsReady();

void inputUCINewGame();

void inputPosition(char *input);

void inputGo(char *input);

void uci_communication();

#endif


