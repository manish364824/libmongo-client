/* lmc/endian.h - endian conversion macros
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

/** @file lib/lmc/endian.h
 */

#ifndef LMC_ENDIAN_H
#define LMC_ENDIAN_H

#include <lmc/common.h>

#include <endian.h>

LMC_BEGIN_DECLS

#define LMC_INT32_TO_LE(i) htole32(i)
#define LMC_INT32_FROM_LE(i) le32toh(i)

#define LMC_INT64_TO_LE(i) htole64(i)
#define LMC_INT64_FROM_LE(i) le64toh(i)

#define LMC_INT32_TO_BE(i) htobe32(i)

inline static double
LMC_DOUBLE_SWAP_LE_BE(double in)
{
  union
  {
    uint64_t i;
    double d;
  } u;

  u.d = in;
  u.i = __bswap_64 (u.i);
  return u.d;
}

#if __BYTE_ORDER == __LITTLE_ENDIAN
#define LMC_DOUBLE_TO_LE(val)   ((double) (val))
#define LMC_DOUBLE_TO_BE(val)   (LMC_DOUBLE_SWAP_LE_BE (val))

#elif __BYTE_ORDER == __BIG_ENDIAN
#define LMC_DOUBLE_TO_LE(val)   (LMC_DOUBLE_SWAP_LE_BE (val))
#define LMC_DOUBLE_TO_BE(val)   ((double) (val))

#else /* !__LITTLE_ENDIAN && !__BIG_ENDIAN */
#error unknown ENDIAN type
#endif /* !__LITTLE_ENDIAN && !__BIG_ENDIAN */

#define LMC_DOUBLE_FROM_LE(val) (LMC_DOUBLE_TO_LE (val))
#define LMC_DOUBLE_FROM_BE(val) (LMC_DOUBLE_TO_BE (val))

LMC_END_DECLS

#endif
