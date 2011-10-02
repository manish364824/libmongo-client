/* bson_oid.c - ObjectID API
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

/** @file lib/bson_oid.c
 * Implementation of the BSON ObjectID API.
 */

#include <lmc.h>
#include "lmc/private.h"

#include <sys/types.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

static uint32_t machine_id = 0;
static int16_t pid = 0;

void
bson_oid_init (int32_t mid)
{
  pid_t p = getpid ();

  if (mid == 0)
    {
      srand (time (NULL));
      machine_id = rand ();
    }
  else
    machine_id = mid;

  /*
   * If our pid has more than 16 bits, let half the bits modulate the
   * machine_id.
   */
  if (sizeof (pid_t) > 2)
    {
      machine_id ^= pid >> 16;
    }
  pid = (int16_t)p;
}

bson_oid_t *
bson_oid_new_with_time (time_t ts, int32_t seq)
{
  bson_oid_t *oid;
  time_t t = LMC_INT32_TO_BE (ts);
  int32_t tmp = LMC_INT32_TO_BE (seq);

  if (machine_id == 0 || pid == 0)
    return NULL;

  oid = lmc_new (bson_oid_t);
  memset (oid->bytes, 0, 12);

  /* Sequence number, last 3 bytes
   * For simplicity's sake, we put this in first, and overwrite the
   * first byte later.
   */
  memcpy (oid->bytes + 4 + 2 + 2, &tmp, 4);
  /* First four bytes: the time, BE byte order */
  memcpy (oid->bytes, &t, 4);
  /* Machine ID, byte order doesn't matter, 3 bytes */
  memcpy (oid->bytes + 4, &machine_id, 3);
  /* PID, byte order doesn't matter, 2 bytes */
  memcpy (oid->bytes + 4 + 3, &pid, 2);

  return oid;
}

bson_oid_t *
bson_oid_new (int32_t seq)
{
  return bson_oid_new_with_time (time (NULL), seq);
}

char *
bson_oid_as_string (const bson_oid_t *oid)
{
  char *str;
  int j;

  if (!oid)
    return NULL;

  str = lmc_alloc (char, 26);
  for (j = 0; j < 12; j++)
    sprintf (&str[j * 2], "%02x", oid->bytes[j]);
  str[25] = 0;
  return str;
}
