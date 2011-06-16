#include "test.h"
#include "mongo.h"

#include <errno.h>

void
test_mongo_sync_gridfs_free (void)
{
  mongo_sync_connection *conn;
  mongo_sync_gridfs *gfs;

  conn = test_make_fake_sync_conn (4, TRUE);
  gfs = mongo_sync_gridfs_new (conn, NULL);

  errno = 0;
  mongo_sync_gridfs_free (NULL, FALSE);
  cmp_ok (errno, "==", ENOTCONN,
	  "mongo_sync_gridfs_free() with a NULL connection shall set errno");

  mongo_sync_gridfs_free (gfs, FALSE);
  cmp_ok (errno, "==", 0,
	  "mongo_sync_gridfs_free() should clear errno on success");

  gfs = mongo_sync_gridfs_new (conn, NULL);
  mongo_sync_gridfs_free (gfs, TRUE);
  cmp_ok (errno, "==", 0,
	  "mongo_sync_gridfs_free() works when asked to free the "
	  "connection too");
}

RUN_TEST (3, mongo_sync_gridfs_free);
