/* lmc/bson-element.h - BSON element API
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

/** @file lib/lmc/bson-element.h
 */

#ifndef LMC_BSON_ELEMENT_H
#define LMC_BSON_ELEMENT_H

#include <lmc/common.h>

LMC_BEGIN_DECLS

/** @addtogroup lmc_bson
 * @{
 *  @defgroup lmc_bson_element Elements
 *  @addtogroup lmc_bson_element
 *  @{
 */

/** @defgroup lmc_bson_element_core Core
 * @addtogroup lmc_bson_element_core
 * @{
 */

/** A BSON element container.
 *
 * This type can hold any type of BSON element.
 */
typedef struct _bson_element_t bson_element_t;

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
} bson_element_type_t;

/** Create a new BSON element.
 *
 * Creates a new BSON element, without a type assigned, and a
 * reference count of one.
 *
 * @returns A newly allocated element.
 */
bson_element_t *bson_element_new (void);

/** Create a BSON element, with preallocated space.
 *
 * @param size is the initial size of the element.
 *
 * @returns A newly allocated element.
 */
bson_element_t *bson_element_new_sized (uint32_t size);

/** Create a new BSON element.
 *
 * @param name is the name of the element.
 * @param type is the BSON type to use.
 *
 * The rest of the parameters depend on the @a type. Whatever the
 * types setter takes, must be passed along here.
 *
 * @returns A new BSON element with its name, type and value set.
 */
bson_element_t *bson_element_create (const char *name,
				     bson_element_type_t type, ...);

/** Create a new BSON element from raw data.
 *
 * @param data is the raw BSON element data.
 *
 * @returns A new BSON element, with its properties copied from the
 * raw data.
 */
bson_element_t *bson_element_new_from_data (const uint8_t *data);

/** Increase the reference count of a BSON element.
 *
 * @param e is the element to increase the refcount of.
 *
 * @returns The BSON element itself.
 */
bson_element_t *bson_element_ref (bson_element_t *e);

/** Decrease the reference count of a BSON element.
 *
 * Whenever the reference count reaches zero, the object will be freed
 * up.
 *
 * @param e is the BSON element to decrease the refcount of.
 *
 * @returns The BSON element, or NULL if it was freed, or an error
 * occurred.
 */
bson_element_t *bson_element_unref (bson_element_t *e);

/** Get the type of a BSON element.
 *
 * @param e is the BSON element to check the type of.
 *
 * @returns The type, or #BSON_TYPE_NONE on error.
 */
bson_element_type_t bson_element_type_get (bson_element_t *e);

/** Set the type of a BSON element.
 *
 * @param e is the BSON element to set the type of.
 * @param type is the type to set.
 *
 * @returns The BSON element with the type set, or NULL on error.
 */
bson_element_t *bson_element_type_set (bson_element_t *e,
				       bson_element_type_t type);

/** Get the name of a BSON element.
 *
 * @param e is the BSON element whose name to retrieve.
 *
 * @returns The name of the element, or NULL.
 *
 * @note The returned string points to an internal structure, and must
 * not be modified! It's only valid as long as the element is.
 *
 * @note Also note that an unnamed element can be identified by an
 * empty string, not NULL!
 */
const char *bson_element_name_get (bson_element_t *e);

/** Set the name of a BSON element.
 *
 * @param e is the element whose name to set.
 * @param name is the name to set. If it is NULL, the element's name
 * will be cleared instead.
 *
 * @returns The BSON element with the name set, or NULL on error.
 */
bson_element_t *bson_element_name_set (bson_element_t *e,
				       const char *name);

/** Element name validity flags.
 *
 * These flags can be OR'd together when used with
 * bson_element_name_validate(), to form any desirable combination.
 */
enum
{
  /** Unless another flag is set, no check is performed on the name. */
  BSON_ELEMENT_NAME_OK = 0,
  /** Forbid dots anywhere in the name. */
  BSON_ELEMENT_NAME_FORBID_DOTS = 1 << 1,
  /** Forbid dollar signs at the start of the name. */
  BSON_ELEMENT_NAME_FORBID_DOLLAR = 1 << 2,
  /** A combination of all other flags, for convenience. */
  BSON_ELEMENT_NAME_STRICT =
    BSON_ELEMENT_NAME_FORBID_DOTS | BSON_ELEMENT_NAME_FORBID_DOLLAR};

/** Validate an element name, based on flags.
 *
 * @param name is the name to validate.
 * @param flags is a combination of any of the #BSON_ELEMENT_NAME_OK,
 * #BSON_ELEMENT_NAME_FORBID_DOTS, #BSON_ELEMENT_NAME_FORBID_DOLLAR
 * and #BSON_ELEMENT_NAME_STRICT flags.
 *
 * @returns TRUE if the name is valid, FALSE otherwise.
 */
lmc_bool_t bson_element_name_validate (const char *name, int flags);

/** @} */

/** @defgroup lmc_bson_element_data Data Access
 * @addtogroup lmc_bson_element_data
 * @{
 */

/** Get the raw data part of a BSON element.
 *
 * The data part of the BSON object contains only the data, neither
 * the type, nor the name is included.
 *
 * @param e is the element whose data part we want to retrieve.
 *
 * @returns The data part, or NULL on error.
 */
const uint8_t *bson_element_data_get (bson_element_t *e);

/** Get the size of the raw data part of a BSON element.
 *
 * @param e is the element whose data part's size we're interested in.
 *
 * @returns The size of the data part only, or -1 on error.
 */
int32_t bson_element_data_get_size (bson_element_t *e);

/** Reset the raw data part of a BSON element.
 *
 * @param e is the element whose data to truncate.
 *
 * @returns The element with its data part truncated.
 */
bson_element_t *bson_element_data_reset (bson_element_t *e);

/** Append to the raw data part of a BSON element.
 *
 * @param e is the element whose data to append to.
 * @param data is the data to append to the element.
 * @param size is the size of the data.
 *
 * @returns The element with its data part appended to.
 */
bson_element_t *bson_element_data_append (bson_element_t *e,
					  const uint8_t *data,
					  uint32_t size);

/** Set the raw data part of a BSON element.
 *
 * @param e is the element whose data to set.
 * @param data is the data to copy into the element.
 * @param size is the size of the data.
 *
 * If @a data is NULL, or @a size is zero, the data part of the
 * element is cleared.
 *
 * @returns The element with its data part set.
 */
bson_element_t *bson_element_data_set (bson_element_t *e, const uint8_t *data,
				       uint32_t size);

/** @} */

/** @defgroup lmc_bson_element_stream Stream
 * @addtogroup lmc_bson_element_stream
 * @{
 */

/** Get the raw bytestream of the full BSON element.
 *
 * @param e is the BSON element to get the bytestream from.
 *
 * @returns A pointer to the beginning of the bytestream, or NULL on
 * error.
 *
 * @note The returned pointer is owned by the BSON element object, and
 * is only valid as long as the element is. It must not be freed or
 * modified!
 */
const uint8_t *bson_element_stream_get (bson_element_t *e);

/** Get the size of a BSON element's raw bytestream.
 *
 * @param e is the BSON element to get the size of.
 *
 * @returns The lenght of the bytestream, or -1 on error.
 */
int32_t bson_element_stream_get_size (bson_element_t *e);

/** @} */

/** @defgroup lmc_bson_element_accessors Accessors
 * @addtogroup lmc_bson_element_accessors
 * @{
 */

/** Convenience macro for automatic length signalling.
 *
 * When an element's length should be computed automatically (not
 * available for every type, mostly only to string-like types), this
 * macro can be used to signal this fact, in a readable way.
 */
#define BSON_LENGTH_AUTO -1

/** Set the value of a BSON element to a double.
 *
 * @param e is the element whose value to set.
 * @param val is the value to set.
 *
 * @returns The BSON element with the value set, or NULL on error.
 */
bson_element_t *bson_element_value_set_double (bson_element_t *e,
					       double val);

/** Get the value of a BSON element, as a double type.
 *
 * @param e is the element whose value we want to retrieve.
 * @param oval is the output variable to put the value into.
 *
 * @returns TRUE on success, FALSE otherwise.
 */
lmc_bool_t bson_element_value_get_double (bson_element_t *e,
					  double *oval);

/** Set the value of a BSON element to a 32bit integer.
 *
 * @param e is the element whose value to set.
 * @param val is the value to set.
 *
 * @returns The BSON element with the value set, or NULL on error.
 */
bson_element_t *bson_element_value_set_int32 (bson_element_t *e,
					      int32_t val);

/** Get the value of a BSON element, as a 32bit integer type.
 *
 * @param e is the element whose value we want to retrieve.
 * @param oval is the output variable to put the value into.
 *
 * @returns TRUE on success, FALSE otherwise.
 */
lmc_bool_t bson_element_value_get_int32 (bson_element_t *e,
					 int32_t *oval);

/** Set the value of a BSON element to a 64bit integer.
 *
 * @param e is the element whose value to set.
 * @param val is the value to set.
 *
 * @returns The BSON element with the value set, or NULL on error.
 */
bson_element_t *bson_element_value_set_int64 (bson_element_t *e,
					      int64_t val);

/** Get the value of a BSON element, as a 64bit integer type.
 *
 * @param e is the element whose value we want to retrieve.
 * @param oval is the output variable to put the value into.
 *
 * @returns TRUE on success, FALSE otherwise.
 */
lmc_bool_t bson_element_value_get_int64 (bson_element_t *e,
					 int64_t *oval);

/** Set the value of a BSON element to a string.
 *
 * @param e is the element whose value to set.
 * @param val is the value to set.
 * @param length is the lenght of the string to use. A negative number
 * (or #BSON_LENGTH_AUTO) means the length should be autodetected.
 *
 * @returns The BSON element with the value set, or NULL on error.
 */
bson_element_t *bson_element_value_set_string (bson_element_t *e,
					       const char *val,
					       int32_t length);

/** Get the value of a BSON element, as a string type.
 *
 * @param e is the element whose value we want to retrieve.
 * @param oval is the output variable to put the value into.
 *
 * @returns TRUE on success, FALSE otherwise.
 *
 * @note The returned string is an internal pointer, it is only valid
 * as long as the object is, and must not be freed or modified.
 */
lmc_bool_t bson_element_value_get_string (bson_element_t *e,
					  const char **oval);

/** Set the value of a BSON element to a javascript code.
 *
 * @param e is the element whose value to set.
 * @param val is the value to set.
 * @param length is the lenght of the string to use. A negative number
 * (or #BSON_LENGTH_AUTO) means the length should be autodetected.
 *
 * @returns The BSON element with the value set, or NULL on error.
 */
bson_element_t *bson_element_value_set_javascript (bson_element_t *e,
						   const char *val,
						   int32_t length);

/** Get the value of a BSON element, as a javascript code type.
 *
 * @param e is the element whose value we want to retrieve.
 * @param oval is the output variable to put the value into.
 *
 * @returns TRUE on success, FALSE otherwise.
 *
 * @note The returned string is an internal pointer, it is only valid
 * as long as the object is, and must not be freed or modified.
 */
lmc_bool_t bson_element_value_get_javascript (bson_element_t *e,
					      const char **oval);

/** Set the value of a BSON element to a boolean.
 *
 * @param e is the element whose value to set.
 * @param val is the value to set.
 *
 * @returns The BSON element with the value set, or NULL on error.
 */
bson_element_t *bson_element_value_set_boolean (bson_element_t *e,
						lmc_bool_t val);

/** Get the value of a BSON object, as a boolean type.
 *
 * @param e is the element whose value we want to retrieve.
 * @param oval is the output variable to put the value into.
 *
 * @returns TRUE on success, FALSE otherwise.
 */
lmc_bool_t bson_element_value_get_boolean (bson_element_t *e,
					   lmc_bool_t *oval);

/** Set the value of a BSON element to UTC datetime.
 *
 * @param e is the element whose value to set.
 * @param val is the value to set.
 *
 * @returns The BSON element with the value set, or NULL on error.
 */
bson_element_t *bson_element_value_set_datetime (bson_element_t *e,
						 int64_t val);

/** Get the value of a BSON element, as a UTC datetime type.
 *
 * @param e is the element whose value we want to retrieve.
 * @param oval is the output variable to put the value into.
 *
 * @returns TRUE on success, FALSE otherwise.
 */
lmc_bool_t bson_element_value_get_datetime (bson_element_t *e,
					    int64_t *oval);


/** Set the type and the value of a BSON element.
 *
 * @param e is the element to modify.
 * @param type is the BSON type to use.
 *
 * The rest of the parameters depend on the @a type. Whatever the
 * types setter takes, must be passed along here.
 *
 * @returns The BSON element with its type and value set.
 */
bson_element_t *bson_element_value_set (bson_element_t *e,
					bson_element_type_t type, ...);

/** Set all properties of a BSON element.
 *
 * @param e is the element to modify.
 * @param name is the name of the element.
 * @param type is the BSON type to use.
 *
 * The rest of the parameters depend on the @a type. Whatever the
 * types setter takes, must be passed along here.
 *
 * @returns The BSON element with its name, type and value set.
 */
bson_element_t *bson_element_set (bson_element_t *e, const char *name,
				  bson_element_type_t type, ...);

/** @}
 * @}
 *@}
 */

LMC_END_DECLS

#endif
