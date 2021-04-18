#ifndef UTILS_H
#define UTILS_H

#include "defs.h"

char* split_line(char* buf, char splitter);

int block_contains(const char* buf, char c);

bool strings_are_equal(char* str1, char* str2);

void empty_buffer(char* buffer);

bool string_is_empty(const char* str);

#endif // UTILS_H
