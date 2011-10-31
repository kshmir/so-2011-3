#include "user_programs.h"
#include "../monix/monix.h"
#include "../libs/pqueue.h"
#include "../libs/mcglib.h"
#include "../drivers/atadisk.h"
#include "../kernel/fs/fs.h"
#include "../libs/heap.h"

typedef struct top_data {
	int pid;
	int ticks;
} top_data;

int proc_comparer(top_data * t1, top_data * t2) {
	return t2->ticks - t1->ticks;
}

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
		int fifo = mkfifo("fifo", O_NEW);
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
	int fifo = mkfifo("fifo", O_NEW);

	
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
		printf("%s", argv[1]);
	}

	return 0;
}


int top_main  (int argc, char ** argv) {

	char c;
	heap h = heap_init(1024, (void *) proc_comparer);
	do {
		int allow_zombies = 0;

		if (argc > 1 && !strcmp(argv[1], "zombies")) {
			allow_zombies = 1;
		}

		int len = 0;
		int i = 0;
		int active_pids[PROCESS_MAX];
		int	active_pids_ticks[PROCESS_MAX];
		int active_pids_n = 0;

		for(; i < PROCESS_MAX; ++i)
		{
			active_pids[i] = -1;
			active_pids_ticks[i] = 0;
		}

		int tick_history[PROCESS_HISTORY_SIZE];
		int * _pticks = pticks();

		i = 0;

		for(; i < PROCESS_HISTORY_SIZE; i++) {
			if(_pticks[i] == -1) {
				break;
			}
			tick_history[i] = _pticks[i];
		}


		len = i;
		i = 0;
		int zombs = 0;
		for(; i < len; ++i)
		{
			int pid = tick_history[i];
			int _pid_index = -1, j = 0;
			for(; j < active_pids_n; j++)	{
				if(active_pids[j] == pid) {
					_pid_index = j;
					break;
				}
			}
			if(_pid_index == -1)	{
				_pid_index = active_pids_n;
				active_pids[_pid_index] = pid;
				active_pids_n++;
			}
			if (pstatus(pid) != PROCESS_ZOMBIE) {
				active_pids_ticks[_pid_index]++;
			} else {
				zombs++;
			}
		}

		if (!allow_zombies) {
			len -= zombs;
		}



		i = 0;
		int printed = 0;
		for(; i < PROCESS_MAX; ++i)	{
			int pid = pgetpid_at(i);

			if (pid != -1){
				int j = -1;
				for(; j < active_pids_n; j++)	{
					if(active_pids[j] == pid) {
						break;
					}
				}
				int ticks = ( j == -1 ) ? 0 : active_pids_ticks[j];
				top_data * data = (top_data *) malloc(sizeof(top_data));
				data->ticks = ticks;
				data->pid = pid;
				heap_insert(data, h);

			}
		} 
		i = 0;
		while(!heap_empty(h)) {
			top_data * data = heap_remove_min(h);
			if(0)	{
				break;
			} else {
				i++;
				int pid = data->pid;
				int ticks = data->ticks;

				char * _pname = pname(pid);
				char * status = NULL;

				int stat = pstatus(pid);
				switch(stat){
					case PROCESS_READY:
					status = "READY";
					break;
					case PROCESS_BLOCKED:
					status = "BLOCKED";
					break;
					case PROCESS_ZOMBIE:
					status = "ZOMBIE ";
					break;
					case PROCESS_RUNNING:
					status = "RUNNING";
					break;
					default:
					status = "UNKNOWN";
				}

				int priority = ppriority(pid);

				len = (!len) ? 1 : len;

				if (stat != PROCESS_ZOMBIE || (stat == PROCESS_ZOMBIE && allow_zombies) ) {
					printed++;

					printf("PID: %d \t NAME: %s \t CPU:%% %d \t STATUS: %s \t PRIORITY: %d\n",
						pid, _pname, 
						(100 * ticks) / len, status, priority);
				}
			}
		}
		
		printf("--------------------------------------------------------------------------------\n");
		sleep(1024);
	}
	while(1);
	
	return 0;
}

int getc_main (int argc, char ** argv) {	
	int c;
	while((c = getC()) != 1) {
		printf("%c", c + 1);
	}
	
	return 0;
}


int _kill (int argc, char ** argv)
{
	int c = getC();
	printf("%c\n", c);
	if (argc > 1) {
		int pid = atoi(argv[1]);
		if (pid >= 0) {
			kill(2,pid);
		}
	}
	return 0;
}

int _setp (int argc, char ** argv)
{
	if (argc > 2) {
		int priority = atoi(argv[2]);
		int pid = atoi(argv[1]);
		if (priority >= 0 && pid >= 0) {
			psetp(pid,priority);
		}
	}
	return 0;
}

int _setsched (int argc, char ** argv)
{
	if (argc > 1) {
		int priority = atoi(argv[1]);
		if (priority >= 0) {
			setsched(priority);
			if(priority == 1) {
				printf("Scheduler set to priority mode\n");
			}
			else if(priority == 0) {
				printf("Scheduler set to round robin mode\n");
			}
		}
	}
	return 0;
}

int _hang (int argc, char ** argv)
{
	int pid = pcreate("smallhang", 0, NULL);
	prun(pid);
	while(1);
	return 0;
}



int _dcheck(int size, char** args) {
	// check_drive(ATA0);
}

///////// TEST PROGRAM, NOT FOR ACTUAL USERS, MAKES DIRECT KERNEL CALLS !
int _dread(int argc, char** argv) {
	if(argc > 1)	{
		int i; 
		char msg[2048];
		for(i = 0; i < 2048; ++i)
		{
			msg[i] = 0;
		}

		unsigned int sector = 0;	

		sector = atoi(argv[1]);
		if(!sector)
		{
			sector = 1;
		}

		int offset            = 0;
		int count             = 1024;
		int ata               = ATA0;

		_disk_read(ata, msg, 1, sector);

		printf("Lei esto del sector %d\n", sector);
		i = 0;
		for(; i < count; ++i) {
			printf("%c", msg[i]);
		}
		printf("\n");
	}
}

///////// TEST PROGRAM, NOT FOR ACTUAL USERS, MAKES DIRECT KERNEL CALLS !
int _dwrite (int argc, char ** argv)
{
	char ans[1024] = "HARA BARA";
	int bytes = 1024;
	unsigned int sector = 1;
	if(argc > 2)
	{
		int i;
		int len = strlen(argv[1]);
		sector = atoi(argv[2]);
		if(len < 1024)
		{
			for(i = 0; i < len; ++i)	{
				ans[i] = argv[1][i];
			}
			for(; i < 1024; ++i)
			{
				ans[i] = 0;
			}
		}
		int offset = 0;
		int ata = ATA0;

		_disk_write(ata, ans, 1, sector);
		printf("Escrito en sector %d\n", sector);
	}
	return 0;
}

///////// TEST PROGRAM, NOT FOR ACTUAL USERS, MAKES DIRECT KERNEL CALLS !
char ans[65536];
int _dfill (int argc, char ** argv)
{
	int j = 0;
	
	for(j = 0; j < 1000; ++j)
	{
		int count = 384;
		int bytes = 65536;
		int sector = 1 + j * 128;
		if(argc > 1)
		{
			int i;
			sector = atoi(argv[1]);
			for(i = 0; i < 65536; ++i)	{
				ans[i] = '0' + (i / 512) % 10;
			}
			int offset = 0;
			int ata = ATA0;
	
			Cli();
			_disk_write(ata, ans, count, sector);
			for(i = 0; i < 65536; ++i)	{
				ans[i] = 0;
			}
			// _disk_read(ata, ans, count, sector);
			Sti();
			
				printf("%d\n", j);
			
	
			// for(i = 0; i < count; ++i)	{
			// 	if(ans[i * 512] != i % 10 + '0')	{
			// 		printf("Err %d %d %d %d\n", i % 10 + '0', ans[i * 512], i, j);
			// 		getC();
			// 		break;
			// 	}
			// }
	
		}
	}
	return 0;
}

int _cd(int argc, char ** argv) {
	if(argc > 1)
	{
		int dir = cd(argv[1]);
		if(dir == ERR_NO_EXIST) { 
			printf("Folder %s doesn't exist\n", argv[1]);			
		}
	}
}

int _pwd(int argc, char ** argv) {
	printf("%s\n", pwd());
}

int _ls (int argc, char ** argv)
{
	char buffer[1024];
	int off = 0;
	int dir = open("", O_RD);
	if(dir == ERR_PERMS) { 
		printf("You don't have the permissions to list %s\n", pwd());			
		return;
	}
	
	
	
	read(dir, buffer, 1024);
	dir_entry * d = (dir_entry *) &buffer;
	while(d->inode) {
		int len = d->name_len;
		int i = 0;
		if(*d->name != '.')
		{
			for(; i < len; ++i)	{
				printf("%c", d->name[i]);
			}
			printf("\t ");
		}

		
		if(!d->rec_len)	{
			break;
		}
		off += d->rec_len;
		
		if(off >= 1024)	{
			read(dir,buffer, 1024);
		}
		d = (dir_entry *) ((char*)&buffer + off);
	}
	if(off) {
		printf("\n");
	}

	return 0;
}

int _mkdir (int argc, char ** argv)
{
	if(argc > 1)	{
		if (mkdir(argv[1]) > 0) { 
			printf("Directory %s created\n", argv[1]);
		} else {
			printf("Directory %s not created, invalid perms? Already exists? D:\n", argv[1]);
		}
	}
	return 0;
}

int _rm (int argc, char ** argv)
{
	if(argc > 1)	{
		int code = rm(argv[1]);
		if(code > 0)	{
			printf("File %s deleted sucessfully.\n", argv[1]);
		} else if(code == ERR_NO_EXIST) {
			printf("File %s doesn't exist, focus boy!\n", argv[1]);
		} else if(code == ERR_PERMS) { 
			printf("You don't have the permissions to delete %s\n", argv[1]);			
		}
	}
	return 0;
}

int _touch(int argc, char ** argv)
{
	if(argc > 1)	{
		int code = open(argv[1], O_NEW);// FIXTHIS
		if(code > 0)	{
			printf("File %s created sucessfully.\n", argv[1]);
		} else if(code == ERR_NO_EXIST) {
			printf("File %s doesn't exist, focus boy!\n", argv[1]);
		} else if(code == ERR_PERMS) { 
			printf("You don't have the permissions to create %s\n", argv[1]);			
		}
	}
	return 0;
}

int _cat(int argc, char ** argv)
{
	if(argc > 1)	{
		int fd = open(argv[1], O_RD);
		
		if(fd == ERR_PERMS)	{
			printf("You are not allowed to read %s!\n", argv[1]);
			return;
		}
		if(fd == ERR_NO_EXIST)	{
			printf("The file %s doesn't exist\n", argv[1]);
			return;
		}
		char buffer[512];
		int i = 0;
		int readsomething = 0;
		int len = 0;
		while((len = read(fd, buffer, 512)) > 0) {
			readsomething = 1;
			i = 0;
			for(; i < 512 && buffer[i]; ++i) {
				printf("%c", buffer[i]);
			}
			for(i = 0; i < 512; ++i) {
				buffer[i] = 0;
			}
		}
		if(readsomething)
		{
			printf("\n");
		}

	}

	return 0;
}

char sect[8096];

int _fbulk(int argc, char ** argv) {
	if(argc > 2)	{
		int fd = open(argv[1], O_WR);
		int times = atoi(argv[2]);
		if(fd == ERR_PERMS)
		{
			printf("You are not allowed to write on %s!\n", argv[1]);
			return;
		}
		if(fd == ERR_NO_EXIST)
		{
			printf("The file %s doesn't exist\n", argv[1]);
			return;
		}
		
		if(!times) {
			times = 1;
		}
		printf("Printing %d times\n", times);
		int i = 0;
		for(i = 0; i < 8096; ++i)
		{
			sect[i] = 'a';
		}
		for(i = 0; i < times * 16; ++i)	{
			write(fd, sect, 512);
		}
	} else {
		printf("Params required: fbulk filename times\n");
		printf("Want to write more data? Try fbulk\n");
	}
	return 0;	
}

int _fwrite(int argc, char ** argv)
{
	if(argc > 2)	{
		int fd = open(argv[1], O_WR);
		if(fd == ERR_PERMS)	{
			printf("You are not allowed to write on %s!\n", argv[1]);
			return;
		}
		if(fd == ERR_NO_EXIST)	{
			printf("The file %s doesn't exist\n", argv[1]);
			return;
		}
		write(fd, argv[2], strlen(argv[2]) - 1);
	} else {
		printf("Params required: fwrite filename data\n");
		printf("Want to write more data? Try fbulk\n");
	}
	return 0;
}

int _logout(int argc, char ** argv) {
	clear_screen();
	printf("Bye bye!\n");
	logout();
	return 0;
}

int _makeuser(int argc, char ** argv) {
	if(argc > 2)
	{
		if(uexists(argv[1]) != -1)
		{
			printf("User %s already exists!\n", argv[1]);
		} else {
			if (makeuser(argv[1], argv[2]) != -1) {
				printf("User %s successfully created!!!\n", argv[1]);
			} else {
				printf("User %s already exists!!\n", argv[1]);
			}
		}
	}
	return 0;
}

int _setgid(int argc, char ** argv) {
	if(argc > 2)
	{
		if(uexists(argv[1]) != -1)
		{
			int uid = getuid(argv[1]);
			int oldgid = getgid(uid);
			if(oldgid == atoi(argv[2]))
			{
				printf("User already has that gid!\n");
				return 0;
			}
			
			if(setgid(argv[1], atoi(argv[2])) == -1)
			{
				printf("User %s doesn't exist or invalid perms.\n", argv[1]);
			} else {
				printf("User %s's gid changed from %d to %d\n", argv[1], oldgid, atoi(argv[2]));
			}
		} else {
			printf("User %s doesn't exist!\n", argv[1]);
		}
	}
	return 0;
}

int _udelete(int argc, char ** argv) {
	if(argc > 1)	{
		if(uexists(argv[1]) != -1)	{
			if(udelete(argv[1]) == -1)	{
				printf("User %s doesn't exist or invalid perms.\n", argv[1]);
			} else {
				printf("User %s deleted, poor boy!\n", argv[1]);
			}
		} else {
			printf("User %s doesn't exist!\n", argv[1]);
		}
	}
	return 0;
}

int _chown(int argc, char ** argv) {
	if(argc > 2)
	{
		if (chown(argv[1], argv[2]) >=0) {
			printf("Owner changed\n");
		} else {
			printf("Something just went WRONG, did you send me the username?\n");
		}
	} else {
		printf("Format: chown filename username\n");
	}
	return 0;
}

int _chmod(int argc, char ** argv) {
	if(argc > 2 && *argv[1] >= '0' && *argv[1] <= '9')
	{
		int val = 0;
		int j = 1;
		int i = strlen(argv[1]); 
		while(i >= 0) {
			if((argv[1][i] - '0') <= 9 && (argv[1][i] - '0') >= 0)
			{
				val += j * (argv[1][i] - '0');
				j *= 8;
			}
			i--;
		}
		
		if (chmod(argv[2], val) == 1) {
			printf("Permissions changed to %d\n", val);
		}else {
			printf("Error, invalid perms?\n");
		}
	} else {
		printf("Format: chmod perms file\n");
		printf("Example: chmod 777 myfile\n");
	}
	return 0;
}

int _getown(int argc, char ** argv) {
	if(argc > 1)
	{
		int res = fgetown(argv[1]);
		if(res >= 0) {
			printf("User #%d is %s's owner\n", res, argv[1]);
		} else {
			printf("Invalid file? Try that again man.\n");
		}
	}
	return 0;
}

int _getmod(int argc, char ** argv) {
	if(argc > 1)
	{
		int res = fgetmod(argv[1]);
		if(res >= 0) {
			printf("%s's permissions are %d\n", argv[1], res);
		} else {
			printf("Invalid file? Try that again man.\n");
		}
	}
	return 0;
}

int _finfo(int argc, char ** argv)	{
	if(argc > 1)	{
		int data[8];
		fs_finfo(argv[1], &data);
		printf("File name: %s\n", argv[1]);
		printf("Inode: %d\n", data[0]);
		printf("File blocks: %d\n", data[1]);
		printf("File bytes: %d\n", data[2]);
		printf("File permissions: %d\n", data[3]);
		printf("File uid: %d\n", data[4]);
		printf("File dir inode: %d\n", data[5]);
	}
	check_drive(0);
	return 0;
}


int _su(int argc, char ** argv)	{
	char * username = NULL;
	char * password = NULL;
	printf("Login: ");
	username = (char *) getConsoleString(1);
	if(strlen(username) > 0)	{
		printf("Password: ");
		password = (char *) getConsoleString(0);
		if (ulogin(username,password) != -1)	{
			printf("Successfully logged in as %s\n", username);
		} else {
			printf("Bad credentials, try again...\n");
		}
	}
	free(username);
	free(password);
}

int _link(int argc, char ** argv)	{
	if(argc > 2)
	{
		int code = makelink(argv[1],argv[2]);
		if(code > 0)	{
			printf("Link from %s to %s created sucessfully.\n", argv[1], argv[2]);
		} else if(code == ERR_NO_EXIST) {
			printf("File or folder %s doesn't exist, focus boy!\n", argv[2]);
		} else if(code == ERR_PERMS) { 
			printf("You don't have the permissions to create %s\n", argv[1]);			
		}
	}
}

int _cp(int argc, char ** argv) {
	if(argc > 2)
	{
		int code = cp(argv[1], argv[2]);
		if(code > 0)
		{
			printf("Copied from %s to %s successfully\n", argv[1], argv[2]);
		} 	else if(code == ERR_PERMS) { 
			printf("You don't have the permissions to copy %s\n", argv[1]);			
		}
	}
}

int _mv(int argc, char ** argv) {
	if(argc > 2)
	{
		int code = mv(argv[1], argv[2]);
		if(code > 0)	{
			printf("Moved from %s to %s successfully\n", argv[1], argv[2]);
		} 	else if(code == ERR_PERMS) { 
			printf("You don't have the permissions to move %s to %s\n", argv[1], argv[2]);			
		} else if(code == ERR_INVALID_TYPE)	{
			printf("Can't move: invalid filetype for %s\n", argv[2]);
		} else if(code == ERR_REPEATED)	{
			printf("Can't move: file %s already exists in destination %s\n", argv[1], argv[2]);
		} else {
			printf("WTF %d uncached error code D:\n", code);
		}
	}
}

int _smallhang(int argc, char ** argv) {
	
	if(argc > 1)
	{
		int msecs = atoi(argv[1]);
		printf("I wait %d msecs\n", msecs);
		sleep(msecs);
		printf("%d have passed\n", msecs);
	}

 	waitpid(getpid());
}

int _fsstat(int argc, char ** argv) {
	int data[8];
	fsstat(data);
	printf("Free blocks: %d\n", data[0]);
	printf("Total blocks: %d\n", data[1]);
	printf("Free inodes: %d\n", data[2]);
	printf("Total inodes: %d\n", data[3]);
	printf("Free Bytes: %d\n", data[4]);
	printf("Total available bytes: %d\n", data[5]);
}