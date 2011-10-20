/* lmc/bson-element.h - BSON element API
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

/** @file lib/lmc/bson-element.h
 */

#ifndef LMC_BSON_ELEMENT_H
#define LMC_BSON_ELEMENT_H

#include <lmc/common.h>

LMC_BEGIN_DECLS

/** A BSON element container.
 *
 * This type can hold any type of BSON element.
 */
typedef struct _bson_element_t bson_element_t;

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
 */
void bson_element_unref (bson_element_t *e);

LMC_END_DECLS

#endif
