#include "test.h"
#include "mongo.h"

#include <sys/socket.h>
#include "libmongo-private.h"

void
test_mongo_sync_cmd_index_create (void)
{
  mongo_sync_connection *c;
  bson *doc, *indexes;

  c = test_make_fake_sync_conn (-1, FALSE);
  doc = test_bson_generate_full ();
  indexes = bson_build (BSON_TYPE_INT32, "sex", 1,
			BSON_TYPE_INT32, "double", 1,
			BSON_TYPE_NONE);
  bson_finish (indexes);

  ok (mongo_sync_cmd_index_create (NULL, "test.ns", indexes, 0) == FALSE,
      "mongo_sync_cmd_index_create() fails with a NULL connection");
  ok (mongo_sync_cmd_index_create (c, NULL, indexes, 0) == FALSE,
      "mongo_sync_cmd_index_create() fails with a NULL namespace");
  ok (mongo_sync_cmd_index_create (c, "test.ns", NULL, 0) == FALSE,
      "mongo_sync_cmd_index_create() fails with NULL indexes");
  ok (mongo_sync_cmd_index_create (c, "bogus", indexes, 0) == FALSE,
      "mongo_sync_cmd_index_create() fails with a bogus namespace");
  ok (mongo_sync_cmd_index_create (c, "test.ns", indexes, 0) == FALSE,
      "mongo_sync_cmd_index_create() fails with a bogus FD");

  mongo_sync_disconnect (c);

  begin_network_tests (1);

  c = mongo_sync_connect (config.primary_host, config.primary_port,
			  TRUE);
  mongo_sync_cmd_insert (c, config.ns, doc, NULL);

  ok (mongo_sync_cmd_index_create(c, config.ns, indexes,
				  MONGO_INDEX_UNIQUE | MONGO_INDEX_DROP_DUPS),
      "mongo_sync_cmd_index_create() works");
  mongo_sync_disconnect (c);

  bson_free (doc);
  bson_free (indexes);

  end_network_tests ();
}

RUN_TEST (6, mongo_sync_cmd_index_create);
