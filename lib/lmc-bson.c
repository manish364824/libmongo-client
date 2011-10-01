/* lmc-bson.c - libmongo-client's BSON implementation
 * Copyright (C) 2011 Gergely Nagy <algernon@balabit.hu>
 * This file is part of the libmongo-client library.
 *
 * This library free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2.1 of
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

/** @file lib/lmc-bson.c
 * Implementation of the BSON API.
 */

#include <lmc.h>
#include "lmc-private.h"

#include <errno.h>

/** @internal BSON structure.
 */
struct _lmc_bson_t
{
  lmc_error_t err; /**< The first error raised while working with the
		      object. */

  uint8_t *data; /**< The actual data of the BSON object. */
  uint32_t size;
  uint32_t alloc;

  lmc_bool_t finished; /**< Flag to indicate whether the object is open
			  or finished. */
};

static inline lmc_bool_t
_lmc_bson_verify_obj (const bson_t *b)
{
  if (!b)
    {
      errno = EINVAL;
      return FALSE;
    }
  if ((errno = lmc_error_get_errn (b)) != 0)
    return FALSE;
  return TRUE;
}

static inline bson_t *
_lmc_bson_ensure_space (bson_t *b, uint32_t size)
{
  if (b->alloc >= size)
    return b;

  b->data = lmc_realloc (b->data, size);
  b->alloc = size;
  return b;
}

static inline bson_t *
_lmc_bson_append_data (bson_t *b, const uint8_t *data, uint32_t size)
{
  b = _lmc_bson_ensure_space (b, b->size + size);
  memcpy (b->data + b->size, data, size);
  b->size += size;

  return b;
}

static inline bson_t *
_lmc_bson_append_int8 (bson_t *b, uint8_t i)
{
  return _lmc_bson_append_data (b, &i, sizeof (uint8_t));
}

static inline bson_t *
_lmc_bson_append_int32 (bson_t *b, int32_t i)
{
  return _lmc_bson_append_data (b, (uint8_t *)&i, sizeof (int32_t));
}

static inline bson_t *
_lmc_bson_append_int64 (bson_t *b, int64_t i)
{
  return _lmc_bson_append_data (b, (uint8_t *)&i, sizeof (int64_t));
}

#define _LMC_APPEND_HEADER(b,type,name,size)				\
  if (_lmc_bson_append_element_header (b, type, name, size) == NULL)	\
    return NULL;							\
  if (!lmc_error_isok (b))						\
    return b

static inline bson_t *
_lmc_bson_append_element_header (bson_t *b, bson_type_t type,
				 const char *name, int32_t full_size)
{
  if (!_lmc_bson_verify_obj (b))
    return b;
  if (b->finished)
    lmc_error_raise (b, EBUSY);
  if (!name)
    lmc_error_raise (b, EINVAL);
  if (full_size <= 0)
    lmc_error_raise (b, ERANGE);

  _lmc_bson_ensure_space (b, b->size + full_size + strlen (name) + 1);

  return _lmc_bson_append_data
    (_lmc_bson_append_int8 (b, (uint8_t)type),
     (const uint8_t *)name, strlen (name) + 1);
}

static inline bson_t *
_lmc_bson_append_string_element (bson_t *b, bson_type_t type,
				 const char *name, const char *val,
				 int32_t length)
{
  size_t len;

  if (!val)
    lmc_error_raise (b, EINVAL);
  if (length == 0 || length < -1)
    lmc_error_raise (b, ERANGE);

  len = (length != -1) ? (size_t)length + 1 : strlen (val) + 1;

  _LMC_APPEND_HEADER (b, type, name, len);

  return _lmc_bson_append_int8
    (_lmc_bson_append_data
     (_lmc_bson_append_int32 (b, len),
      (uint8_t *)val, len - 1), 0);
}

static inline bson_t *
_lmc_bson_append_document_element (bson_t *b, bson_type_t type,
				   const char *name, const bson_t *doc)
{
  _LMC_APPEND_HEADER (b, type, name, bson_size (doc));
  return _lmc_bson_append_data (b, bson_data (doc), bson_size (doc));
}

/********************
 * Public interface *
 ********************/

const char *
bson_type_as_string (bson_type_t type)
{
  switch (type)
    {
    case BSON_TYPE_NONE:
      return "BSON_TYPE_NONE";
    case BSON_TYPE_DOUBLE:
      return "BSON_TYPE_DOUBLE";
    case BSON_TYPE_STRING:
      return "BSON_TYPE_STRING";
    case BSON_TYPE_DOCUMENT:
      return "BSON_TYPE_DOCUMENT";
    case BSON_TYPE_ARRAY:
      return "BSON_TYPE_ARRAY";
    case BSON_TYPE_BINARY:
      return "BSON_TYPE_BINARY";
    case BSON_TYPE_UNDEFINED:
      return "BSON_TYPE_UNDEFINED";
    case BSON_TYPE_OID:
      return "BSON_TYPE_OID";
    case BSON_TYPE_BOOLEAN:
      return "BSON_TYPE_BOOLEAN";
    case BSON_TYPE_UTC_DATETIME:
      return "BSON_TYPE_UTC_DATETIME";
    case BSON_TYPE_NULL:
      return "BSON_TYPE_NULL";
    case BSON_TYPE_REGEXP:
      return "BSON_TYPE_REGEXP";
    case BSON_TYPE_DBPOINTER:
      return "BSON_TYPE_DBPOINTER";
    case BSON_TYPE_JS_CODE:
      return "BSON_TYPE_JS_CODE";
    case BSON_TYPE_SYMBOL:
      return "BSON_TYPE_SYMBOL";
    case BSON_TYPE_JS_CODE_W_SCOPE:
      return "BSON_TYPE_JS_CODE_W_SCOPE";
    case BSON_TYPE_INT32:
      return "BSON_TYPE_INT32";
    case BSON_TYPE_TIMESTAMP:
      return "BSON_TYPE_TIMESTAMP";
    case BSON_TYPE_INT64:
      return "BSON_TYPE_INT64";
    case BSON_TYPE_MIN:
      return "BSON_TYPE_MIN";
    case BSON_TYPE_MAX:
      return "BSON_TYPE_MAX";
    default:
      errno = EINVAL;
      return NULL;
  }
}

bson_t *
bson_new (void)
{
  return bson_new_sized (0);
}

bson_t *
bson_new_sized (uint32_t size)
{
  bson_t *b;

  b = lmc_new (bson_t);
  b->size = sizeof (uint32_t);
  b->alloc = size + sizeof (uint32_t) + sizeof (uint8_t);
  b->finished = FALSE;
  lmc_error_reset (&b->err);

  b->data = lmc_alloc (uint8_t, b->alloc);

  return b;
}

bson_t *
bson_new_from_data (const uint8_t *data, int32_t size)
{
  bson_t *b;

  if (!data)
    {
      errno = EINVAL;
      return NULL;
    }
  if (size <= 0)
    {
      errno = ERANGE;
      return NULL;
    }

  b = lmc_new (bson_t);
  b->alloc = b->size = size;
  b->finished = FALSE;
  lmc_error_reset (&b->err);

  b->data = lmc_alloc (uint8_t, b->alloc);

  memcpy (b->data, data, size);

  return b;
}

bson_t *
bson_finish (bson_t *b)
{
  int32_t *i;

  if (!_lmc_bson_verify_obj (b))
    return b;
  if (b->finished)
    return b;

  _lmc_bson_append_int8 (b, 0);

  i = (int32_t *) (&b->data[0]);
  *i = LMC_INT32_TO_LE ((int32_t) (b->size));

  b->finished = TRUE;

  return b;
}

bson_t *
bson_reset (bson_t *b)
{
  if (!_lmc_bson_verify_obj (b))
    return b;

  b->finished = FALSE;
  b->size = sizeof (uint32_t);

  return b;
}

bson_t *
bson_free (bson_t *b)
{
  if (!b)
    return NULL;

  free (b->data);
  free (b);

  return NULL;
}

int32_t
bson_size (const bson_t *b)
{
  if (!_lmc_bson_verify_obj (b))
    return -1;
  if (!b->finished)
    {
      errno = EAGAIN;
      return -1;
    }

  return b->size;
}

const uint8_t *
bson_data (const bson_t *b)
{
  if (!_lmc_bson_verify_obj (b))
    return NULL;
  if (!b->finished)
    {
      errno = EAGAIN;
      return NULL;
    }

  return b->data;
}

lmc_bool_t
bson_validate_key (const char *key,
		   bson_valid_context_t flags)
{
  if (!key)
    {
      errno = EINVAL;
      return FALSE;
    }
  errno = 0;

  if ((flags & BSON_VALID_CONTEXT_NO_DOTS) &&
      (strchr (key, '.') != NULL))
    return FALSE;

  if ((flags & BSON_VALID_CONTEXT_NO_DOLLAR) &&
      (key[0] == '$'))
    return FALSE;

  return TRUE;
}

bson_t *
bson_append_string (bson_t *b, const char *name, const char *val,
		    int32_t length)
{
  return _lmc_bson_append_string_element (b, BSON_TYPE_STRING,
					  name, val, length);
}

bson_t *
bson_append_double (bson_t *b, const char *name, double d)
{
  double v = LMC_DOUBLE_TO_LE (d);

  _LMC_APPEND_HEADER (b, BSON_TYPE_DOUBLE, name, sizeof (double));

  return _lmc_bson_append_data (b, (uint8_t *)&v, sizeof (double));
}

bson_t *
bson_append_document (bson_t *b, const char *name, const bson_t *doc)
{
  return _lmc_bson_append_document_element (b, BSON_TYPE_DOCUMENT, name, doc);
}

bson_t *
bson_append_array (bson_t *b, const char *name, const bson_t *array)
{
  return _lmc_bson_append_document_element (b, BSON_TYPE_ARRAY, name, array);
}

bson_t *
bson_append_binary (bson_t *b, const char *name,
		    bson_binary_subtype_t subtype,
		    const uint8_t *data, int32_t size)
{
  if (!data)
    lmc_error_raise (b, EINVAL);

  _LMC_APPEND_HEADER (b, BSON_TYPE_BINARY, name, size);

  return _lmc_bson_append_data
    (_lmc_bson_append_int8
     (_lmc_bson_append_int32 (b, LMC_INT32_TO_LE (size)), (uint8_t)subtype),
     data, size);
}

bson_t *
bson_append_oid (bson_t *b, const char *name, const bson_oid_t *oid)
{
  if (!oid)
    lmc_error_raise (b, EINVAL);

  _LMC_APPEND_HEADER (b, BSON_TYPE_OID, name, 12);

  return _lmc_bson_append_data (b, (const uint8_t *)oid->bytes, 12);
}

bson_t *
bson_append_boolean (bson_t *b, const char *name, lmc_bool_t value)
{
  _LMC_APPEND_HEADER (b, BSON_TYPE_BOOLEAN, name, 1);

  return _lmc_bson_append_int8 (b, (value) ? 1 : 0);
}

bson_t *
bson_append_utc_datetime (bson_t *b, const char *name, int64_t ts)
{
  _LMC_APPEND_HEADER (b, BSON_TYPE_UTC_DATETIME, name, sizeof (int64_t));
  return _lmc_bson_append_int64 (b, ts);
}

bson_t *
bson_append_null (bson_t *b, const char *name)
{
  _LMC_APPEND_HEADER (b, BSON_TYPE_NULL, name, 1);
  return b;
}

bson_t *
bson_append_regex (bson_t *b, const char *name, const char *regexp,
		   const char *options)
{
  if (!regexp || !options)
    lmc_error_raise (b, EINVAL);

  _LMC_APPEND_HEADER (b, BSON_TYPE_REGEXP, name,
		      strlen (regexp) + strlen (options) + 1);
  return _lmc_bson_append_data
    (_lmc_bson_append_data (b, (const uint8_t *)regexp, strlen (regexp) + 1),
     (const uint8_t *)options, strlen (options) + 1);
}

bson_t *
bson_append_javascript (bson_t *b, const char *name, const char *js,
			int32_t len)
{
  return _lmc_bson_append_string_element (b, BSON_TYPE_JS_CODE, name,
					  js, len);
}

bson_t *
bson_append_symbol (bson_t *b, const char *name, const char *symbol,
		    int32_t len)
{
  return _lmc_bson_append_string_element (b, BSON_TYPE_SYMBOL, name,
					  symbol, len);
}

bson_t *
bson_append_javascript_w_scope (bson_t *b, const char *name,
				const char *js, int32_t len,
				const bson_t *scope)
{
  int32_t size;
  size_t length;

  if (!js || bson_size (scope) < 0)
    lmc_error_raise (b, EINVAL);
  if (len < -1)
    lmc_error_raise (b, ERANGE);

  length = (len != -1) ? (size_t)len + 1 : strlen (js) + 1;
  size = length + sizeof (int32_t) * 2 + bson_size (scope);

  _LMC_APPEND_HEADER (b, BSON_TYPE_JS_CODE_W_SCOPE, name, size);

  return _lmc_bson_append_data
    (_lmc_bson_append_int8
     (_lmc_bson_append_data
      (_lmc_bson_append_int32
       (_lmc_bson_append_int32 (b, LMC_INT32_TO_LE (size)),
	LMC_INT32_TO_LE (length)),
       (const uint8_t *)js, length - 1), 0),
     bson_data (scope), bson_size (scope));
}

bson_t *
bson_append_int32 (bson_t *b, const char *name, int32_t i)
{
  _LMC_APPEND_HEADER (b, BSON_TYPE_INT32, name, sizeof (int32_t));

  return _lmc_bson_append_int32 (b, LMC_INT32_TO_LE (i));
}

bson_t *
bson_append_timestamp (bson_t *b, const char *name,
		       bson_timestamp_t *ts)
{
  int64_t v;

  _LMC_APPEND_HEADER (b, BSON_TYPE_TIMESTAMP, name, sizeof (int64_t));
  memcpy (&v, ts, sizeof (v));
  return _lmc_bson_append_int64 (b, v);
}

bson_t *
bson_append_int64 (bson_t *b, const char *name, int64_t i)
{
  _LMC_APPEND_HEADER (b, BSON_TYPE_INT64, name, sizeof (int64_t));
  return _lmc_bson_append_int64 (b, i);
}
