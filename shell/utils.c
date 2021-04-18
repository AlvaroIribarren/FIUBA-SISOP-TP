#include "utils.h"

// splits a string line in two
// acording to the splitter character
char* split_line(char* buf, char splitter) {

	int i = 0;

	while (buf[i] != splitter &&
		buf[i] != END_STRING)
		i++;
		
	buf[i++] = END_STRING;
	
	while (buf[i] == SPACE)
		i++;
	
	return &buf[i];
}

// looks in a block for the 'c' character
// and returns the index in which it is, or -1
// in other case
int block_contains(const char* buf, char c) {
	for (size_t i = 0; i < strlen(buf); i++)
		if (buf[i] == c)
			return i;
	
	return -1;
}

//returns true if the strings are equal
bool strings_are_equal(char* str1, char* str2){
    return (strcmp(str1, str2) == 0);
}

void empty_buffer(char* buf){
	for (unsigned int i=0; i<strlen(buf); i++){
		buf[i] = '\0';
	}
}

bool string_is_empty(const char *str) {
	return strlen(str) == 0;
}

