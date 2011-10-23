/* lib/bson.c - BSON API
 * Copyright (C) 2011 Gergely Nagy <algernon@balabit.hu>
 * This file is part of the libmongo-client library.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA.
 */

/** @file lib/bson.c
 */

#include <lmc/common.h>
#include <lmc/endian.h>
#include <lmc/bson.h>

#include <stdlib.h>
#include <signal.h>
#include <stdarg.h>
#include <string.h>

typedef struct _bson_node_t bson_node_t;

struct _bson_node_t
{
  bson_element_t *e;
  bson_node_t *next;
};

struct _bson_t
{
  sig_atomic_t ref;

  struct
  {
    uint32_t len;
    uint32_t total_size;
    bson_node_t *head;

    struct
    {
      bson_node_t **ptrs;
      uint32_t alloc;
    } index;
  } elements;
  struct
  {
    uint32_t alloc;
    uint32_t len;

    union
    {
      struct
      {
	uint32_t size;
	uint8_t data[0];
      } with_size;
      uint8_t data[0];
    };
  } stream;
};

static inline void
_bson_index_init (bson_t *b)
{
  b->elements.index.ptrs =
    (bson_node_t **)malloc (16 * sizeof (bson_node_t *));
  b->elements.index.alloc = 16;
}

static inline void
_bson_index_ensure_size (bson_t *b)
{
  if (bson_length (b) < b->elements.index.alloc)
    return;

  do
    {
      b->elements.index.alloc *= 2;
    }
  while (bson_length (b) > b->elements.index.alloc);

  b->elements.index.ptrs =
    (bson_node_t **)realloc (b->elements.index.ptrs,
			     b->elements.index.alloc * sizeof (bson_node_t *));
}

static inline void
_bson_index_rebuild (bson_t *b)
{
  bson_node_t *t;
  bson_node_t **index;
  uint32_t i = 0;

  _bson_index_ensure_size (b);

  index = b->elements.index.ptrs;
  t = b->elements.head;

  while (t)
    {
      index[i++] = t;
      t = t->next;
    }
}

static inline void
_bson_index_add (bson_t *b, uint32_t n, bson_node_t *node)
{
  _bson_index_ensure_size (b);
  b->elements.index.ptrs[n] = node;
}

bson_t *
bson_new_sized (uint32_t size)
{
  bson_t *b;

  b = (bson_t *)calloc (1, sizeof (bson_t) + size);

  b->ref = 1;
  b->stream.alloc = size;
  _bson_index_init (b);

  return b;
}

bson_t *
bson_new (void)
{
  return bson_new_sized (0);
}

static inline void
_bson_elements_drop (bson_t *b)
{
  if (b->elements.len > 0)
    {
      bson_node_t *o = b->elements.head;

      do
	{
	  bson_node_t *n = o->next;

	  bson_element_unref (o->e);
	  free (o);
	  o = n;
	}
      while (o);
    }
}

bson_t *
bson_reset_elements (bson_t *b)
{
  if (!b)
    return NULL;

  _bson_elements_drop (b);
  b->stream.len = 0;
  b->elements.len = 0;
  b->elements.head = NULL;
  b->elements.total_size = 0;
  return b;
}

bson_t *
bson_ref (bson_t *b)
{
  if (b)
    b->ref++;
  return b;
}

bson_t *
bson_unref (bson_t *b)
{
  if (!b)
    return NULL;
  b->ref--;
  if (b->ref <= 0)
    {
      _bson_elements_drop (b);
      if (b->elements.index.ptrs)
	free (b->elements.index.ptrs);
      free (b);
      return NULL;
    }
  return b;
}

static inline bson_t *
bson_flatten (bson_t *b)
{
  uint32_t size, i, pos = 0;
  bson_node_t **index;

  size = sizeof (int32_t) + sizeof (uint8_t) + b->elements.total_size;

  if (size > b->stream.alloc)
    {
      b = (bson_t *)realloc (b, sizeof (bson_t) + size);
      b->stream.alloc = size;
    }
  b->stream.len = size;
  b->stream.with_size.size = LMC_INT32_TO_LE (size);

  index = b->elements.index.ptrs;
  for (i = 1; i <= bson_length (b); i++)
    {
      memcpy (b->stream.with_size.data + pos,
	      bson_element_data_get (index[i - 1]->e),
	      bson_element_data_get_size (index[i - 1]->e));
      pos += bson_element_data_get_size (index[i - 1]->e);
    }
  b->stream.with_size.data[size] = 0;

  return b;
}

bson_t *
bson_open (bson_t *b)
{
  if (!b || b->stream.len == 0)
    return b;

  b->stream.len = 0;
  return b;
}

bson_t *
bson_close (bson_t *b)
{
  if (!b || b->stream.len != 0)
    return b;

  return bson_flatten (b);
}

uint32_t
bson_length (bson_t *b)
{
  if (!b)
    return 0;
  return b->elements.len;
}

const uint8_t *
bson_data_get (bson_t *b)
{
  if (!b || b->stream.len == 0)
    return NULL;
  return b->stream.data;
}

uint32_t
bson_data_get_size (bson_t *b)
{
  if (!b)
    return 0;
  return b->stream.len;
}

static inline bson_t *
bson_add_elements_va (bson_t *b, va_list ap)
{
  bson_node_t *t, dummy;
  bson_element_t *e;
  uint32_t c = 0;
  va_list aq;

  if (!b || b->stream.len != 0)
    return b;

  t = &dummy;

  va_copy (aq, ap);
  while ((e = va_arg (aq, bson_element_t *)) != NULL)
    {
      bson_node_t *n = (bson_node_t *)malloc (sizeof (bson_node_t));
      n->e = e;
      n->next = NULL;

      _bson_index_add (b, b->elements.len + c, n);
      b->elements.total_size += bson_element_stream_get_size (e);

      t->next = n;
      t = t->next;
      c++;
    }
  va_end (aq);

  b->elements.len += c;
  t->next = b->elements.head;
  b->elements.head = dummy.next;

  return b;
}

bson_t *
bson_add_elements (bson_t *b, ...)
{
  va_list ap;

  va_start (ap, b);
  b = bson_add_elements_va (b, ap);
  va_end (ap);
  return b;
}

bson_t *
bson_new_build (bson_element_t *e, ...)
{
  va_list ap;
  bson_t *b;

  if (!e)
    return NULL;

  b = bson_new ();
  va_start (ap, e);
  b = bson_add_elements (b, e, BSON_END);
  b = bson_add_elements_va (b, ap);
  va_end (ap);

  return b;
}

bson_element_t *
bson_get_nth_element (bson_t *b, uint32_t n)
{
  if (!b || n > bson_length (b))
    return NULL;

  return b->elements.index.ptrs[n - 1]->e;
}
