/* mongo-utils.h - libmongo-client utility functions
 * Copyright 2011 Gergely Nagy <algernon@balabit.hu>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/** @file src/mongo-utils.h
 * Public header for various libmongo-client helper functions.
 */

#ifndef LIBMONGO_CLIENT_UTILS_H
#define LIBMONGO_CLIENT_UTILS_H 1

#include <glib.h>

G_BEGIN_DECLS

/** @defgroup mongo_util Mongo Utils
 *
 * Various utility functions related to MongoDB.
 *
 * @addtogroup mongo_util
 * @{
 */

/** Parse a HOST:IP pair.
 *
 * Given a HOST:IP pair, split it up into a host and a port. IPv6
 * addresses supported, the function cuts at the last ":".
 *
 * @param addr is the address to split.
 * @param host is a pointer to a string where the host part will be
 * stored.
 * @param port is a pointer to an integer, where the port part will be
 * stored.
 *
 * @returns TRUE on success, FALSE otherwise. The @a host parameter
 * will contain a newly allocated string on succes. On failiure, host
 * will be set to NULL, and port to -1.
 */
gboolean mongo_util_parse_addr (const gchar *addr, gchar **host,
				gint *port);

/** @} */

G_END_DECLS

#endif
