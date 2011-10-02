#include "test.h"

static void
test_bson_oid_as_string (void)
{
  bson_oid_t *oid;
  char *oid_str;

  bson_oid_init (0);

  oid = bson_oid_new (1);

  ok (bson_oid_as_string (NULL) == NULL,
      "bson_oid_as_string() should fail with a NULL oid");

  oid_str = bson_oid_as_string (oid);

  ok (oid_str != NULL,
      "mongo_util_oid_as_string() works");

  free (oid_str);
  free (oid);
}

RUN_TEST (2, bson_oid_as_string);
