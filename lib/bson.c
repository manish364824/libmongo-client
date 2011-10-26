/* lib/bson.c - BSON API
 * Copyright (C) 2011 Gergely Nagy <algernon@balabit.hu>
 * This file is part of the libmongo-client library.
 *
 * This library is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program.  If not, see
 * <http://www.gnu.org/licenses/>.
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
  if (bson_elements_length (b) < b->elements.index.alloc)
    return;

  do
    {
      b->elements.index.alloc *= 2;
    }
  while (bson_elements_length (b) > b->elements.index.alloc);

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
bson_elements_reset (bson_t *b)
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
  for (i = 1; i <= bson_elements_length (b); i++)
    {
      memcpy (b->stream.with_size.data + pos,
	      bson_element_stream_get (index[i - 1]->e),
	      bson_element_stream_get_size (index[i - 1]->e));
      pos += bson_element_stream_get_size (index[i - 1]->e);
    }
  b->stream.with_size.data[pos] = 0;

  return b;
}

bson_t *
bson_stream_open (bson_t *b)
{
  if (!b || b->stream.len == 0)
    return b;

  b->stream.len = 0;
  return b;
}

bson_t *
bson_stream_close (bson_t *b)
{
  if (!b || b->stream.len != 0)
    return b;

  return bson_flatten (b);
}

uint32_t
bson_elements_length (bson_t *b)
{
  if (!b)
    return 0;
  return b->elements.len;
}

const uint8_t *
bson_stream_get_data (bson_t *b)
{
  if (!b || b->stream.len == 0)
    return NULL;
  return b->stream.data;
}

uint32_t
bson_stream_get_size (bson_t *b)
{
  if (!b)
    return 0;
  return b->stream.len;
}

static inline bson_t *
bson_elements_add_va (bson_t *b, va_list ap)
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
bson_elements_add (bson_t *b, ...)
{
  va_list ap;

  va_start (ap, b);
  b = bson_elements_add_va (b, ap);
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
  b = bson_elements_add (b, e, BSON_END);
  b = bson_elements_add_va (b, ap);
  va_end (ap);

  return b;
}

bson_element_t *
bson_elements_nth_get (bson_t *b, uint32_t n)
{
  if (!b || n > bson_elements_length (b) || n == 0)
    return NULL;

  return b->elements.index.ptrs[n - 1]->e;
}

bson_t *
bson_elements_nth_set (bson_t *b, uint32_t n, bson_element_t *e)
{
  if (!b)
    return NULL;
  if (n > bson_elements_length (b) || !e || n == 0)
    return b;

  bson_element_unref (b->elements.index.ptrs[n - 1]->e);
  b->elements.index.ptrs[n - 1]->e = e;
  return b;
}

uint32_t
bson_elements_key_find (bson_t *b, const char *key)
{
  uint32_t i = 0;
  uint32_t keylen;

  if (!b || !key)
    return 0;

  keylen = strlen (key);

  do
    {
      const char *n = bson_element_name_get (b->elements.index.ptrs[i]->e);
      uint32_t len = strlen (n);

      if (len == keylen && memcmp (n, key, len) == 0)
	return i + 1;
      i++;
    }
  while (i < bson_elements_length (b));

  return 0;
}

bson_element_t *
bson_elements_key_get (bson_t *b, const char *key)
{
  return bson_elements_nth_get (b, bson_elements_key_find (b, key));
}

bson_t *
bson_elements_key_set (bson_t *b, const char *key,
		       bson_element_t *e)
{
  return bson_elements_nth_set (b, bson_elements_key_find (b, key), e);
}

static bson_t *
_bson_stream_parse (bson_t *b, const uint8_t *data)
{
  const uint8_t *t;
  uint32_t *size;

  size = (uint32_t *)data;
  *size -= sizeof (int32_t) + 1;

  t = data + sizeof (int32_t);

  while ((uint32_t)(t - data) < *size)
    {
      bson_element_t *e = bson_element_new_from_data (t);

      if (!e)
	break;

      b = bson_elements_add (b, e, BSON_END);
      t += bson_element_stream_get_size (e);
    }

  return b;
}

bson_t *
bson_stream_merge (bson_t *b, const uint8_t *data)
{
  if (!b)
    return NULL;
  if (!data || b->stream.len != 0)
    return b;

  return _bson_stream_parse (b, data);
}

bson_t *
bson_stream_set (bson_t *b, const uint8_t *data)
{
  return bson_stream_merge (bson_elements_reset (b), data);
}

bson_t *
bson_new_from_data (const uint8_t *data)
{
  return bson_stream_merge (bson_new (), data);
}

bson_t *
bson_elements_merge (bson_t *b, bson_t *src)
{
  uint32_t i;

  if (!b)
    return NULL;
  if (!src)
    return b;

  if (b->stream.len != 0)
    {
      bson_unref (src);
      return b;
    }

  for (i = 1; i <= bson_elements_length (src); i++)
    b = bson_elements_add
      (b, bson_element_ref (bson_elements_nth_get (src, i)), BSON_END);
  bson_unref (src);
  return b;
}

bson_t *
bson_elements_nth_remove (bson_t *b, uint32_t n)
{
  bson_node_t *old, *cn;

  if (!b)
    return NULL;
  if (bson_elements_length (b) < n || n == 0)
    return b;

  cn = b->elements.index.ptrs[n - 1];

  if (cn == b->elements.head)
    {
      old = b->elements.head;
      b->elements.head = b->elements.head->next;
    }
  else
    {
      old = b->elements.head;

      while (old->next != cn)
	old = old->next;

      old->next = cn->next;
      old = cn;
    }
  b->elements.len--;
  b->elements.total_size -= bson_element_stream_get_size (old->e);

  bson_element_unref (old->e);
  free (old);

  _bson_index_rebuild (b);
  return b;
}

bson_t *
bson_elements_key_remove (bson_t *b, const char *key)
{
  return bson_elements_nth_remove (b, bson_elements_key_find (b, key));
}
