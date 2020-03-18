#include <assert.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <fcntl.h>
#include <errno.h>
#include <limits.h>

#include "fileaccess.h"
#include "minmax.h"
#include "db_support.h"
#include "fa_proto.h"

typedef struct part {
  int fd;
  int64_t size;
} part_t;

typedef struct fs_handle {
  fa_handle_t h;
  int part_count;
  int64_t total_size; // Only valid if part_count != 1
  int64_t read_pos;
  part_t parts[0];
} fs_handle_t;

void
fs_urlsnprintf(char *buf, size_t bufsize, const char *prefix, const char *base,
	       const char *fname);

int
is_splitted_file_name(const char *s);

int
file_exists(char *fn);

void
get_split_piece_name(char *dst, size_t dstlen, const char *fn, int num);

int
split_exists(const char* fn,int num);

unsigned int
get_split_piece_count(const char *fn);

int fs_scandir(fa_protocol_t *fap, fa_dir_t *fd, const char *url,
           char *errbuf, size_t errlen, int flags);
		   
void
fs_close(fa_handle_t *fh0);

fa_handle_t *
fs_open(fa_protocol_t *fap, const char *url, char *errbuf, size_t errlen,
	int flags, struct fa_open_extra *foe);
	
int
get_current_read_piece_num(fs_handle_t *fh);

int
fs_read(fa_handle_t *fh0, void *buf, size_t size);

int
fs_write(fa_handle_t *fh0, const void *buf, size_t size);

int64_t
fs_seek(fa_handle_t *fh0, int64_t pos, int whence, int lazy);

int64_t
fs_fsize(fa_handle_t *fh0);

int
fs_stat(fa_protocol_t *fap, const char *url, struct fa_stat *fs,
	int flags, char *errbuf, size_t errlen);
	
int
fs_rmdir(const fa_protocol_t *fap, const char *url, char *errbuf, size_t errlen);

int
fs_unlink(const fa_protocol_t *fap, const char *url,
          char *errbuf, size_t errlen);
		  
fa_err_code_t
fs_makedir(struct fa_protocol *fap, const char *url);

int
fs_rename(const fa_protocol_t *fap, const char *old, const char *new,
          char *errbuf, size_t errlen);
		  
void
fs_notify(struct fa_protocol *fap, const char *url,
	  void *opaque,
	  void (*change)(void *opaque,
			 fa_notify_op_t op, 
			 const char *filename,
			 const char *url,
			 int type),
	  int (*breakcheck)(void *opaque));
	  

int
fs_normalize(struct fa_protocol *fap, const char *url, char *dst, size_t dstlen);

fa_err_code_t
fs_set_xattr(struct fa_protocol *fap, const char *url,
             const char *name,
             const void *data, size_t len);
			 
fa_err_code_t
fs_get_xattr(struct fa_protocol *fap, const char *url,
             const char *name,
             void **datap, size_t *lenp);
			 
fa_err_code_t
fs_fsinfo(struct fa_protocol *fap, const char *url, fa_fsinfo_t *ffi);

fa_err_code_t
fs_fsinfo(struct fa_protocol *fap, const char *url, fa_fsinfo_t *ffi);

int
fs_ftruncate(fa_handle_t *fh0, uint64_t newsize);

