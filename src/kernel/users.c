#include "users.h"
#include "../libs/list.h"
#include "../libs/string.h"
#include "../../include/defs.h"
#include "tty.h"

list users;

int users_inode;

int max_uid = 0;

unsigned long file_r_offset = 0;

int user_comparer(user * u1, char * name) {
	int ret = strlen(u1->username) == strlen(name);
	if (ret) {
		return strcmp(u1->username, name);
	}
	else {
		return !ret;
	}
}

int	users_init() {
	
	users = list_init();
	
	int existed = 1;

	if (!(users_inode = fs_indir(".users", 1))) {
		existed = 0;
	}
	if(!existed)
	{
		users_inode = fs_open_file(".users", 1, O_RD | O_WR | O_CREAT );
	} else {
		users_inode = fs_open_file(".users", 1, O_RD | O_WR | O_NEW );
	}

	
	

	
	user buffer;
	max_uid = users_inode;
	int i = 0;
	while (fs_read_file(users_inode, &buffer, sizeof(buffer),&file_r_offset) > 0) {		
		user * u = (user *) malloc(sizeof(user));
		existed = 1;
		strcpy(u->username, buffer.username);
		strcpy(u->password, buffer.password);
		u->gid = buffer.gid;
		u->uid = buffer.uid;
		
		if(strlen(u->username) > 1)
		{
			list_add(users, u);
		}


		if(u->uid > max_uid)
		{
			max_uid = u->uid + 1;
		}
	}


	if(!existed || !list_size(users)) // Create root and save it
	{
		user * root = (user *) malloc(sizeof(user));
		strcpy(root->username, "root");
		strcpy(root->password, "root");
		root->uid = 0;
		root->gid = 0;

		fs_write_file(users_inode, root, sizeof(user));		
		
		user * guest = (user *) malloc(sizeof(user));
		strcpy(guest->username, "guest");
		strcpy(guest->password, "");
		guest->uid = 1;
		guest->gid = 1;
		
		fs_write_file(users_inode, guest, sizeof(user));
		
		list_add(users, root);
		list_add(users, guest);
	}

	for(; i < list_size(users); i++)
	{
		user * _u = list_get(users, i);
		printf("Username: %s %d\n", _u->username, strlen(_u->username));
	}			
	
	fs_bitmaps_write_all();

	
}

int	user_create(char * username, char * password, int gid) {
	if (list_indexOf(users, username, (void *)user_comparer) == -1) {
		user * u = (user*)malloc(sizeof(user));
		u->gid = gid;
		u->uid = max_uid++;
		strcpy(u->password, password);
		strcpy(u->username, username);
		fs_write_file(users_inode, u, sizeof(user));
		list_add(users, u);
		return 1;
	} else {
		return -1;
	}
}

int user_table_reload() {
	users_inode = fs_open_reg_file(".users", 1, O_CREAT); // Editing such file would be a pain, wouldn't it? D:
	
	int i = 0;
	for(; i < list_size(users); i++)
	{
		user * _u = list_get(users, i);
		fs_write_file(users_inode, _u, sizeof(user));	
	}
}

int	user_setgid(char * username, int gid) {
	int index = 0;
	if((index = list_indexOf(users, username, (void *) user_comparer)) == -1 )	{
		return -1;
	}
	else {
		user * u = list_get(users, index);
		

		u->gid = gid;

		user_table_reload();
		return 1;
	}
}

int		user_delete(char * username) {
	if(!strcmp(username, "root") && strlen(username) == strlen("root"))	{
		return -1;
	}
	
	int index = 0;
	if((index = list_indexOf(users, username, (void *) user_comparer)) == -1 )	{
		return -1;
	}
	list_remove(users, index);
	user_table_reload();
}

int		user_exists(char * username) {
	int index = list_indexOf(users, username, (void *) user_comparer);
	if(index == -1)
	{
		return -1;
	} else {
		user * u = list_get(users, index);
		return u->uid;
	}
}

int		user_login(char * username, char * password) {
	int i = list_indexOf(users, username, (void *) user_comparer);
	
	if(i == -1)
	{
		return -1;
	} else {
		user * u = list_get(users, i);
		if(strcmp(u->password, password) == 0 && strlen(u->password) == strlen(password) )
		{
			current_ttyc()->uid = u->uid;
			return u->uid;
		} else
		{
			return -1;
		}
	}
}



int		user_gid(int uid) {
	int i = 0;
	for(; i < list_size(users); i++)
	{
		user * _u = list_get(users, i);
		if(_u->uid == uid)	{
			return _u->gid;
		}
	}
	return -1;
}

int		user_logout() {
	current_ttyc()->uid = -1;
	return 1;
}