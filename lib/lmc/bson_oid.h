/* lmc/bson_oid.h - ObjectID API
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

/** @file lib/lmc/bson_oid.h
 */

#ifndef LMC_BSON_OID_H
#define LMC_BSON_OID_H 1

#include <lmc/common.h>
#include <lmc/bson.h>

#include <time.h>

LMC_BEGIN_DECLS

/** @addtogroup bson_mod
 * @{
 *
 * @defgroup bson_oid ObjectID functions
 *
 * Functions to help creating ObjectIDs.
 *
 * @addtogroup bson_oid
 * @{
 */

/** Intitialize the static ObjectID components.
 *
 * @param machine_id is the machine id to use, or zero to generate one
 * automatically.
 *
 * This function needs to be called once, before any OIDs are
 * generated. It is also a good idea to call it whenever the calling
 * program's PID might change.
 */
void bson_oid_init (int32_t machine_id);

/** Generate a new ObjectID.
 *
 * Based on the current time, the pre-determined pid and machine ID
 * and a supplied sequence number, generate a new ObjectID.
 *
 * The machine id and the PID are updated whenever bson_oid_init() is
 * called.
 *
 * @param seq is the sequence number to use.
 *
 * @note The ObjectID has space for only 24 bits of sequence bytes, so
 * it should be noted that while @a seq is 32 bits wide, only 24 of
 * that will be used.
 *
 * @returns A newly allocated ObjectID or NULL on error. Freeing it is
 * the responsibility of the caller.
 */
bson_oid_t *bson_oid_new (int32_t seq);

/** Generate a new ObjectID, with a predefined timestamp.
 *
 * Based on the suppiled time and sequence number, and the
 * pre-determined pid and machine ID, generate a new ObjectID.
 *
 * The machine id and the PID are updated whenever bson_oid_init() is
 * called.
 *
 * @param time is the timestamp to use.
 * @param seq is the sequence number to use.
 *
 * @note The ObjectID has space for only 24 bits of sequence bytes, so
 * it should be noted that while @a seq is 32 bits wide, only 24 of
 * that will be used.
 *
 * @returns A newly allocated ObjectID or NULL on error. Freeing it is
 * the responsibility of the caller.
 */
bson_oid_t *bson_oid_new_with_time (time_t time, int32_t seq);

/** Convert an ObjectID to its string representation.
 *
 * Turns a binary ObjectID into a hexadecimal string.
 *
 * @param oid is the binary ObjectID.
 *
 * @returns A newly allocated string representation of the ObjectID,
 * or NULL on error. It is the responsibility of the caller to free it
 * once it is no longer needed.
 */
char *bson_oid_as_string (const bson_oid_t *oid);

/** @} */

/** @} */

LMC_END_DECLS

#endif
