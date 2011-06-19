#include "test.h"
#include "mongo.h"

#include "libmongo-private.h"

void
test_mongo_sync_gridfs_file_cursor_get_chunk (void)
{
  mongo_sync_cursor c;
  gint32 size;

  ok (mongo_sync_gridfs_file_cursor_get_chunk (NULL, &size) == NULL,
      "mongo_sync_gridfs_file_cursor_get_chunk() fails with a NULL cursor");
  ok (mongo_sync_gridfs_file_cursor_get_chunk (&c, NULL) == NULL,
      "mongo_sync_gridfs_file_cursor_get_chunk() fails with a NULL size pointer");
}

RUN_TEST (2, mongo_sync_gridfs_file_cursor_get_chunk);
