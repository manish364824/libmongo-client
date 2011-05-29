#include "test.h"
#include "mongo.h"
#include "config.h"

#include "libmongo-private.h"

#include <errno.h>

void
test_mongo_sync_cursor_next (void)
{
  mongo_sync_connection *conn;
  mongo_packet *p;
  mongo_packet_header h;
  mongo_sync_cursor *c;

  test_env_setup ();

  p = test_mongo_wire_generate_reply (TRUE, 2, TRUE);
  conn = test_make_fake_sync_conn (-1, FALSE);

  c = mongo_sync_cursor_new (conn, config.ns, p);

  ok (mongo_sync_cursor_next (NULL) == FALSE,
      "mongo_sync_cursor_next() should fail with a NULL cursor");
  ok (mongo_sync_cursor_next (c) == TRUE,
      "mongo_sync_cursor_next() works");

  mongo_sync_cursor_free (c);
  mongo_sync_disconnect (conn);
  test_env_free ();
}

RUN_TEST (2, mongo_sync_cursor_next);
