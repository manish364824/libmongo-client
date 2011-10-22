/* lmc/bson.h - BSON API
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

/** @file lib/lmc/bson.h
 */

#ifndef LMC_BSON_H
#define LMC_BSON_H

#include <lmc/common.h>
#include <lmc/bson-element.h>

LMC_BEGIN_DECLS

/** @addtogroup lmc_bson
 * @{
 */

/** A BSON container.
 */
typedef struct _bson_t bson_t;

/** Create a new BSON object.
 *
 * @returns A newly allocated BSON object.
 */
bson_t *bson_new (void);

/** Create a new BSON object, with preallocated space.
 *
 * @param size is the amount of space to preallocate for the final
 * representation.
 *
 * @returns A newly allocated BSON object, with preallocated space.
 */
bson_t *bson_new_sized (uint32_t size);

/** Reset a BSON object.
 *
 * Removes all elements from the object, without completely destroying
 * it.
 *
 * @param b is the BSON object to drop elements from.
 *
 * @returns The BSON object with its elements removed.
 */
bson_t *bson_reset (bson_t *b);

/** Increase the reference count of a BSON object.
 *
 * @param b is the object to increase the refcount of.
 *
 * @returns The BSON object with its refcount changed.
 */
bson_t *bson_ref (bson_t *b);

/** Decrease the reference count of a BSON object.
 *
 * Whenever the refcount reaches zero, the object will be freed, and
 * all associated elements unrefed.
 *
 * @param b is the object to decrease the refcount of.
 *
 * @returns The BSON object, or NULL if it was freed.
 */
bson_t *bson_unref (bson_t *b);

/** Open a BSON object, so that elements can be added.
 *
 * @param b is the object to open.
 *
 * @returns The opened BSON object.
 */
bson_t *bson_open (bson_t *b);

/** Close a BSON object, so that elements cannot be added.
 *
 * @param b is the object to close.
 *
 * @returns The closed BSON object.
 */
bson_t *bson_close (bson_t *b);

/** Get the number of elements in a BSON object.
 *
 * @param b is the object to count elements in.
 *
 * @returns The number of elements in the object.
 */
uint32_t bson_length (bson_t *b);

/** Get the data stream of the BSON object.
 *
 * @param b is the BSON object whose data we want to retrieve.
 *
 * @returns The raw BSON data stream for the object. The returned data
 * is a pointer to an internal structure, and must not be freed or
 * modified. It is also only valid until the BSON object exists.
 *
 * @note The object must be closed!
 */
const uint8_t *bson_data_get (bson_t *b);

/** Get the size of the BSON object's data stream.
 *
 * @param b is the BSON object whose data's length we want to see.
 *
 * @returns The length of the raw BSON data stream.
 *
 * @note The object must be closed!
 */
uint32_t bson_data_get_size (bson_t *b);

/** @defgroup lmc_bson_builder Builder
 * @addtogroup lmc_bson_builder
 * @{
 */

/** End of element list signal. */
#define BSON_END NULL

/** Append elements to a BSON object.
 *
 * @param b is the BSON object to append to.
 *
 * It must be followed by #bson_element_t objects, terminated by
 * #BSON_END, and all of them will be appended to the object.
 *
 * @returns The BSON object with the elements appended.
 *
 * @note The elements' refcount is NOT incremented by this function,
 * if one wishes to use them after unrefing the BSON object, the
 * refcount must be incremented manually.
 */
bson_t *bson_append (bson_t *b, ...);

/** Build a new BSON object from elements.
 *
 * @param e is the first element to add.
 *
 * The rest of the arguments are #bson_element_t objects aswell,
 * terminated by a #BSON_END.
 *
 * @returns A new BSON object with the elements appended.
 *
 * @note The elements' refcount is NOT incremented by this function,
 * if one wishes to use them after unrefing the BSON object, the
 * refcount must be incremented manually.
 */
bson_t *bson_new_build (bson_element_t *e, ...);

/** @}
 * @}
 */

LMC_END_DECLS

#endif
