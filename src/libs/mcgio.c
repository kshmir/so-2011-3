#include "../../include/defs.h"
#include "mcgio.h"
#include "stdio.h"
#include <stdarg.h>

// Callback for tabs
char* (*onTabCall)(char*) = NULL;

void setTabCall(char* (*ptr)(char*)) {
	onTabCall = ptr;
}

// Callback for arrows
char* (*onArrowHit)(int) = NULL;

void setArrowHit(char* (*ptr)(int)) {
	onArrowHit = ptr;
}

// Prints a string
void printString(char* c) {
	int i = 0;
	while (c[i] != 0)
		putchar(c[i++]);
}

// Internal, non standard putchar
void mcg_putchar(char c) {
	if (c == '\r') {
		backSpace();
	} else if (c == '\n') {
		newLine();
	} else if (c == 0x0f) {
		return;
	} else if (c != 0) {
		putChar(c);
		putC(c);
	}
}

// Console scanf
char* getConsoleString(int showdata) {
	char c;
	char* str = NULL;
	str = (char*) malloc(sizeof(char) * 255 + 1); // TODO: This should increment on hit
	int strlen = 255;
	int i      = 0;
	int sx     = getCursorX();
	int sy     = getCursorY();


	while ((c = getC()) != '\n') {
		int dirKey = 0;//getA();

		if (i == strlen && c != '\r') {
			continue;
		}
		
		

		if (dirKey == 0) {
			if (c != 0x0f && c != 0) {
				if ((c != '\r' || getCursorY() > sy || getCursorX() > sx) && showdata)
					mcg_putchar(c);
				if (c != '\r') {
					str[i] = c;
					i++;
				} else {
					if (i > 0) {
						str[i] = 0;
						i--;
					}
				}
			}
		} else {
			char *new_line = NULL;
			if (dirKey == 8)
				new_line = onArrowHit(1);
			else
				new_line = onArrowHit(-1);
			if (new_line != NULL) {
				while (i > 0) {
					str[i + 1] = 0;
					i--;
					mcg_putchar('\r');
				}
				str[i] = 0;
				while (*new_line != 0) {
					str[i] = *new_line;
					mcg_putchar(*new_line);
					i++;
					*new_line++;
				}
			}
		}
	}
	mcg_putchar(c);
	clear_screen_topdown();
	str[i] = 0;
	return str;
}

// Another printstring
void printstring(char* message) {
	int i = 0;
	while (message[i] != '\0') {
		putchar(message[i]);
		i++;
	}
}

// Builds an int
int getint(char* mensaje, ...) {
	int n = 0, salir = 0;
	va_list ap;

	do {
		va_start(ap, mensaje);
		vprintf(mensaje, ap);
		va_end(ap);
		if (scanf("%d", &n) != 1) {
			BORRA_BUFFER;
			printf("\nInvalid Value, please Try again\n");
		} else {
			BORRA_BUFFER;
			salir = 1;
		}
	} while (!salir);
	return n;
}

// Prints a double
void printdouble(double number, char* format) {
	char chardouble[40];
	ftoa(number, chardouble);
	int i = 0;
	while (chardouble[i] != '\0') {
		putchar(chardouble[i]);
		i++;
	}
}

// Prints an int
void printint(int number, char* format) {
	char charint[40];
	itoa(number, charint);
	int i = 0;
	while (charint[i] != '\0')
		putchar(charint[i++]);
}

// For internal use on some prints
void internalswap(char* answ, int pos) {
	int correccion = 0;
	int i = 0;
	correccion += pos % 2;
	while (i < (pos + correccion) / 2) {
		char aux = answ[i];
		answ[i] = answ[pos - i];
		answ[pos - i] = aux;
		i++;
	}
}

// Printf with stdargs
int mcg_printf(char* string, ...) {
	int i = 0, c = 0, va_count;
	va_list ap, bp;
	va_start(ap, string);
	while (string[i] != '\0') {
		if (string[i] == '\n')
			c++;
		else if (string[i] == '%') {
			i++;
			switch (string[i]) {
			case 's':
				c += entersOnString(va_arg(ap,char*));
				break;
			case 'c':
				c += (va_arg( ap, int)) ? 1 : 0;
				break;
			}
		}
		i++;
	}
	va_start(bp, string);
	vprintf(string, bp);
	va_end(ap);
	return c;
}

// Detects enters on string
int entersOnString(char* str) {
	int i = 0, c = 0;
	for (i = 0; str[i] != 0; ++i)
		c += (str[i] == '\n') ? 1 : 0;
	return c;
}

