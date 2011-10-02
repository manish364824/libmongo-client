/* mongo-utils.c - libmongo-client utility functions
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

/** @file src/mongo-utils.c
 * Implementation for various libmongo-client helper functions.
 */

#include <glib.h>
#include <glib/gprintf.h>

#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>

gboolean
mongo_util_parse_addr (const gchar *addr, gchar **host, gint *port)
{
  gchar *port_s, *ep;
  glong p;

  if (!addr || !host || !port)
    {
      if (host)
	*host = NULL;
      if (port)
	*port = -1;
      errno = EINVAL;
      return FALSE;
    }

  /* Check for IPv6 literal */
  if (addr[0] == '[')
    {
      /* Host is everything between [] */
      port_s = strchr (addr + 1, ']');
      if (!port_s || port_s - addr == 1)
	{
	  *host = NULL;
	  *port = -1;
	  errno = EINVAL;
	  return FALSE;
	}
      *host = g_strndup (addr + 1, port_s - addr - 1);

      port_s += 2;
      if (port_s - addr >= (glong)strlen (addr))
	return TRUE;
    }
  else
    {
      /* Dealing with something that's not an IPv6 literal */

      /* Split up to host:port */
      port_s = g_strrstr (addr, ":");
      if (!port_s)
	{
	  *host = g_strdup (addr);
	  return TRUE;
	}
      if (port_s == addr)
	{
	  *host = NULL;
	  *port = -1;
	  errno = EINVAL;
	  return FALSE;
	}
      port_s++;
      *host = g_strndup (addr, port_s - addr - 1);
    }

  p = strtol (port_s, &ep, 10);
  if (p == LONG_MIN || p == LONG_MAX)
    {
      g_free (*host);
      *host = NULL;
      *port = -1;
      errno = ERANGE;
      return FALSE;
    }
  if (p < 0 || p > INT_MAX)
    {
      g_free (*host);
      *host = NULL;
      *port = -1;
      errno = ERANGE;
      return FALSE;
    }
  *port = (gint)p;

  if (ep && *ep)
    {
      g_free (*host);
      *host = NULL;
      *port = -1;
      errno = EINVAL;
      return FALSE;
    }
  return TRUE;
}
