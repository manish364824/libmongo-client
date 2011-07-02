#include "test.h"
#include "mongo.h"

void
test_mongo_sync_gridfs_stream_from_file (void)
{
  ok (mongo_sync_gridfs_stream_from_file (NULL) == NULL,
      "mongo_sync_gridfs_stream_from_file() should fail with a NULL file");
}

RUN_TEST (1, mongo_sync_gridfs_stream_from_file);
