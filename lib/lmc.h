/* lmc.h - libmongo-client meta-header
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

/** @file lib/lmc.h
 * libmongo-client meta-header.
 *
 * This header includes all the rest, it is advised for applications
 * to include this header, and this header only.
 */

#ifndef LMC_H
#define LMC_H 1

#include <lmc/common.h>
#include <lmc/error.h>

#include <lmc/bson.h>
#include <lmc/bson_oid.h>

#include <lmc/mongo/wire.h>

#endif
