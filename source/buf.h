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
#pragma once
#include <assert.h>
#include "atomic.h"
#include "rstr.h"
#include "compiler.h"

typedef struct buf {
  atomic_t b_refcount;
  size_t b_size;
  void *b_ptr;
  void (*b_free)(void *);
  rstr_t *b_content_type;
  uint8_t b_content[0];
} buf_t;

#define buf_data(buf) ((const void *)(buf)->b_ptr)

#define buf_cstr(buf) ((const char *)(buf)->b_ptr)

static __inline char *
buf_str(buf_t *b)
{
  assert(atomic_get(&b->b_refcount) == 1);
  return (char *)b->b_ptr;
}

#define buf_len(buf)  ((buf)->b_size)
#define buf_size(buf) ((buf)->b_size)


#define buf_c8(buf) ((const uint8_t *)(buf)->b_ptr)

void buf_release(buf_t *b);

buf_t *buf_make_writable(buf_t *b);

buf_t *buf_create(size_t size);

buf_t *buf_create_and_copy(size_t size, const void *data);

buf_t *buf_create_and_adopt(size_t size, void *data, void (*freefn)(void *));

buf_t *buf_create_from_malloced(size_t size, void *data);

static __inline buf_t *  attribute_unused_result
buf_retain(buf_t *b)
{
  atomic_inc(&b->b_refcount);
  return b;
}

