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
  uint32_t size; /**< The current size of the BSON object. */
  uint32_t alloc; /**< Bytes currently allocated for the object. May
		     be bigger than size. */

  lmc_bool_t finished; /**< Flag to indicate whether the object is open
			  or finished. */
};

/** @internal BSON cursor structure.
 */
struct _lmc_bson_cursor_t
{
  lmc_error_t err; /**< The first error raised while working with the
		      object. */
  const bson_t *obj; /**< The BSON object this is a cursor for. */
  const char *key; /**< Pointer within the BSON object to the
		      current key. */
  size_t pos; /**< Position within the BSON object, pointing at the
		 element type. */
  size_t value_pos; /**< The start of the value within the BSON
		       object, pointing right after the end of the
		       key. */
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

/* Constructors & meta-data access */

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

/* Append functions */

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

/* Cursor functions */
bson_cursor_t *
bson_cursor_new (const bson_t *b)
{
  bson_cursor_t *c;

  if (bson_size (b) == -1)
    {
      errno = EINVAL;
      return NULL;
    }

  c = lmc_new (bson_cursor_t);
  c->obj = b;
  c->key = NULL;
  c->pos = 0;
  c->value_pos = 0;
  lmc_error_reset (c);

  return c;
}

bson_cursor_t *
bson_cursor_free (bson_cursor_t *c)
{
  if (c)
    free (c);
  return NULL;
}

/** @internal Figure out the block size of a given type.
 *
 * Provided a #bson_type and some raw data, figures out the length of
 * the block, counted from rigth after the element name's position.
 *
 * @param type is the type of object we need the size for.
 * @param data is the raw data (starting right after the element's
 * name).
 *
 * @returns The size of the block, or -1 on error.
 */
static int32_t
_bson_get_block_size (bson_type_t type, const uint8_t *data)
{
  int64_t l;

  switch (type)
    {
    case BSON_TYPE_STRING:
    case BSON_TYPE_JS_CODE:
    case BSON_TYPE_SYMBOL:
      return bson_stream_doc_size (data, 0) + sizeof (int32_t);
    case BSON_TYPE_DOCUMENT:
    case BSON_TYPE_ARRAY:
    case BSON_TYPE_JS_CODE_W_SCOPE:
      return bson_stream_doc_size (data, 0);
    case BSON_TYPE_DOUBLE:
      return sizeof (double);
    case BSON_TYPE_BINARY:
      return bson_stream_doc_size (data, 0) +
	sizeof (int32_t) + sizeof (uint8_t);
    case BSON_TYPE_OID:
      return 12;
    case BSON_TYPE_BOOLEAN:
      return 1;
    case BSON_TYPE_UTC_DATETIME:
    case BSON_TYPE_TIMESTAMP:
    case BSON_TYPE_INT64:
      return sizeof (int64_t);
    case BSON_TYPE_NULL:
      return 0;
    case BSON_TYPE_REGEXP:
      l = strlen((char *)data);
      return l + strlen((char *)(data + l + 1)) + 2;
    case BSON_TYPE_INT32:
      return sizeof (int32_t);
    case BSON_TYPE_DBPOINTER:
      return bson_stream_doc_size (data, 0) + sizeof (int32_t) + 12;
    case BSON_TYPE_NONE:
    default:
      return -1;
    }
}

static inline lmc_bool_t
_bson_cursor_find (const bson_t *b, const char *name, size_t start_pos,
		   int32_t end_pos, lmc_bool_t wrap_over,
		   bson_cursor_t *dest_c)
{
  int32_t pos = start_pos, bs, name_len;
  const uint8_t *d;

  name_len = strlen (name);

  d = bson_data (b);

  if (pos == 0)
    pos = sizeof (int32_t);

  while (pos < end_pos)
    {
      bson_type_t t = (bson_type_t) d[pos];
      const char *key = (char *) &d[pos + 1];
      int32_t key_len = strlen (key);
      int32_t value_pos = pos + key_len + 2;

      if (!memcmp (key, name, (name_len <= key_len) ? name_len : key_len))
	{
	  dest_c->obj = b;
	  dest_c->key = key;
	  dest_c->pos = pos;
	  dest_c->value_pos = value_pos;

	  return TRUE;
	}
      bs = _bson_get_block_size (t, &d[value_pos]);
      if (bs == -1)
	return FALSE;
      pos = value_pos + bs;
    }

  if (wrap_over)
    return _bson_cursor_find (b, name, sizeof (int32_t), start_pos,
			      FALSE, dest_c);

  return FALSE;
}

bson_cursor_t *
bson_cursor_find (bson_cursor_t *c, const char *name)
{
  if (!c || !name)
    lmc_error_raise (c, EINVAL);

  if (_bson_cursor_find (c->obj, name, c->pos, bson_size (c->obj) - 1,
			 TRUE, c))
    return c;
  else
    lmc_error_raise (c, ENOKEY);
}

bson_cursor_t *
bson_cursor_next (bson_cursor_t *c)
{
  const uint8_t *d;
  int32_t pos, bs;

  if (!c)
    {
      errno = EINVAL;
      return NULL;
    }

  d = bson_data (c->obj);

  if (c->pos == 0)
    pos = sizeof (int32_t);
  else
    {
      bs = _bson_get_block_size (bson_cursor_type (c), d + c->value_pos);
      if (bs == -1)
	lmc_error_raise (c, EPROTO);
      pos = c->value_pos + bs;
    }

  if (pos >= bson_size (c->obj) - 1)
    lmc_error_raise (c, ENOKEY);

  c->pos = pos;
  c->key = (char *) &d[c->pos + 1];
  c->value_pos = c->pos + strlen (c->key) + 2;

  return c;
}

bson_type_t
bson_cursor_type (const bson_cursor_t *c)
{
  if (!c || c->pos < sizeof (int32_t))
    return BSON_TYPE_NONE;

  return (bson_type_t)(bson_data (c->obj)[c->pos]);
}

const char *
bson_cursor_type_as_string (const bson_cursor_t *c)
{
  if (!c || c->pos < sizeof (int32_t))
    return NULL;

  return bson_type_as_string (bson_cursor_type (c));
}

const char *
bson_cursor_key (const bson_cursor_t *c)
{
  if (!c)
    return NULL;

  return c->key;
}

/* Cursor getters */

/** @internal Convenience macro to verify a cursor's type.
 *
 * Verifies that the cursor's type is the same as the type requested
 * by the caller, and returns FALSE if there is a mismatch.
 */
#define BSON_CURSOR_CHECK_TYPE(c,type)		\
  if (bson_cursor_type(c) != type)		\
    return FALSE;

lmc_bool_t
bson_cursor_get_string (const bson_cursor_t *c, const char **dest)
{
  if (!dest)
    {
      errno = EINVAL;
      return FALSE;
    }

  BSON_CURSOR_CHECK_TYPE (c, BSON_TYPE_STRING);

  *dest = (char *)(bson_data (c->obj) + c->value_pos + sizeof (int32_t));

  return TRUE;
}

lmc_bool_t
bson_cursor_get_double (const bson_cursor_t *c, double *dest)
{
  if (!dest)
    {
      errno = EINVAL;
      return FALSE;
    }

  BSON_CURSOR_CHECK_TYPE (c, BSON_TYPE_DOUBLE);

  memcpy (dest, bson_data (c->obj) + c->value_pos, sizeof (double));
  *dest = LMC_DOUBLE_FROM_LE (*dest);

  return TRUE;
}

lmc_bool_t
bson_cursor_get_document (const bson_cursor_t *c, bson_t **dest)
{
  bson_t *b;
  int32_t size;

  if (!dest)
    {
      errno = EINVAL;
      return FALSE;
    }

  BSON_CURSOR_CHECK_TYPE (c, BSON_TYPE_DOCUMENT);

  size = bson_stream_doc_size (bson_data(c->obj), c->value_pos) - 1;
  b = bson_finish (bson_new_from_data
		   (bson_data (c->obj) + c->value_pos, size));
  *dest = b;

  return TRUE;
}

lmc_bool_t
bson_cursor_get_array (const bson_cursor_t *c, bson_t **dest)
{
  bson_t *b;
  int32_t size;

  if (!dest)
    {
      errno = EINVAL;
      return FALSE;
    }

  BSON_CURSOR_CHECK_TYPE (c, BSON_TYPE_ARRAY);

  size = bson_stream_doc_size (bson_data(c->obj), c->value_pos) - 1;
  b = bson_finish (bson_new_from_data
		   (bson_data (c->obj) + c->value_pos, size));
  *dest = b;

  return TRUE;
}

lmc_bool_t
bson_cursor_get_binary (const bson_cursor_t *c,
			bson_binary_subtype_t *subtype,
			const uint8_t **data, int32_t *size)
{
  if (!subtype || !data || !size)
    {
      errno = EINVAL;
      return FALSE;
    }

  BSON_CURSOR_CHECK_TYPE (c, BSON_TYPE_BINARY);

  *size = bson_stream_doc_size (bson_data(c->obj), c->value_pos);
  *subtype = (bson_binary_subtype_t)(bson_data (c->obj)[c->value_pos +
							sizeof (int32_t)]);
  *data = (uint8_t *)(bson_data (c->obj) + c->value_pos +
		      sizeof (int32_t) + 1);

  return TRUE;
}

lmc_bool_t
bson_cursor_get_oid (const bson_cursor_t *c, const bson_oid_t **dest)
{
  if (!dest)
    {
      errno = EINVAL;
      return FALSE;
    }

  BSON_CURSOR_CHECK_TYPE (c, BSON_TYPE_OID);

  *dest = (bson_oid_t *)(bson_data (c->obj) + c->value_pos);

  return TRUE;
}

lmc_bool_t
bson_cursor_get_boolean (const bson_cursor_t *c, lmc_bool_t *dest)
{
  if (!dest)
    {
      errno = EINVAL;
      return FALSE;
    }

  BSON_CURSOR_CHECK_TYPE (c, BSON_TYPE_BOOLEAN);

  *dest = (lmc_bool_t)(bson_data (c->obj) + c->value_pos)[0];

  return TRUE;
}

lmc_bool_t
bson_cursor_get_utc_datetime (const bson_cursor_t *c,
			      int64_t *dest)
{
  if (!dest)
    {
      errno = EINVAL;
      return FALSE;
    }

  BSON_CURSOR_CHECK_TYPE (c, BSON_TYPE_UTC_DATETIME);

  memcpy (dest, bson_data (c->obj) + c->value_pos, sizeof (int64_t));
  *dest = LMC_INT64_FROM_LE (*dest);

  return TRUE;
}

lmc_bool_t
bson_cursor_get_regex (const bson_cursor_t *c, const char **regex,
		       const char **options)
{
  if (!regex || !options)
    {
      errno = EINVAL;
      return FALSE;
    }

  BSON_CURSOR_CHECK_TYPE (c, BSON_TYPE_REGEXP);

  *regex = (char *)(bson_data (c->obj) + c->value_pos);
  *options = (char *)(*regex + strlen(*regex) + 1);

  return TRUE;
}

lmc_bool_t
bson_cursor_get_javascript (const bson_cursor_t *c, const char **dest)
{
  if (!dest)
    {
      errno = EINVAL;
      return FALSE;
    }

  BSON_CURSOR_CHECK_TYPE (c, BSON_TYPE_JS_CODE);

  *dest = (char *)(bson_data (c->obj) + c->value_pos + sizeof (int32_t));

  return TRUE;
}

lmc_bool_t
bson_cursor_get_symbol (const bson_cursor_t *c, const char **dest)
{
  if (!dest)
    {
      errno = EINVAL;
      return FALSE;
    }

  BSON_CURSOR_CHECK_TYPE (c, BSON_TYPE_SYMBOL);

  *dest = (char *)(bson_data (c->obj) + c->value_pos + sizeof (int32_t));

  return TRUE;
}

lmc_bool_t
bson_cursor_get_javascript_w_scope (const bson_cursor_t *c,
				    const char **js,
				    bson_t **scope)
{
  bson_t *b;
  int32_t size, docpos;

  if (!js || !scope)
    {
      errno = EINVAL;
      return FALSE;
    }

  BSON_CURSOR_CHECK_TYPE (c, BSON_TYPE_JS_CODE_W_SCOPE);

  docpos = bson_stream_doc_size (bson_data (c->obj),
				 c->value_pos + sizeof (int32_t)) +
    sizeof (int32_t) * 2;
  size = bson_stream_doc_size (bson_data (c->obj), c->value_pos + docpos) - 1;

  b = bson_finish (bson_new_from_data
		   (bson_data (c->obj) + c->value_pos + docpos, size));
  *scope = b;
  *js = (char *)(bson_data (c->obj) + c->value_pos + sizeof (int32_t) * 2);

  return TRUE;
}

lmc_bool_t
bson_cursor_get_int32 (const bson_cursor_t *c, int32_t *dest)
{
  if (!dest)
    {
      errno = EINVAL;
      return FALSE;
    }

  BSON_CURSOR_CHECK_TYPE (c, BSON_TYPE_INT32);

  memcpy (dest, bson_data (c->obj) + c->value_pos, sizeof (int32_t));
  *dest = LMC_INT32_FROM_LE (*dest);

  return TRUE;
}

lmc_bool_t
bson_cursor_get_timestamp (const bson_cursor_t *c, bson_timestamp_t *dest)
{
  if (!dest)
    {
      errno = EINVAL;
      return FALSE;
    }

  BSON_CURSOR_CHECK_TYPE (c, BSON_TYPE_TIMESTAMP);

  memcpy (dest, bson_data (c->obj) + c->value_pos, sizeof (int64_t));
  *dest = LMC_INT64_FROM_LE (*dest);

  return TRUE;
}

lmc_bool_t
bson_cursor_get_int64 (const bson_cursor_t *c, int64_t *dest)
{
  if (!dest)
    {
      errno = EINVAL;
      return FALSE;
    }

  BSON_CURSOR_CHECK_TYPE (c, BSON_TYPE_INT64);

  memcpy (dest, bson_data (c->obj) + c->value_pos, sizeof (int64_t));
  *dest = LMC_INT64_FROM_LE (*dest);

  return TRUE;
}
