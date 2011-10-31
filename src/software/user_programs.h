
#ifndef _USER_PROGRAMS_H_
#define _USER_PROGRAMS_H_

// All this programs are documented in the formal documentation in the report.

// Helps teachers to understand a bit our mess, well, no
int _printHelp(int size, char** args);

// Test the breakable code
int _test(int size, char** args);

// Just to have more functions in the autocomplete
int _ssh(int size, char** args);

// Clears the screen
int _clear(int size, char** args);

int top_main  (int argc, char ** argv);

int _hola_main (int argc, char ** argv);

int writer_main (int argc, char ** argv);

int reader_main (int argc, char ** argv);

int _fork (int argc, char ** argv);

int putc_main (int argc, char ** argv);

int getc_main (int argc, char ** argv);

int _hang (int argc, char ** argv);

int _ls (int argc, char ** argv);

int _mkdir (int argc, char ** argv);

int _touch (int argc, char ** argv);

int _cat(int argc, char ** argv);

int _fwrite(int argc, char ** argv);

int _logout(int argc, char ** argv);

int _fsstat(int argc, char ** argv);

#endif