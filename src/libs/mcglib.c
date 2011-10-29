#include "mcglib.h"
#include "../kernel/video.h"

// Splits an string, returns the length of the splitted array
// C: char array.
// ch: separator.
// len: length of the string.
char** split_string(char* c, char ch, int* len) {
	char** result = (char**) malloc(sizeof(char*) * 5);
	int result_s = 5;
	*len = 0;
	int i = 0;
	int start = i;
	char* currentbuilder = 0;
	while (*(c + i) != 0) {
		if (*len >= result_s - 1) {
			result = (char**) realloc(result, sizeof(char*) * (result_s + 5), sizeof(char*) * (*len));
			result_s = *len + 5;
		}
		if (*(c + i) == ch) {
			currentbuilder = (char*) malloc(sizeof(char) * (i - start + 2));
			char cur = *(c + i);
			*(c + i) = 0;
			currentbuilder = (char*) strcpy(currentbuilder, c + start);
			start = i + 1;
			*(c + i) = cur;
			result[*len] = currentbuilder;
			(*len)++;
		}
		i++;
	}
	if (start != i) {
		currentbuilder = (char*) malloc(sizeof(char) * (i - start + 1));
		char cur = *(c + i);
		*(c + i) = 0;
		currentbuilder = (char*) strcpy(currentbuilder, c + start);
		start = i + 1;
		*(c + i) = cur;
		result[*len] = currentbuilder;
		(*len)++;
	}
	return result;
}

char * file_in_pwd(char * pwd) {
	int i = 0;
	int len = strlen(pwd);
	int last_slash = 0;
	for(i = 0; i < len; ++i)
	{
		if(pwd[i] == '/') {
			last_slash = i;
		}
	}
	return pwd + last_slash + 1;
}

int string_ends_with(char * str, char c) { 
	int i = 0;
	// I dont give a f...
	return str[strlen(str) - 2] == c;
}