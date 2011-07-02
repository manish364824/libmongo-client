#include "test.h"
#include "mongo.h"

#include "libmongo-private.h"

#include <unistd.h>

void
test_mongo_sync_gridfs_stream_seek (void)
{
  mongo_sync_connection *conn;
  mongo_sync_gridfs *gfs;
  mongo_sync_gridfs_stream *stream;

  ok (mongo_sync_gridfs_stream_seek (NULL, 0, SEEK_SET) == FALSE,
      "mongo_sync_gridfs_stream_seek() fails with a NULL stream");

  begin_network_tests (7);

  conn = mongo_sync_connect (config.primary_host, config.primary_port, FALSE);
  gfs = mongo_sync_gridfs_new (conn, config.gfs_prefix);

  stream = mongo_sync_gridfs_stream_new (gfs, NULL);

  ok (mongo_sync_gridfs_stream_seek (stream, 0, SEEK_SET) == FALSE,
      "mongo_sync_gridfs_stream_seek() fails with a write stream");

  stream->write_stream = FALSE;

  ok (mongo_sync_gridfs_stream_seek (stream, -1, SEEK_SET) == FALSE,
      "mongo_sync_gridfs_stream_seek() fails with SEEK_SET and a negative "
      "position");

  ok (mongo_sync_gridfs_stream_seek (stream, 10, SEEK_SET) == FALSE,
      "mongo_sync_gridfs_stream_seek() fails with SEEK_SET and a position "
      "past EOF");

  ok (mongo_sync_gridfs_stream_seek (stream, -1, SEEK_CUR) == FALSE,
      "mongo_sync_gridfs_stream_seek() fails with SEEK_CUR and a position "
      "before the start");

  ok (mongo_sync_gridfs_stream_seek (stream, 10, SEEK_CUR) == FALSE,
      "mongo_sync_gridfs_stream_seek() fails with SEEK_CUR and a position "
      "past EOF");

  ok (mongo_sync_gridfs_stream_seek (stream, 1, SEEK_END) == FALSE,
      "mongo_sync_gridfs_stream_seek() fails with SEEK_END and a position "
      "past EOF");

  ok (mongo_sync_gridfs_stream_seek (stream, -1, SEEK_END) == FALSE,
      "mongo_sync_gridfs_stream_seek() fails with SEEK_END and a position "
      "before the start");

  mongo_sync_gridfs_stream_close (stream);
  mongo_sync_gridfs_free (gfs, TRUE);

  end_network_tests ();
}

RUN_TEST (8, mongo_sync_gridfs_stream_seek);
