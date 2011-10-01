/* lmc/error.h - error reporting and propagation tools
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

/** @file lib/lmc/error.h
 */

#ifndef LMC_ERROR_H
#define LMC_ERROR_H 1

#include <lmc/common.h>

#include <assert.h>
#include <stddef.h>

LMC_BEGIN_DECLS

/** @defgroup lmc_error Error handling
 *
 * The types and functions within this module are the basis of error
 * handling within the library.
 *
 * Objects that wish to use these capabilities should use #lmc_error_t
 * as their parent, or otherwise provide access to the lmc_error_t
 * structure embedded into them.
 *
 * The basic idea is that every function that can be chained, will
 * return the resulting object, and if any of them encounter an error,
 * they use lmc_error_raise() to signal an error, and every subsequent
 * function will simply skip its function, and re-throw the object
 * back.
 *
 * This way, we end up with the error bubling up appropriately, so it
 * can be handled at the most appropriate level.
 *
 * @addtogroup lmc_error
 * @{
 */

/** The basic error structure.
 * While defined publicly, this should not be accessed directly: use
 * the accessor functions and macros instead!
 */
typedef struct
{
  const char *source; /**< The source of the error. */
  int errn; /**< The errno value of the error. */
} lmc_error_t;

/** Reset an error to a succesful value.
 * Clear the errno and error source of an error.
 *
 * @param err is the error to reset.
 */
static inline void lmc_error_reset (void *err)
{
  lmc_error_t *e = (lmc_error_t *)err;

  if (!err)
    return;

  e->source = NULL;
  e->errn = 0;
}

/** Set the source and errno of an error.
 *
 * @param err is the error whose value should be set.
 * @param source is the source function the error occurred at.
 * @param errn is the errno of the error.
 */
static inline void lmc_error_set (lmc_error_t *err, const char *source,
				  int errn)
{
  if (!err)
    return;

  err->source = source;
  err->errn = errn;
}

/** Get the source of the error.
 * @param err is the error whose source one's interested in.
 *
 * @returns The source where the error occurred. The returned string
 * is owned by the object, and shall not be freed or modified.
 */
static inline const char *lmc_error_get_source (const void *err)
{
  const lmc_error_t *e = (const lmc_error_t *)err;

  assert (e != NULL);
  return (char *)e->source;
}

/** Get the errno that belongs to an error.
 *
 * @param err is the error whose errno one's interested in.
 *
 * @returns The errno associated with the specified error.
 */
static inline int lmc_error_get_errn (const void *err)
{
  const lmc_error_t *e = (const lmc_error_t *)err;

  assert (e != NULL);
  return e->errn;
}

/** Raise an error from the current function.
 * This is a convenience wrapper around lmc_error_set(), that uses the
 * name of the current function as the source.
 *
 * @param err is the error to set values in.
 * @param errn is the errno value to set.
 *
 * @note The macro will force a return with the supplied error object!
 */
#define lmc_error_raise(err, errn)					\
  do									\
    {									\
      lmc_error_set ((lmc_error_t *)(err), __PRETTY_FUNCTION__, errn);	\
      return err;							\
    } while (0);							\

/** Verify whether an error is clear.
 * @param err is the error object to verify.
 *
 * @returns A non-zero value when the errno is set, zero otherwise.
 */
#define lmc_error_isok(err) \
  (((lmc_error_t *)err)->errn == 0)

/** @} */

LMC_END_DECLS

#endif
