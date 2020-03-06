/*
 *  Copyright (C) 2007-2015 Lonelycoder AB
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  This program is also available under a commercial proprietary license.
 *  For more information, contact andreas@lonelycoder.com
 */
#ifndef HTTP_H__
#define HTTP_H__

#include <time.h>
#include "queue.h"


typedef enum {
    HTTP_CMD_GET,
    HTTP_CMD_HEAD,
    HTTP_CMD_POST,
    HTTP_CMD_SUBSCRIBE,
    HTTP_CMD_UNSUBSCRIBE,
} http_cmd_t;


#define HTTP_STATUS_OK           200
#define HTTP_STATUS_FOUND        302
#define HTTP_STATUS_BAD_REQUEST  400
#define HTTP_STATUS_UNAUTHORIZED 401
#define HTTP_STATUS_NOT_FOUND    404
#define HTTP_STATUS_METHOD_NOT_ALLOWED 405
#define HTTP_STATUS_PRECONDITION_FAILED 412
#define HTTP_STATUS_UNSUPPORTED_MEDIA_TYPE 415
#define HTTP_NOT_IMPLEMENTED 501

LIST_HEAD(http_header_list, http_header);

typedef struct http_header {
  LIST_ENTRY(http_header) hh_link;
  char *hh_key;
  char *hh_value;
} http_header_t;


void http_headers_free(struct http_header_list *headers);

const char *http_header_get(struct http_header_list *headers, 
			    const char *key);

void http_header_add(struct http_header_list *headers, const char *key,
		     const char *value, int appene);

void http_header_add_alloced(struct http_header_list *headers, const char *key,
			     char *value, int append);

void http_header_add_lws(struct http_header_list *headers, const char *data);

void http_header_add_int(struct http_header_list *headers, const char *key,
			 int value);

void http_header_merge(struct http_header_list *dst,
		       const struct http_header_list *src);

int http_ctime(time_t *tp, const char *d);

const char *http_asctime(time_t tp, char *out, size_t outlen);

void http_parse_uri_args(struct http_header_list *hc, char *args,
			 int append);

struct htsbuf_queue;
char *http_read_line(struct htsbuf_queue *q);

#endif // HTTP_H__
