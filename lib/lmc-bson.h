/* lmc-bson.h - libmongo-client's BSON implementation, public headers
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

/** @file lib/lmc-bson.h
 * The BSON API's public header.
 */

#ifndef LMC_BSON_H
#define LMC_BSON_H 1

#include <lmc-common.h>
#include <lmc-endian.h>

#include <inttypes.h>
#include <string.h>

LMC_BEGIN_DECLS

/** @defgroup bson_mod BSON
 *
 * The types, functions and everything else within this module is
 * meant to allow one to work with BSON objects easily.
 *
 * @addtogroup bson_mod
 * @{
 */

/** @defgroup bson_types Types
 *
 * @addtogroup bson_types
 * @{
 */

/** An opaque BSON object.
 * A BSON object represents a full BSON document, as specified at
 * http://bsonspec.org/.
 *
 * Each object has two states: open and finished. While the document
 * is open, it can be appended to, but it cannot be read from. While
 * it is finished, it can be read from, and iterated over, but cannot
 * be appended to.
 */
typedef struct _lmc_bson_t bson_t;

/** Opaque BSON cursor.
 * Cursors are used to represent a single entry within a BSON object,
 * and to help iterating over said document.
 */
typedef struct _lmc_bson_cursor_t bson_cursor_t;

/** Supported BSON object types.
 */
typedef enum
{
  BSON_TYPE_NONE = 0, /**< Only used for errors */
  BSON_TYPE_DOUBLE = 0x01, /**< 8byte double */
  BSON_TYPE_STRING, /**< 4byte length + NULL terminated string */
  BSON_TYPE_DOCUMENT, /**< 4byte length + NULL terminated document */
  BSON_TYPE_ARRAY, /**< 4byte length + NULL terminated document */
  BSON_TYPE_BINARY, /**< 4byte length + 1byte subtype + data */
  BSON_TYPE_UNDEFINED, /**< Deprecated type. */
  BSON_TYPE_OID, /**< 12byte ObjectID */
  BSON_TYPE_BOOLEAN, /**< 1byte boolean value */
  BSON_TYPE_UTC_DATETIME, /**< 8byte timestamp; milliseconds since
			     Unix epoch */
  BSON_TYPE_NULL, /**< NULL value, No following data. */
  BSON_TYPE_REGEXP, /**< Two NULL terminated C strings, the regex
		       itself, and the options. */
  BSON_TYPE_DBPOINTER, /**< Deprecated type. */
  BSON_TYPE_JS_CODE, /**< 4byte length + NULL terminated string */
  BSON_TYPE_SYMBOL, /**< 4byte length + NULL terminated string */
  BSON_TYPE_JS_CODE_W_SCOPE, /**< 4byte length, followed by a string
				and a document */
  BSON_TYPE_INT32, /**< 4byte integer */
  BSON_TYPE_TIMESTAMP, /**< 4bytes increment + 4bytes timestamp */
  BSON_TYPE_INT64, /**< 8byte integer */
  BSON_TYPE_MIN = 0xff,
  BSON_TYPE_MAX = 0x7f
} bson_type_t;

/** Return a type's stringified name.
 *
 * @param type is the type to stringify.
 *
 * @returns The stringified type, or NULL on error.
 */
const char *bson_type_as_string (bson_type_t type);

/** Supported BSON binary subtypes.
 */
typedef enum
{
  BSON_BINARY_SUBTYPE_GENERIC = 0x00, /**< The Generic subtype, the
					 default. */
  BSON_BINARY_SUBTYPE_FUNCTION = 0x01, /**< Binary representation of a
					  function. */
  BSON_BINARY_SUBTYPE_BINARY = 0x02, /**< Obsolete, do not use. */
  BSON_BINARY_SUBTYPE_UUID = 0x03, /**< Binary representation of an
				      UUID. */
  BSON_BINARY_SUBTYPE_MD5 = 0x05, /**< Binary representation of an MD5
				     sum. */
  BSON_BINARY_SUBTYPE_USER_DEFINED = 0x80 /**< User defined data,
					     nothing's known about the
					     structure. */
} bson_binary_subtype_t;

/** Verification contexts.
 *
 * These values can be OR'd together and used with
 * bson_validate_key(), to validate certain aspects of a BSON key.
 */
typedef enum
{
  BSON_VALID_CONTEXT_ALL = 0, /**< The key should always be valid. */
  BSON_VALID_CONTEXT_NO_DOTS = 1 << 1, /**< The key should contain no
					  dots. */
  BSON_VALID_CONTEXT_NO_DOLLAR = 1 << 2, /**< The key should not
					    start with a dollar sign. */
} bson_valid_context_t;

/** BSON ObjectID type.
 *
 * This union offers various alternate representations of an ObjectID
 * type.
 */
#pragma pack(1)
typedef union
{
  uint8_t bytes[12]; /**< The ObjectID represented as 12 bytes. */
  uint32_t ints[3]; /**< The ObjectID represented as 3 32-bit
		       numbers. */
} bson_oid_t;
#pragma pack()

/** BSON Timestamp type.
 */
typedef struct
{
  int32_t i; /**< 32-bit increment. */
  uint32_t t; /**< 32-bit timestamp, since the epoch. */
} bson_timestamp_t;

/** @} */

/** @defgroup bson_object_access Object Access
 *
 * Functions that operate on whole BSON objects.
 *
 * @addtogroup bson_object_access
 * @{
 */

/** Create a new BSON object.
 *
 * @note The created object will have no memory pre-allocated for data,
 * resulting in possibly more reallocations than neccessary when
 * appending elements.
 *
 * @note If at all possible, use bson_new_sized() instead.
 *
 * @returns A newly allocated object, or NULL on error.
 */
bson_t *bson_new (void);

/** Create a new BSON object, preallocating a given amount of space.
 *
 * Creates a new BSON object, pre-allocating @a size bytes of space
 * for the data.
 *
 * @param size is the space to pre-allocate for data.
 *
 * @note It is not an error to pre-allocate either less, or more space
 * than what will really end up being added. Pre-allocation does not
 * set the size of the final object, it is merely a hint, a way to
 * help the system avoid memory reallocations.
 *
 * @returns A newly allocated object, or NULL on error.
 */
bson_t *bson_new_sized (uint32_t size);

/** Create a BSON object from existing data.
 *
 * In order to be able to parse existing BSON, one must load it up
 * into a bson object - and this function does just that.
 *
 * @note Objects created by this function are not final objects, in
 * order to be able to extend them. As such, when importing existing
 * BSON data, which are terminated by a zero byte, specify the size as
 * one smaller than the original data stream.
 *
 * @note This is because bson_finish() will append a zero byte, thus
 * one would end up with an invalid document if it had an extra one.
 *
 * @param data is the BSON byte stream to import.
 * @param size is the size of the byte stream.
 *
 * @returns A newly allocated object, with a copy of @a data as its
 * contents.
 */
bson_t *bson_new_from_data (const uint8_t *data, int32_t size);

/** Finish a BSON object.
 *
 * Terminate a BSON object. This includes appending the trailing zero
 * byte and finalising the length of the object.
 *
 * The object cannot be appended to after it is finalised.
 *
 * @param b is the BSON object to close & finish.
 *
 * @returns The finished object, with its error status set
 * appropriately.
 */
bson_t *bson_finish (bson_t *b);

/** Reset a BSON object.
 *
 * Resetting a BSON object clears the finished status, and sets its
 * size to zero. Resetting is most useful when wants to keep the
 * already allocated memory around for reuse.
 *
 * @param b is the BSON object to reset.
 *
 * @returns The reseted object, with its error status set
 * appropriately.
 */
bson_t *bson_reset (bson_t *b);

/** Free the memory associated with a BSON object.
 *
 * Frees up all memory associated with a BSON object. The variable
 * shall not be used afterwards.
 *
 * @param b is the BSON object to free.
 *
 * @returns NULL.
 */
bson_t *bson_free (bson_t *b);

/** Return the size of a finished BSON object.
 *
 * @param b is the finished BSON object.
 *
 * @returns The size of the document, or -1 on error.
 *
 * @note Trying to get the size of a BSON object that has not been
 * closed by bson_finish() is considered an error.
 */
int32_t bson_size (const bson_t *b);

/** Return the raw bytestream form of the BSON object.
 *
 * @param b is the BSON object to retrieve data from.
 *
 * @returns The raw datastream or NULL on error. The stream s all not
 * be freed.
 *
 * @note Trying to retrieve the data of an unfinished BSON object is
 * considered an error.
 */
const uint8_t *bson_data (const bson_t *b);

/** Validate a BSON key.
 *
 * Verifies that a given key is a valid BSON field name. Depending on
 * context (togglable by the boolean flags) this means that the string
 * must either be free of dots, or must not start with a dollar sign.
 *
 * @param key is the field name to validate.
 * @param flags is the combination of properties to validate.
 *
 * @returns TRUE if the field name is found to be valid, FALSE
 * otherwise.
 *
 * @note This function does NOT do UTF-8 validation. That is left up
 * to the application.
 */
lmc_bool_t bson_validate_key (const char *key,
			      bson_valid_context_t flags);

/** Reads out the 32-bit documents size from a BSON bytestream.
 *
 * This function can be used when reading data from a stream, and one
 * wants to build a BSON object from the bytestream: for
 * bson_new_from_data(), one needs the length. This function provides
 * that.
 *
 * @param doc is the byte stream to check the size of.
 * @param pos is the position in the bytestream to start reading at.
 *
 * @returns The size of the document at the appropriate position.
 *
 * @note The byte stream is expected to be in little-endian byte
 * order.
 */
static __inline__ int32_t bson_stream_doc_size (const uint8_t *doc,
						int32_t pos)
{
  int32_t size;

  memcpy (&size, doc + pos, sizeof (int32_t));
  return LMC_INT32_FROM_LE (size);
}

/** @} */

/** @defgroup bson_append Appending
 *
 * @brief Functions to append various kinds of elements to existing
 * BSON objects.
 *
 * Every such function expects the BSON object to be open, and not in
 * an error state. All of them will return a BSON object aswell (or
 * NULL, if they were given a NULL object too).
 *
 * The resulting object will have it's error flag set (see
 * lmc_error_isok()) upon error, and not touched otherwise.
 *
 * If any of the append functions encounter a BSON object that has the
 * error flag set, they will do nothing, and return the original
 * object instead. This way, errors propagate up to the appropriate
 * level, even if the functions were chained.
 *
 * The only way to append to a finished BSON object is to @a clone it
 * with bson_new_from_data(), and append to the newly created object.
 *
 * @addtogroup bson_append
 * @{
 */

/** Append a string to a BSON object.
 *
 * @param b is the BSON object to append to.
 * @param name is the key name.
 * @param val is the value to append.
 * @param length is the length of value. Use @a -1 to use the full
 * string supplied as @a name.
 *
 * @returns The BSON object, with the new field appended, or the
 * original, with errors set appropriately.
 */
bson_t *bson_append_string (bson_t *b, const char *name, const char *val,
			    int32_t length);

/** Append a double to a BSON object.
 *
 * @param b is the BSON object to append to.
 * @param name is the key name.
 * @param d is the double value to append.
 *
 * @returns The BSON object, with the new field appended, or the
 * original, with errors set appropriately.
 */
bson_t *bson_append_double (bson_t *b, const char *name, double d);

/** Append a BSON document to a BSON object.
 *
 * @param b is the BSON object to append to.
 * @param name is the key name.
 * @param doc is the BSON document to append.
 *
 * @note @a doc MUST be a finished BSON document.
 *
 * @returns The BSON object, with the new field appended, or the
 * original, with errors set appropriately.
 */
bson_t *bson_append_document (bson_t *b, const char *name, const bson_t *doc);

/** Append a BSON array to a BSON object.
 *
 * @param b is the BSON object to append to.
 * @param name is the key name.
 * @param array is the BSON array to append.
 *
 * @note @a array MUST be a finished BSON document.
 *
 * @note The difference between plain documents and arrays - as far as
 * this library is concerned, and apart from the type - is that array
 * keys must be numbers in increasing order. However, no extra care is
 * taken to verify that: it is the responsibility of the caller to set
 * the array up appropriately.
 *
 * @returns The BSON object, with the new field appended, or the
 * original, with errors set appropriately.
 */
bson_t *bson_append_array (bson_t *b, const char *name, const bson_t *array);

/** Append a BSON binary blob to a BSON object.
 *
 * @param b is the BSON object to append to.
 * @param name is the key name.
 * @param subtype is the BSON binary subtype to use.
 * @param data is a pointer to the blob data.
 * @param size is the size of the blob.
 *
 * @returns The BSON object, with the new field appended, or the
 * original, with errors set appropriately.
 */
bson_t *bson_append_binary (bson_t *b, const char *name,
			    bson_binary_subtype_t subtype,
			    const uint8_t *data, int32_t size);

/** Append an ObjectID to a BSON object.
 *
 * ObjectIDs are 12 byte values, the first four being a timestamp in
 * big endian byte order, the next three a machine ID, then two bytes
 * for the PID, and finally three bytes of sequence number, in big
 * endian byte order again.
 *
 * @param b is the BSON object to append to.
 * @param name is the key name.
 * @param oid is the ObjectID to append.
 *
 * @note The OID must be 12 bytes long, and formatting it
 * appropriately is the responsiblity of the caller.
 *
 * @returns The BSON object, with the new field appended, or the
 * original, with errors set appropriately.
 */
bson_t *bson_append_oid (bson_t *b, const char *name, const bson_oid_t *oid);

/** Append a boolean to a BSON object.
 *
 * @param b is the BSON object to append to.
 * @param name is the key name.
 * @param value is the boolean value to append.
 *
 * @returns The BSON object, with the new field appended, or the
 * original, with errors set appropriately.
 */
bson_t *bson_append_boolean (bson_t *b, const char *name, lmc_bool_t value);

/** Append an UTC datetime to a BSON object.
 *
 * @param b is the BSON object to append to.
 * @param name is the key name.
 * @param ts is the UTC timestamp: the number of milliseconds since
 * the Unix epoch.
 *
 * @returns The BSON object, with the new field appended, or the
 * original, with errors set appropriately.
 */
bson_t *bson_append_utc_datetime (bson_t *b, const char *name, int64_t ts);

/** Append a NULL value to a BSON object.
 *
 * @param b is the BSON object to append to.
 * @param name is the key name.
 *
 * @returns The BSON object, with the new field appended, or the
 * original, with errors set appropriately.
 */
bson_t *bson_append_null (bson_t *b, const char *name);

/** Append a regexp object to a BSON object.
 *
 * @param b is the BSON object to append to.
 * @param name is the key name.
 * @param regexp is the regexp string itself.
 * @param options represents the regexp options, serialised to a
 * string.
 *
 * @returns The BSON object, with the new field appended, or the
 * original, with errors set appropriately.
 */
bson_t *bson_append_regex (bson_t *b, const char *name, const char *regexp,
			   const char *options);

/** Append Javascript code to a BSON object.
 *
 * @param b is the BSON object to append to.
 * @param name is the key name.
 * @param js is the javascript code as a C string.
 * @param len is the length of the code, use @a -1 to use the full
 * length of the string supplised in @a js.
 *
 * @returns The BSON object, with the new field appended, or the
 * original, with errors set appropriately.
 */
bson_t *bson_append_javascript (bson_t *b, const char *name, const char *js,
				int32_t len);

/** Append a symbol to a BSON object.
 *
 * @param b is the BSON object to append to.
 * @param name is the key name.
 * @param symbol is the symbol to append.
 * @param len is the length of the code, use @a -1 to use the full
 * length of the string supplised in @a symbol.
 *
 * @returns The BSON object, with the new field appended, or the
 * original, with errors set appropriately.
 */
bson_t *bson_append_symbol (bson_t *b, const char *name, const char *symbol,
			    int32_t len);

/** Append Javascript code (with scope) to a BSON object.
 *
 * @param b is the BSON object to append to.
 * @param name is the key name.
 * @param js is the javascript code as a C string.
 * @param len is the length of the code, use @a -1 to use the full
 * length of the string supplied in @a js.
 * @param scope is scope to evaluate the javascript code in.
 *
 * @returns The BSON object, with the new field appended, or the
 * original, with errors set appropriately.
 */
bson_t *bson_append_javascript_w_scope (bson_t *b, const char *name,
					const char *js, int32_t len,
					const bson_t *scope);

/** Append a 32-bit integer to a BSON object.
 *
 * @param b is the BSON object to append to.
 * @param name is the key name.
 * @param i is the integer to append.
 *
 * @returns The BSON object, with the new field appended, or the
 * original, with errors set appropriately.
 */
bson_t *bson_append_int32 (bson_t *b, const char *name, int32_t i);

/** Append a timestamp to a BSON object.
 *
 * @param b is the BSON object to append to.
 * @param name is the key name.
 * @param ts is the timestamp to append.
 *
 * @returns The BSON object, with the new field appended, or the
 * original, with errors set appropriately.
 */
bson_t *bson_append_timestamp (bson_t *b, const char *name,
			       bson_timestamp_t *ts);

/** Append a 64-bit integer to a BSON object.
 *
 * @param b is the BSON object to append to.
 * @param name is the key name.
 * @param i is the integer to append.
 *
 * @returns The BSON object, with the new field appended, or the
 * original, with errors set appropriately.
 */
bson_t *bson_append_int64 (bson_t *b, const char *name, int64_t i);

/** @} */

/** @defgroup bson_cursor Cursor & Retrieval
 *
 * This section documents the cursors, and the data retrieval
 * functions. Each and every function here operates on finished BSON
 * objects, and will return with an error if passed an open object.
 *
 * Data can be retrieved from cursors, which in turn point to a
 * specific part of the BSON object.
 *
 * The idea is to place the cursor to the appropriate key first, then
 * retrieve the data stored there. Trying to retrieve data that is of
 * different type than what the cursor is results in an error.
 *
 * Functions to iterate to the next key, and retrieve the current
 * keys name are also provided.
 *
 * @addtogroup bson_cursor
 * @{
 */

/** Create a new cursor.
 *
 * Creates a new cursor, and positions it to the beginning of the
 * supplied BSON object.
 *
 * @param b is the BSON object to create a cursor for.
 *
 * @returns A newly allocated cursor, or NULL on error.
 */
bson_cursor_t *bson_cursor_new (const bson_t *b);

/** Delete a cursor, and free up all resources used by it.
 *
 * @param c is the cursor to free.
 *
 * @returns NULL.
 */
bson_cursor_t *bson_cursor_free (bson_cursor_t *c);

/** Move the cursor to a given key
 *
 * Starts canning the BSON object for the given key from the current
 * position, wrapping around if neccessary.
 *
 * @param c is the cursor to move.
 * @param name is the key name to position to.
 *
 * @returns The cursor positioned to the found key on success, NULL
 * otherwise.
 */
bson_cursor_t *bson_cursor_find (bson_cursor_t *c, const char *name);

/** Position the cursor to the next key.
 *
 * @param c is the cursor to move forward.
 *
 * @returns The cursor positioned to the next key, if any, NULL
 * otherwise.
 */
bson_cursor_t *bson_cursor_next (bson_cursor_t *c);

/** Determine the type of the current element.
 *
 * @param c is the cursor pointing at the appropriate element.
 *
 * @returns The type of the element, or #BSON_TYPE_NONE on error.
 */
bson_type_t bson_cursor_type (const bson_cursor_t *c);

/** Retrieve the type of the current element, as string.
 *
 * @param c is the cursor pointing at the appropriate element.
 *
 * @returns The type of the element, as string, or NULL on error.
 *
 * @note The string points to an internal structure, it should not be
 * freed or modified.
 */
const char *bson_cursor_type_as_string (const bson_cursor_t *c);

/** Determine the name of the current elements key.
 *
 * @param c is the cursor pointing at the appropriate element.
 *
 * @returns The name of the key, or NULL on error.
 *
 * @note The name is a pointer to an internal string, one must NOT
 * free it.
 */
const char *bson_cursor_key (const bson_cursor_t *c);

#if 0
/** Get the value stored at the cursor, as string.
 *
 * @param c is the cursor pointing at the appropriate element.
 * @param dest is a pointer to a variable where the value can be
 * stored.
 *
 * @note The @a dest pointer will be set to point to an internal
 * structure, and must not be freed or modified by the caller.
 *
 * @returns TRUE on success, FALSE otherwise.
 */
gboolean bson_cursor_get_string (const bson_cursor *c, const gchar **dest);

/** Get the value stored at the cursor, as a double.
 *
 * @param c is the cursor pointing at the appropriate element.
 * @param dest is a pointer to a variable where the value can be
 * stored.
 *
 * @returns TRUE on success, FALSE otherwise.
 */
gboolean bson_cursor_get_double (const bson_cursor *c, gdouble *dest);

/** Get the value stored at the cursor, as a BSON document.
 *
 * @param c is the cursor pointing at the appropriate element.
 * @param dest is a pointer to a variable where the value can be
 * stored.
 *
 * @note The @a dest pointer will be a newly allocated, finished
 * object: it is the responsibility of the caller to free it.
 *
 * @returns TRUE on success, FALSE otherwise.
 */
gboolean bson_cursor_get_document (const bson_cursor *c, bson_t **dest);

/** Get the value stored at the cursor, as a BSON array.
 *
 * @param c is the cursor pointing at the appropriate element.
 * @param dest is a pointer to a variable where the value can be
 * stored.
 *
 * @note The @a dest pointer will be a newly allocated, finished
 * object: it is the responsibility of the caller to free it.
 *
 * @returns TRUE on success, FALSE otherwise.
 */
gboolean bson_cursor_get_array (const bson_cursor *c, bson_t **dest);

/** Get the value stored at the cursor, as binary data.
 *
 * @param c is the cursor pointing at the appropriate element.
 * @param subtype is a pointer to store the binary subtype at.
 * @param data is a pointer to where the data shall be stored.
 * @param size is a pointer to store the size at.
 *
 * @note The @a data pointer will be pointing to an internal
 * structure, it must not be freed or modified.
 *
 * @returns TRUE on success, FALSE otherwise.
 */
gboolean bson_cursor_get_binary (const bson_cursor *c,
				 bson_binary_subtype *subtype,
				 const guint8 **data, gint32 *size);

/** Get the value stored at the cursor, as an ObjectID.
 *
 * @param c is the cursor pointing at the appropriate element.
 * @param dest is a pointer to a variable where the value can be
 * stored.
 *
 * @note The @a dest pointer will be set to point to an internal
 * structure, and must not be freed or modified by the caller.
 *
 * @returns TRUE on success, FALSE otherwise.
 */
gboolean bson_cursor_get_oid (const bson_cursor *c, const guint8 **dest);

/** Get the value stored at the cursor, as a boolean.
 *
 * @param c is the cursor pointing at the appropriate element.
 * @param dest is a pointer to a variable where the value can be
 * stored.
 *
 * @returns TRUE on success, FALSE otherwise.
 */
gboolean bson_cursor_get_boolean (const bson_cursor *c, gboolean *dest);

/** Get the value stored at the cursor, as an UTC datetime.
 *
 * @param c is the cursor pointing at the appropriate element.
 * @param dest is a pointer to a variable where the value can be
 * stored.
 *
 * @returns TRUE on success, FALSE otherwise.
 */
gboolean bson_cursor_get_utc_datetime (const bson_cursor *c, gint64 *dest);

/** Get the value stored at the cursor, as a regexp.
 *
 * @param c is the cursor pointing at the appropriate element.
 * @param regex is a pointer to a variable where the regex can be
 * stored.
 * @param options is a pointer to a variable where the options can be
 * stored.
 *
 * @note Both the @a regex and @a options pointers will be set to
 * point to an internal structure, and must not be freed or modified
 * by the caller.
 *
 * @returns TRUE on success, FALSE otherwise.
 */
gboolean bson_cursor_get_regex (const bson_cursor *c, const gchar **regex,
				const gchar **options);

/** Get the value stored at the cursor, as javascript code.
 *
 * @param c is the cursor pointing at the appropriate element.
 * @param dest is a pointer to a variable where the value can be
 * stored.
 *
 * @note The @a dest pointer will be set to point to an internal
 * structure, and must not be freed or modified by the caller.
 *
 * @returns TRUE on success, FALSE otherwise.
 */
gboolean bson_cursor_get_javascript (const bson_cursor *c, const gchar **dest);

/** Get the value stored at the cursor, as a symbol.
 *
 * @param c is the cursor pointing at the appropriate element.
 * @param dest is a pointer to a variable where the value can be
 * stored.
 *
 * @note The @a dest pointer will be set to point to an internal
 * structure, and must not be freed or modified by the caller.
 *
 * @returns TRUE on success, FALSE otherwise.
 */
gboolean bson_cursor_get_symbol (const bson_cursor *c, const gchar **dest);

/** Get the value stored at the cursor, as javascript code w/ scope.
 *
 * @param c is the cursor pointing at the appropriate element.
 * @param js is a pointer to a variable where the javascript code can
 * be stored.
 * @param scope is a pointer to a variable where the scope can be
 * stored.
 *
 * @note The @a scope pointer will be a newly allocated, finished
 * BSON object: it is the responsibility of the caller to free it.
 *
 * @returns TRUE on success, FALSE otherwise.
 */
gboolean bson_cursor_get_javascript_w_scope (const bson_cursor *c,
					     const gchar **js,
					     bson_t **scope);

/** Get the value stored at the cursor, as a 32-bit integer.
 *
 * @param c is the cursor pointing at the appropriate element.
 * @param dest is a pointer to a variable where the value can be
 * stored.
 *
 * @returns TRUE on success, FALSE otherwise.
 */
gboolean bson_cursor_get_int32 (const bson_cursor *c, gint32 *dest);

/** Get the value stored at the cursor, as a timestamp.
 *
 * @param c is the cursor pointing at the appropriate element.
 * @param dest is a pointer to a variable where the value can be
 * stored.
 *
 * @returns TRUE on success, FALSE otherwise.
 */
gboolean bson_cursor_get_timestamp (const bson_cursor *c, gint64 *dest);

/** Get the value stored at the cursor, as a 64-bit integer.
 *
 * @param c is the cursor pointing at the appropriate element.
 * @param dest is a pointer to a variable where the value can be
 * stored.
 *
 * @returns TRUE on success, FALSE otherwise.
 */
gboolean bson_cursor_get_int64 (const bson_cursor *c, gint64 *dest);

/** @} */

#endif

/** @} */



LMC_END_DECLS

#endif
