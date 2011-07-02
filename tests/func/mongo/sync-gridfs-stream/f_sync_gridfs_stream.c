#include "test.h"
#include "mongo.h"

#define FILE_SIZE 1024 * 1024 + 12345
#define BUFFER_SIZE 64 * 1024

void
test_func_sync_gridfs_stream_write (void)
{
  mongo_sync_connection *conn;
  mongo_sync_gridfs *gfs;
  mongo_sync_gridfs_stream *stream;
  bson *meta;
  guint8 *data;
  gint pos = 0;

  conn = mongo_sync_connect (config.primary_host, config.primary_port, FALSE);
  gfs = mongo_sync_gridfs_new (conn, config.gfs_prefix);
  meta = bson_build (BSON_TYPE_STRING, "filename", "libmongo-test-stream", -1,
		     BSON_TYPE_NONE);
  bson_finish (meta);
  stream = mongo_sync_gridfs_stream_new (gfs, meta);
  bson_free (meta);

  data = g_malloc (BUFFER_SIZE);

  while (pos < FILE_SIZE)
    {
      gint csize = BUFFER_SIZE;

      if (csize + pos > FILE_SIZE)
	csize = FILE_SIZE - pos;

      memset (data, pos / BUFFER_SIZE, BUFFER_SIZE);

      mongo_sync_gridfs_stream_write (stream, data, csize);
      pos += csize;
    }

  g_free (data);
  mongo_sync_gridfs_stream_close (stream);
  mongo_sync_gridfs_free (gfs, TRUE);
}

void
test_func_sync_gridfs_stream (void)
{
  mongo_util_oid_init (0);

  test_func_sync_gridfs_stream_write ();
}

RUN_NET_TEST (0, func_sync_gridfs_stream);
