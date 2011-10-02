/* lmc/endian.h - endian conversion macros
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

/** @file lib/lmc/endian.h
 */

#ifndef LMC_ENDIAN_H
#define LMC_ENDIAN_H

#include <lmc/common.h>

LMC_BEGIN_DECLS

#define LMC_INT32_TO_LE(i) (i)
#define LMC_INT32_FROM_LE(i) (i)

#define LMC_DOUBLE_TO_LE(d) (d)
#define LMC_DOUBLE_FROM_LE(d) (d)

#define LMC_INT64_TO_LE(i) (i)
#define LMC_INT64_FROM_LE(i) (i)

#define LMC_INT32_TO_BE(i) (i)

LMC_END_DECLS

#endif
