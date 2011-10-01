/* lmc-error.h - error reporting and propagation tools
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

/** @file lib/lmc-error.h
 */

#ifndef LMC_ERROR_H
#define LMC_ERROR_H 1

#include <lmc-common.h>
#include <assert.h>
#include <stddef.h>

LMC_BEGIN_DECLS

typedef struct
{
  const char *source;
  int errn;
} lmc_error_t;

static inline void lmc_error_reset (void *err)
{
  lmc_error_t *e = (lmc_error_t *)err;

  if (!err)
    return;

  e->source = NULL;
  e->errn = 0;
}

static inline void lmc_error_set (lmc_error_t *err, const char *source,
				  int errn)
{
  if (!err)
    return;

  err->source = source;
  err->errn = errn;
}

static inline const char *lmc_error_get_source (const void *err)
{
  const lmc_error_t *e = (const lmc_error_t *)err;

  assert (e != NULL);
  return (char *)e->source;
}

static inline int lmc_error_get_errn (const void *err)
{
  const lmc_error_t *e = (const lmc_error_t *)err;

  assert (e != NULL);
  return e->errn;
}

#define lmc_error_raise(err, errn) \
  lmc_error_set ((lmc_error_t *)(err), __PRETTY_FUNCTION__, errn)
#define lmc_error_isok(err) \
  (((lmc_error_t *)err)->errn == 0)

LMC_END_DECLS

#endif
