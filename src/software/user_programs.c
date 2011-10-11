#include "user_programs.h"

// Helps teachers to understand a bit our mess, well, no
int _printHelp(int size, char** args) {
	printf("MonkeyOS 1 - MurcielagOS kernel v0.1 (i686-pc-murcielago)\n");
	printf("These shell commands are defined internally.  Type `help' to see this list.\n");
}

// Test the breakable code
int _test(int size, char** args) {

	printf("Welcome to the test programme\n\n");
	printf("We are gonna test the following code:\n");

	char name[200];
	char surname[200];
	int age = 0;
	double height = 2.0;

	printf("char name[200];\nchar surname[200];\nint age = 0;\n");
	printf("double height = 0.0;\nprintf(\"please enter your name:\");\n");
	printf("scanf(\"%%s\",name);\n");
	printf("printf(\"please enter you age:\");\n");
	printf("scanf(\"%%d\",&age);\n");
	printf("printf(\"please enter you height:\");\n");
	printf("scanf(\"%%d\",&height);\n");
	printf(
			"printf(\"my name is:%%s i am %%d years old and %%d feet tall\\n\",name,age,height);\n");
	printf(
			"printf(\"please enter your name surname \\nfor example Bruce Wayne:\");\n");
	printf("scanf(\"%%s %%s\",name,surname);\n");
	printf(
			"printf(\"my name is %%s and my surname is %%s\\n\",name,surname);\n");

	printf("\nPress ENTER to begin the test");
	getchar();
	printf("\n\nplease enter your name:");
	scanf("%s", name);
	printf("please enter you age:");
	scanf("%d", &age);
	printf("please enter you height:");
	scanf("%f", &height);
	printf("my name is:%s i am %d years old and %f feet tall\n", name, age, height);
	printf("please enter your name surname \nfor example Bruce Wayne:");
	scanf("%s %s", name, surname);
	printf("my name is %s and my surname is %s\n", name, surname);
}

// Just to have more functions in the autocomplete
int _ssh(int size, char** args) {
	printf("Attempting to connect...\n");
	printf("Oooops we forgot our internals don't have any TCP/IP backend...\n");
	printf("You'll have to wait for MurcielagOS 2.0 or maybe 11.0 to see this working\n");
	printf("Take a seat and wait!!!\n");
	printf("...\n");
	printf("...\n");
	printf("...\n");
	printf("Our autocomplete looked just empty so we made this :)\n");
}

// Clears the screen
int _clear(int size, char** args) {
	clear_screen();
}


int _hola_main (int argc, char ** argv)
{
	int i = 0;
	for(; i < 100; ++i) {
		printf("HOLA :) %d\n",i);
	}

	return 0;
}

int writer_main (int argc, char ** argv)
{
	if(argc > 1)
	{
		int fifo = mkfifo("teta", 0666);
		int i = 1;
		for(; i < argc; ++i)
		{
			printf("param %d %s\n", i, argv[i]);
			write(fifo, argv[i], strlen(argv[i]));
		}

	} else { 
		printf("Not enough params\n");
	}

	return 0;
}

int reader_main (int argc, char ** argv) {
	int fifo = mkfifo("teta", 0666);

	
	char buff[1024];
	int len = read(fifo, buff, 1024);	
	int i = 0;
	printf("I READ RAW:");
	for(;i < len; i++)	{
		printf("%c", buff[i], i);
	}
	printf("\n");
	return 0;
}


int putc_main (int argc, char ** argv) {
	if(argc > 1) {
		printf("%s\n", argv[1]);
	}

	
	return 0;
}

int getc_main (int argc, char ** argv) {	
	int c;
	while((c = getC()) != '\n') {
		printf("%c", c);
	}
	printf("\n");
	
	return 0;
}


int _fork (int argc, char ** argv)
{

	return 0;
}