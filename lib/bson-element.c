/* lib/bson-element.c - BSON element API
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

/** @file lib/bson-element.c
 */

#include <lmc/common.h>
#include <lmc/endian.h>
#include <lmc/bson-element.h>

#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <stdarg.h>

#define BSON_ELEMENT_NAME(e) ((char *)(e->as_typed.data))
#define BSON_ELEMENT_VALUE(e) \
  ((bson_element_value_t *)(e->as_typed.data + e->name_len))

typedef bson_element_t *(*bson_element_value_set_va_cb) (bson_element_t *e,
							 va_list ap);
typedef int32_t (*bson_element_value_get_size_cb) (const uint8_t *data);

typedef union
{
  double dbl;
  int32_t i32;
  int64_t i64;
  uint8_t bool;
  struct
  {
    int32_t len;
    char str[0];
  } str;
} bson_element_value_t;

struct _bson_element_t
{
  sig_atomic_t ref;
  uint32_t alloc;
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
bson_element_new_sized (uint32_t size)
{
  bson_element_t *e;

  e = (bson_element_t *)malloc (sizeof (bson_element_t) + size);
  memset (e, 0, sizeof (bson_element_t));

  e->ref = 1;
  e->as_typed.type = BSON_TYPE_NONE;
  e->alloc = size;
  e->len = 0;
  e->name_len = 0;

  return e;
}

bson_element_t *
bson_element_new (void)
{
  return bson_element_new_sized (0);
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

static inline bson_element_t *
bson_element_ensure_space (bson_element_t *e, int32_t size)
{
  if (size > 0 && e->len + e->name_len + size > e->alloc)
    {
      e->alloc = e->len + e->name_len + size;
      e = (bson_element_t *)realloc (e, sizeof (bson_element_t) + e->alloc);
    }
  return e;
}

bson_element_t *
bson_element_name_set (bson_element_t *e,
		       const char *name)
{
  if (!e)
    return NULL;

  if (name)
    {
      size_t name_len = strlen (name) + 1;
      uint32_t size = e->len - e->name_len;

      e = bson_element_ensure_space (e, name_len - e->name_len);
      e->len = size + name_len;

      memmove (e->as_typed.data + name_len,
	       e->as_typed.data + e->name_len,
	       size + 1);
      memcpy (e->as_typed.data, name, name_len);
      e->name_len = name_len;

      return e;
    }

  memmove (e->as_typed.data,
	   e->as_typed.data + e->name_len - 1,
	   e->len - e->name_len + 1);
  e->len -= e->name_len;
  e->name_len = 0;
  return e;
}

const uint8_t *
bson_element_data_get (bson_element_t *e)
{
  if (!e || e->len == 0)
    return NULL;
  return e->as_typed.data + e->name_len;
}

int32_t
bson_element_data_get_size (bson_element_t *e)
{
  if (!e)
    return -1;
  return e->len - e->name_len;
}

bson_element_t *
bson_element_data_reset (bson_element_t *e)
{
  if (!e)
    return NULL;
  e->len = e->name_len;
  return e;
}

bson_element_t *
bson_element_data_append (bson_element_t *e, const uint8_t *data,
			  uint32_t size)
{
  if (!e || !data || size == 0)
    return e;

  e = bson_element_ensure_space (e, size);
  memcpy (e->as_typed.data + e->len, data, size);
  e->len += size;
  return e;
}

bson_element_t *
bson_element_data_set (bson_element_t *e, const uint8_t *data,
		       uint32_t size)
{
  if (!e)
    return NULL;
  bson_element_data_reset (e);
  return bson_element_data_append (e, data, size);
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
  return e->len + 1;
}

static inline bson_element_value_t *
bson_element_data_type_get (bson_element_t *e, bson_element_type_t type)
{
  if (bson_element_type_get (e) != type || e->len == 0)
    return NULL;
  return BSON_ELEMENT_VALUE (e);
}

/** Element accessors **/

static inline bson_element_t *
_bson_element_meta_set (bson_element_t *e, bson_element_type_t type,
			uint32_t space, uint32_t len)
{
  if (!e)
    return NULL;

  e = bson_element_type_set (e, type);
  e = bson_element_ensure_space (e, space);
  e->len = e->name_len + len;
  return e;
}

/* stringish */
static inline bson_element_t *
_bson_element_value_set_stringish (bson_element_t *e,
				   bson_element_type_t type,
				   const char *val,
				   int32_t length)
{
  int32_t l = length;

  if (l <= BSON_LENGTH_AUTO)
    l = strlen (val);

  if (!(e = _bson_element_meta_set (e, type,
				    sizeof (int32_t) + l + 1,
				    sizeof (int32_t))))
    return NULL;


  BSON_ELEMENT_VALUE (e)->str.len = LMC_INT32_TO_LE (l + 1);
  e = bson_element_data_append (e, (uint8_t *)val, l);
  return bson_element_data_append (e, (uint8_t *)"\0", 1);
}

static inline lmc_bool_t
_bson_element_value_get_stringish (bson_element_t *e,
				   bson_element_type_t type,
				   const char **oval)
{
  bson_element_value_t *v = bson_element_data_type_get (e, type);

  if (!v || !oval)
    return FALSE;

  *oval = v->str.str;
  return TRUE;
}

static inline int32_t
_bson_element_value_get_size_stringish (const uint8_t *data)
{
  return LMC_INT32_FROM_LE (((bson_element_value_t *)data)->str.len) +
    (int32_t)sizeof (int32_t);
}

static bson_element_t *
_bson_element_value_set_stringish_va (bson_element_t *e,
				      bson_element_type_t type, va_list ap)
{
  va_list aq;
  char *s;
  bson_element_t *n;

  va_copy (aq, ap);
  s = va_arg (aq, char *);
  n = _bson_element_value_set_stringish (e, type, s, va_arg (aq, int32_t));
  va_end (aq);
  return n;
}

/* factories */
#include "bson-element-double.c"
#include "bson-element-int32.c"
#include "bson-element-int64.c"
#include "bson-element-datetime.c"
#include "bson-element-boolean.c"

#include "bson-element-string.c"
#include "bson-element-js-code.c"
#include "bson-element-symbol.c"

/* null */
static bson_element_t *
_bson_element_value_set_NULL_va (bson_element_t *e)
{
  e = bson_element_type_set (e, BSON_TYPE_NULL);
  e->len = e->name_len;
  return e;
}

static int32_t
_bson_element_value_get_size_NULL (void)
{
  return 0;
}

/** Builders **/

#define BSON_VALUE_SET_CB(type)						\
  [BSON_TYPE_##type] =							\
    (bson_element_value_set_va_cb) _bson_element_value_set_##type##_va

#define BSON_VALUE_GET_SIZE_CB(type)					\
  [BSON_TYPE_##type] =							\
    (bson_element_value_get_size_cb) _bson_element_value_get_size_##type

static bson_element_value_set_va_cb _bson_element_value_set_cbs[BSON_TYPE_MAX] =
{
  BSON_VALUE_SET_CB(DOUBLE),
  BSON_VALUE_SET_CB(INT32),
  BSON_VALUE_SET_CB(INT64),
  BSON_VALUE_SET_CB(STRING),
  BSON_VALUE_SET_CB(BOOLEAN),
  BSON_VALUE_SET_CB(UTC_DATETIME),
  BSON_VALUE_SET_CB(JS_CODE),
  BSON_VALUE_SET_CB(SYMBOL),
  [BSON_TYPE_NULL] =
   (bson_element_value_set_va_cb) _bson_element_value_set_NULL_va
};

static bson_element_value_get_size_cb _bson_element_value_get_size_cbs[BSON_TYPE_MAX] =
{
  BSON_VALUE_GET_SIZE_CB(DOUBLE),
  BSON_VALUE_GET_SIZE_CB(INT32),
  BSON_VALUE_GET_SIZE_CB(INT64),
  BSON_VALUE_GET_SIZE_CB(STRING),
  BSON_VALUE_GET_SIZE_CB(BOOLEAN),
  BSON_VALUE_GET_SIZE_CB(UTC_DATETIME),
  BSON_VALUE_GET_SIZE_CB(JS_CODE),
  BSON_VALUE_GET_SIZE_CB(SYMBOL),
  [BSON_TYPE_NULL] =
   (bson_element_value_get_size_cb) _bson_element_value_get_size_NULL
};

static bson_element_t *
bson_element_value_set_va (bson_element_t *e, bson_element_type_t type,
			   va_list ap)
{
  if (type >= BSON_TYPE_MAX)
    return e;

  if (_bson_element_value_set_cbs[type])
    return _bson_element_value_set_cbs[type] (e, ap);
  else
    return e;
}

bson_element_t *
bson_element_value_set (bson_element_t *e,
			bson_element_type_t type, ...)
{
  va_list ap;

  if (!e)
    return NULL;

  va_start (ap, type);
  e = bson_element_value_set_va (e, type, ap);
  va_end (ap);
  return e;
}

bson_element_t *
bson_element_set (bson_element_t *e, const char *name,
		  bson_element_type_t type, ...)
{
  va_list ap;

  if (!e)
    return NULL;

  e = bson_element_name_set (e, name);
  va_start (ap, type);
  e = bson_element_value_set_va (e, type, ap);
  va_end (ap);
  return e;
}

bson_element_t *
bson_element_create (const char *name,
		     bson_element_type_t type, ...)
{
  va_list ap;
  bson_element_t *e;

  e = bson_element_name_set (bson_element_new (), name);
  va_start (ap, type);
  e = bson_element_value_set_va (e, type, ap);
  va_end (ap);

  if (bson_element_type_get (e) != type)
    e = bson_element_unref (e);

  return e;
}

bson_element_t *
bson_element_new_from_data (const uint8_t *data)
{
  bson_element_t *e;
  uint8_t type;
  const char *name;
  const uint8_t *value;
  int32_t size = -1;

  if (!data)
    return NULL;

  type = data[0];
  name = (char *)(data + 1);
  value = data + 2 + strlen (name);

  if (type >= BSON_TYPE_MAX)
    return NULL;

  if (_bson_element_value_get_size_cbs[type])
    size = _bson_element_value_get_size_cbs[type] (value);

  if (size < 0)
    return NULL;

  e = bson_element_new_sized (size);
  e = bson_element_type_set (e, type);
  e = bson_element_name_set (e, name);
  e = bson_element_data_set (e, value, size);
  return e;
}

lmc_bool_t
bson_element_name_validate (const char *name, int flags)
{
  if (!name)
    return FALSE;

  if ((flags & BSON_ELEMENT_NAME_FORBID_DOTS) &&
      (strchr (name, '.') != NULL))
    return FALSE;

  if ((flags & BSON_ELEMENT_NAME_FORBID_DOLLAR) &&
      (name[0] == '$'))
    return FALSE;

  return TRUE;
}
