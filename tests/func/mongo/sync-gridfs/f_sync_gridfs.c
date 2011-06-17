#include "test.h"
#include "mongo.h"

gchar *
oid_to_string (const guint8* oid)
{
  static gchar os[25];
  gint j;

  for (j = 0; j < 12; j++)
    sprintf (&os[j * 2], "%02x", oid[j]);
  os[25] = 0;
  return os;
}

void
test_func_sync_gridfs (void)
{
  mongo_sync_connection *conn;
  mongo_sync_gridfs *gfs;
  mongo_sync_gridfs_file *f;
  mongo_sync_cursor *cursor;
  bson *b;

  conn = mongo_sync_connect (config.primary_host, config.primary_port, TRUE);
  gfs = mongo_sync_gridfs_new (conn, "test.fs");
  b = bson_build (BSON_TYPE_STRING, "filename", "/bin/bash", -1,
		  BSON_TYPE_NONE);
  bson_finish (b);

  f = mongo_sync_gridfs_find (gfs, b);

  diag ("id = %s; length = %d; chunk_size = %d; date = %lu; md5 = %s; n = %d\n",
	oid_to_string (mongo_sync_gridfs_file_get_id (f)),
	mongo_sync_gridfs_file_get_length (f),
	mongo_sync_gridfs_file_get_chunk_size (f),
	mongo_sync_gridfs_file_get_date (f),
	mongo_sync_gridfs_file_get_md5 (f),
	mongo_sync_gridfs_file_get_chunk_count (f));

  cursor = mongo_sync_gridfs_file_get_chunks (f, 0, 0);
  while (mongo_sync_cursor_next (cursor))
    {
      gint32 size;
      guint8 *data;

      data = mongo_sync_gridfs_file_cursor_get_chunk (cursor, &size);

      diag ("size = %d\n", size);
    }

  mongo_sync_gridfs_free (gfs, TRUE);
  mongo_sync_gridfs_file_free (f);
}

RUN_NET_TEST (0, func_sync_gridfs);
