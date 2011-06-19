#include "test.h"
#include "mongo.h"

#define FILE_SIZE 1024 * 1024 + 12345

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
test_func_sync_gridfs_get (void)
{
  mongo_sync_connection *conn;
  mongo_sync_gridfs *gfs;
  mongo_sync_gridfs_file *f;
  mongo_sync_cursor *cursor;
  bson *b;

  conn = mongo_sync_connect (config.primary_host, config.primary_port, TRUE);
  gfs = mongo_sync_gridfs_new (conn, "test.fs");
  b = bson_build (BSON_TYPE_STRING, "filename", "libmongo-test", -1,
		  BSON_TYPE_NONE);
  bson_finish (b);

  f = mongo_sync_gridfs_find (gfs, b);

  bson_free (b);

  diag ("id = %s; length = %d; chunk_size = %d; date = %lu; md5 = %s; n = %d\n",
	oid_to_string (mongo_sync_gridfs_file_get_id (f)),
	mongo_sync_gridfs_file_get_length (f),
	mongo_sync_gridfs_file_get_chunk_size (f),
	mongo_sync_gridfs_file_get_date (f),
	mongo_sync_gridfs_file_get_md5 (f),
	mongo_sync_gridfs_file_get_chunks (f));

  cursor = mongo_sync_gridfs_file_cursor_new (f, 0, 0);
  while (mongo_sync_cursor_next (cursor))
    {
      gint32 size;
      guint8 *data;

      data = mongo_sync_gridfs_file_cursor_get_chunk (cursor, &size);

      diag ("size = %d\n", size);

      g_free (data);
    }
  mongo_sync_cursor_free (cursor);

  mongo_sync_gridfs_free (gfs, TRUE);
  mongo_sync_gridfs_file_free (f);
}

void
test_func_sync_gridfs_put (void)
{
  mongo_sync_connection *conn;
  mongo_sync_gridfs *gfs;
  mongo_sync_gridfs_file *gfile;
  bson *meta;
  guint8 *data;

  conn = mongo_sync_connect (config.primary_host, config.primary_port, FALSE);
  gfs = mongo_sync_gridfs_new (conn, "test.fs");
  meta = bson_build (BSON_TYPE_STRING, "filename", "libmongo-test", -1,
		     BSON_TYPE_NONE);
  bson_finish (meta);

  data = g_malloc (FILE_SIZE);
  memset (data, 'x', FILE_SIZE);

  gfile = mongo_sync_gridfs_file_new_from_buffer (gfs, meta, data, FILE_SIZE);

  g_free (data);
  bson_free (meta);
  mongo_sync_gridfs_free (gfs, TRUE);
  mongo_sync_gridfs_file_free (gfile);
}

void
test_func_sync_gridfs (void)
{
  mongo_util_oid_init (0);

  test_func_sync_gridfs_put ();
  test_func_sync_gridfs_get ();
}

RUN_NET_TEST (0, func_sync_gridfs);
