#include "test.h"
#include "mongo.h"

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

  cursor = mongo_sync_gridfs_file_get_chunks (f, 0, 0);
  while (mongo_sync_cursor_next (cursor))
    {
      bson *b = mongo_sync_cursor_get_data (cursor);
      bson_cursor *c = bson_find (b, "n");

      printf ("n = %s\n", bson_cursor_type_as_string (c));
      bson_cursor_free (c);
      bson_free (b);
    }

  mongo_sync_gridfs_free (gfs, TRUE);
  mongo_sync_gridfs_file_free (f);
}

RUN_NET_TEST (0, func_sync_gridfs);
