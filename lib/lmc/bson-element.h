/* lmc/bson-element.h - BSON element API
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

/** @file lib/lmc/bson-element.h
 */

#ifndef LMC_BSON_ELEMENT_H
#define LMC_BSON_ELEMENT_H

#include <lmc/common.h>

LMC_BEGIN_DECLS

/** @addtogroup lmc_bson
 * @{
 *  @defgroup lmc_bson_element BSON elements
 *  @addtogroup lmc_bson_element
 *  @{
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

/** @}
 * @}
 */

LMC_END_DECLS

#endif
