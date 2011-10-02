#include "test.h"

static void
test_bson_oid_init (void)
{
  bson_oid_init (0);
  bson_oid_init (1234);

  /* We don't do any real testing here, only check if it does not
     crash. To verify that it works, we need to create a new OID, and
     that will be tested by other unit tests.
  */
  ok (TRUE,
      "bson_oid_init() does not crash.");
}

RUN_TEST (1, bson_oid_init);
