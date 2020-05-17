#undef FA_DUMP
#undef NO_ACTUAL_UNLINK

#include <assert.h>
#ifdef FA_DUMP
#include <sys/types.h>
#include <fcntl.h>
#endif
#include <sys/stat.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include <limits.h>

#include "db_support.h"
#include "fileaccess.h"
#include "minmax.h"
#include "fa_proto.h"
#include "buf.h"

static struct fa_protocol_list fileaccess_all_protocols;
//static HTS_MUTEX_DECL(fap_mutex);
static fa_protocol_t *native_fap;

//Functions
void
buf_release(buf_t *b)
{
  if(b != NULL && !atomic_dec(&b->b_refcount)) {
    if(b->b_free != NULL)
      b->b_free(b->b_ptr);
    rstr_release(b->b_content_type);
    free(b);
  }
}


void
fileaccess_register_entry(fa_protocol_t *fap)
{
  LIST_INSERT_HEAD(&fileaccess_all_protocols, fap, fap_link);
  if(fap->fap_name != NULL && !strcmp(fap->fap_name, "file"))
    native_fap = fap;
}

static int
fa_dir_cmp1(const fa_dir_entry_t *a, const fa_dir_entry_t *b)
{
  int r = strcmp(rstr_get(a->fde_url), rstr_get(b->fde_url));
  if(r)
    return r;
  return a < b ? 1 : -1;
}

void
fa_dir_insert(fa_dir_t *fd, fa_dir_entry_t *fde)
{
  if(RB_INSERT_SORTED(&fd->fd_entries, fde, fde_link, fa_dir_cmp1))
    abort();
  fd->fd_count++;
}

static fa_dir_entry_t *
fde_create(fa_dir_t *fd, const char *url, const char *filename, int type)
{
  fa_dir_entry_t *fde;

  if(filename[0] == '.')
    return NULL; /* Skip all dot-filenames */

  fde = calloc(1, sizeof(fa_dir_entry_t));

  fde->fde_url      = rstr_alloc(url);
  fde->fde_filename = rstr_alloc(filename);
  fde->fde_type     = type;

  if(fd != NULL)
    fa_dir_insert(fd, fde);

  return fde;
}

fa_dir_entry_t *
fa_dir_add(fa_dir_t *fd, const char *url, const char *filename, int type)
{
  return fde_create(fd, url, filename, type);
}

static void
fap_retain(fa_protocol_t *fap)
{
  atomic_inc(&fap->fap_refcount);
}

void
fap_release(fa_protocol_t *fap)
{
  if(fap->fap_fini == NULL)
    return;

  if(atomic_dec(&fap->fap_refcount))
    return;

  fap->fap_fini(fap);
}


char *
fa_resolve_proto(const char *url, fa_protocol_t **p,
		 char *errbuf, size_t errsize)
{
  //printf("fa_resolve_proto entrato\n");
  
  struct fa_stat fs;
  fa_protocol_t *fap;
  const char *url0 = url;
  char buf[URL_MAX];
  int n = 0;

  while(*url != ':' && *url>31 && n < sizeof(buf) - 1)
    buf[n++] = *url++;

  buf[n] = 0;

  if(!strcmp(buf, "data") && url[0] == ':') {
    extern fa_protocol_t fa_protocol_data;
    *p = &fa_protocol_data;
    return strdup(url + 1);
  }


  if(url[0] != ':' || url[1] != '/' || url[2] != '/') {
    /* No protocol specified, assume a plain file */
	//printf("fa_resolve_proto plain text\n");
	if (native_fap == NULL)
		printf("native_fap == NULL\n");
    if(native_fap == NULL ||
       (url0[0] != '/' &&
        native_fap->fap_stat(native_fap, url0, &fs, 0, NULL, 0))) {
      snprintf(errbuf, errsize, "File not found");
      return NULL;
    }
    *p = native_fap;
    return strdup(url0);
  }

  url += 3;

  if(!strcmp("dataroot", buf)) {
    const char *pfx = app_dataroot();
    snprintf(buf, sizeof(buf), "%s%s%s",
	     pfx, pfx[strlen(pfx) - 1] == '/' ? "" : "/", url);
    return fa_resolve_proto(buf, p, errbuf, errsize);
  }


  //hts_mutex_lock(&fap_mutex);

  LIST_FOREACH(fap, &fileaccess_all_protocols, fap_link) {

    if(fap->fap_match_proto != NULL) {
      if(fap->fap_match_proto(buf))
	continue;
    } else {
      if(strcmp(fap->fap_name, buf))
	continue;
    }

    fap_retain(fap);
    //hts_mutex_unlock(&fap_mutex);

    const char *fname = fap->fap_flags & FAP_INCLUDE_PROTO_IN_URL ? url0 : url;


    if(fap->fap_redirect != NULL) {
      rstr_t *newurl = fap->fap_redirect(fap, fname);
      if(newurl != NULL) {
        fap_release(fap);
        char *r = fa_resolve_proto(rstr_get(newurl), p, errbuf, errsize);
        rstr_release(newurl);
        return r;
      }
    }

    *p = fap;
    return strdup(fname);
  }
  //hts_mutex_unlock(&fap_mutex);
  snprintf(errbuf, errsize, "Protocol %s not supported", buf);
  return NULL;
}


void
fa_close(void *fh_)
{
  fa_handle_t *fh = fh_;
#ifdef FA_DUMP
  if(fh->fh_dump_fd != -1)
    close(fh->fh_dump_fd);
#endif
  fh->fh_proto->fap_close(fh);
}

int64_t
fa_seek4(void *fh_, int64_t pos, int whence, int lazy)
{
  fa_handle_t *fh = fh_;
#ifdef FA_DUMP
  if(fh->fh_dump_fd != -1) {
    printf("--------------------- Dumpfile seek to %ld (%d)\n", pos, whence);
    lseek(fh->fh_dump_fd, pos, whence);
  }
#endif
  return fh->fh_proto->fap_seek(fh, pos, whence, lazy);
}

int
fa_read(void *fh_, void *buf, size_t size)
{
  fa_handle_t *fh = fh_;
  if(size == 0)
    return 0;
  int r = fh->fh_proto->fap_read(fh, buf, size);
#ifdef FA_DUMP
  if(fh->fh_dump_fd != -1) {
    printf("---------------- Dumpfile write %zd bytes at %ld\n",
	   size, lseek(fh->fh_dump_fd, 0, SEEK_CUR));
    if(write(fh->fh_dump_fd, buf, size) != size)
      printf("Warning: Dump data write error\n");
  }
#endif
  return r;
}

int
fa_write(void *fh_, const void *buf, size_t size)
{
  fa_handle_t *fh = fh_;
  if(size == 0)
    return 0;
  return fh->fh_proto->fap_write(fh, buf, size);
}

int
fa_ftruncate(void *fh_, uint64_t newsize)
{
  fa_handle_t *fh = fh_;
  if(fh->fh_proto->fap_ftruncate == NULL)
    return FAP_NOT_SUPPORTED;
  return fh->fh_proto->fap_ftruncate(fh, newsize);
}

int64_t
fa_fsize(void *fh_)
{
  fa_handle_t *fh = fh_;
  return fh->fh_proto->fap_fsize(fh);
}

void *
fa_open_ex(const char *url, char *errbuf, size_t errsize, int flags,
	   struct fa_open_extra *foe)
{
  //printf("fa_open_ex entrato -  flags :%d - url:%s\n", flags, url);
  fa_protocol_t *fap;
  char *filename;
  fa_handle_t *fh;

  /*if(!(flags & FA_WRITE)) {
    // Only do caching if we are in read only mode

    if(flags & (FA_BUFFERED_SMALL | FA_BUFFERED_BIG)) {
      //printf("FA_BUFFERED_SMALL | FA_BUFFERED_BIG\n");
	  return fa_buffered_open(url, errbuf, errsize, flags, foe);
	}
  }*/

  if((filename = fa_resolve_proto(url, &fap, errbuf, errsize)) == NULL)
    return NULL;

  //printf("fa_open_ex WRITE...\n");
  if(flags & FA_WRITE && fap->fap_write == NULL) {
    snprintf(errbuf, errsize, "FS does not support writing");
    fh = NULL;
  } else {
    //printf("fap->fap_open\n");
	fh = fap->fap_open(fap, filename, errbuf, errsize, flags, foe);
  }
  fap_release(fap);
  free(filename);
#ifdef FA_DUMP
  if(flags & FA_DUMP)
    fh->fh_dump_fd = open("dumpfile.bin", O_CREAT | O_TRUNC | O_WRONLY, 0666);
  else
    fh->fh_dump_fd = -1;
#endif
  return fh;
}

int
fa_unlink(const char *url, char *errbuf, size_t errsize)
{
  fa_protocol_t *fap;
  char *filename;
  int r;

  if((filename = fa_resolve_proto(url, &fap, errbuf, errsize)) == NULL)
    return -1;

  if(fap->fap_unlink == NULL) {
    snprintf(errbuf, errsize, "No unlink support in filesystem");
    r = -1;
  } else {
    r = fap->fap_unlink(fap, filename, errbuf, errsize);
  }
  fap_release(fap);
  free(filename);
  return r;
}
