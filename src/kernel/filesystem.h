#ifndef _FD_H_
#define _FD_H_

#define _FS_INST_MAX       1024

int fs_open (char * filename, int perms, char * mode);

// No tan necesario :p
int fs_read (int fd, char * buffer, int len);

// No tan necesario :p
int fs_write (int fd, char * buffer, int len);

int fs_close (int fd);

int fs_instance_open ();			// FS Instance sirve para navegar por el fs. Cada proceso tiene 1.

int fs_instance_close (int instance);

int fs_pwd     (int fs_inst);

int fs_mkdir   (char * name, int len);

int fs_link    (char * name, int len); // Hard link

int fs_symlink (char * name, int len); // Symlink

int fs_rm (char * name);

int fs_cd (char * name);

int fs_mv (char * name);

#endif