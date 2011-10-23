/* lmc.h - libmongo-client meta-header
 * Copyright (C) 2011 Gergely Nagy <algernon@balabit.hu>
 * This file is part of the libmongo-client library.
 *
 * This library is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

/** @file lib/lmc.h
 * libmongo-client meta-header.
 *
 * This header includes all the rest, it is advised for applications
 * to include this header, and this header only.
 */

#ifndef LMC_H
#define LMC_H 1

#include <lmc/common.h>

/** @defgroup lmc_bson BSON */
#include <lmc/bson-element.h>
#include <lmc/bson.h>

#endif
