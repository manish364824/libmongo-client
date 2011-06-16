/* sync-gridfs.c - libmongo-client GridFS implementation
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

/** @file src/sync-gridfs.c
 * MongoDB GridFS implementation.
 */

#include "sync-gridfs.h"
#include "libmongo-private.h"

#include <errno.h>

mongo_sync_gridfs *
mongo_sync_gridfs_new (mongo_sync_connection *conn,
		       const gchar *ns_prefix)
{
  mongo_sync_gridfs *gfs;

  if (!conn)
    {
      errno = ENOTCONN;
      return NULL;
    }

  gfs = g_new (mongo_sync_gridfs, 1);
  gfs->conn = conn;
  if (ns_prefix)
    gfs->ns.prefix = g_strdup (ns_prefix);
  else
    gfs->ns.prefix = g_strdup ("fs");

  gfs->ns.files = g_strconcat (gfs->ns.prefix, ".files", NULL);
  gfs->ns.chunks = g_strconcat (gfs->ns.prefix, ".chunks", NULL);

  gfs->chunk_size = 256 * 1024;

  return gfs;
}

void
mongo_sync_gridfs_free (mongo_sync_gridfs *gfs, gboolean disconnect)
{
  if (!gfs)
    {
      errno = ENOTCONN;
      return;
    }

  g_free (gfs->ns.prefix);
  g_free (gfs->ns.files);
  g_free (gfs->ns.chunks);

  if (disconnect)
    mongo_sync_disconnect (gfs->conn);

  g_free (gfs);
  errno = 0;
}

gint32
mongo_sync_gridfs_get_chunk_size (mongo_sync_gridfs *gfs)
{
  if (!gfs)
    {
      errno = ENOTCONN;
      return -1;
    }
  return gfs->chunk_size;
}

gboolean
mongo_sync_gridfs_set_chunk_size (mongo_sync_gridfs *gfs,
				  gint32 chunk_size)
{
  if (!gfs)
    {
      errno = ENOTCONN;
      return FALSE;
    }
  if (chunk_size < 1)
    {
      errno = EINVAL;
      return FALSE;
    }

  gfs->chunk_size = chunk_size;
  return TRUE;
}
