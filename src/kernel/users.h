#ifndef _USERS_H_
#define	_USERS_H_

// Default user: root
// Default gid : 0  (root's gid)
// Default secondary user: guest
// Guest's gid: 1  

typedef struct user {
	char username[128];
	char password[124];
	int		uid;
	int		gid;
} user;


int	users_init();

int		user_create(char * username, char * password, int gid);

int		user_setgid(char * username, int gid);

int		user_delete(char * username);

int		user_exists(char * username);

int		user_login(char * username, char * password);

int		user_gid(int uid);

int 	user_logout();

#endif