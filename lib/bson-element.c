/* lib/bson-element.c - BSON element API
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

/** @file lib/bson-element.c
 */

#include <lmc/common.h>
#include <lmc/bson-element.h>

#include <stdlib.h>
#include <signal.h>
#include <string.h>

#define BSON_ELEMENT_NAME(e) ((char *)(e->as_typed.data))

struct _bson_element_t
{
  sig_atomic_t ref;
  uint32_t len;
  uint32_t name_len;

  union
  {
    struct
    {
      uint8_t type;
      uint8_t data[0];
    } as_typed;
    uint8_t bytestream[0];
  };
};

bson_element_t *
bson_element_new (void)
{
  bson_element_t *e;

  e = (bson_element_t *)malloc (sizeof (bson_element_t));
  memset (e, 0, sizeof (bson_element_t));

  e->ref = 1;
  e->as_typed.type = BSON_TYPE_NONE;
  e->len = 0;
  e->name_len = 0;

  return e;
}

bson_element_t *
bson_element_ref (bson_element_t *e)
{
  if (e)
    e->ref++;
  return e;
}

bson_element_t *
bson_element_unref (bson_element_t *e)
{
  if (!e)
    return NULL;

  e->ref--;
  if (e->ref <= 0)
    {
      free (e);
      return NULL;
    }
  return e;
}

bson_element_type_t
bson_element_type_get (bson_element_t *e)
{
  if (!e)
    return BSON_TYPE_NONE;
  return e->as_typed.type;
}

bson_element_t *
bson_element_type_set (bson_element_t *e,
		       bson_element_type_t type)
{
  if (!e)
    return NULL;

  e->as_typed.type = type;
  return e;
}

const char *
bson_element_name_get (bson_element_t *e)
{
  if (!e)
    return NULL;
  return BSON_ELEMENT_NAME (e);
}

bson_element_t *
bson_element_name_set (bson_element_t *e,
		       const char *name)
{
  if (!e)
    return NULL;

  if (name)
    {
      size_t name_len = strlen (name);
      uint32_t size = e->len - e->name_len;

      if (name_len > e->name_len)
	e = realloc (e, sizeof (bson_element_t) + size + name_len);

      memmove (e->as_typed.data + name_len + 1,
	       e->as_typed.data + e->name_len + 1,
	       e->len - e->name_len);
      memcpy (e->as_typed.data, name, name_len + 1);
      e->name_len = name_len;
      e->len = size + name_len;
    }
  else
    {
      memmove (e->as_typed.data, e->as_typed.data + e->name_len,
	       e->len - e->name_len + 1);
      e->len -= e->name_len;
      e->name_len = 0;
    }

  return e;
}

const uint8_t *
bson_element_data_get (bson_element_t *e)
{
  if (!e || e->len == 0)
    return NULL;
  return e->as_typed.data + e->name_len + 1;
}

int32_t
bson_element_data_get_size (bson_element_t *e)
{
  if (!e)
    return -1;
  return e->len - e->name_len;
}

bson_element_t *
bson_element_data_set (bson_element_t *e, const uint8_t *data,
		       uint32_t size)
{
  uint32_t new_size;

  if (!e)
    return NULL;
  if (size == 0 || data == NULL)
    {
      e->len = e->name_len;
      return e;
    }

  new_size = e->name_len + size;
  if (new_size > e->len)
    e = (bson_element_t *)realloc (e, sizeof (bson_element_t) + new_size + 1);
  memcpy (e->as_typed.data + e->name_len + 1, data, size);
  e->len = new_size;
  return e;
}

const uint8_t *
bson_element_stream_get (bson_element_t *e)
{
  if (!e)
    return NULL;
  return e->bytestream;
}

int32_t
bson_element_stream_get_size (bson_element_t *e)
{
  if (!e)
    return -1;
  return e->len + e->name_len + 1;
}

bson_element_t *
bson_element_value_set_double (bson_element_t *e,
			       double val)
{
  return bson_element_data_set (e, (uint8_t *)&val, sizeof (double));
}

lmc_bool_t
bson_element_value_get_double (bson_element_t *e,
			       double *oval)
{
  double *d;

  if (!oval)
    return FALSE;

  d = (double *)bson_element_data_get (e);
  if (d)
    *oval = *d;
  return (d != NULL);
}
