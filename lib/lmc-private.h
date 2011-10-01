/* lmc-private.h - private headers for libmongo-client
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

/** @file lmc-private.h
 *
 * Private types and functions, for internal use in libmongo-client only.
 */

#ifndef LMC_PRIVATE_H
#define LMC_PRIVATE_H 1

#include <lmc-common.h>
#include <inttypes.h>
#include <stdlib.h>

#define lmc_new(type) \
  (type *)lmc_alloc (type, 1)

#define lmc_alloc(type, num) \
  (type *)_lmc_malloc (sizeof (type) * num)

static inline void *_lmc_malloc (size_t size)
{
  void *m;

  m = malloc (size);
  assert (m != NULL);
  return m;
}

static inline void *lmc_realloc (void *p, size_t size)
{
  void *m;

  m = realloc (p, size);
  assert (m != NULL);
  return m;
}

#endif
