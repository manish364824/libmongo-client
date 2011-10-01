#include "test.h"

static void
test_bson_int64 (void)
{
  bson_t *b;
  int64_t l = 9876543210;

  b = bson_finish
    (bson_append_int64 (bson_new (), "i64", l));
  ok (lmc_error_isok (b),
      "bson_append_int64() works");

  cmp_ok (bson_size (b), "==", 18, "BSON int64 element size check");
  ok (memcmp (bson_data (b),
	      "\022\000\000\000\022\151\066\064\000\352\026\260\114\002\000"
	      "\000\000\000",
	      bson_size (b)) == 0,
      "BSON int64 element contents check");

  bson_free (b);

  b = bson_new ();

  b = bson_append_int64 (b, NULL, l);
  ok (!lmc_error_isok (b),
      "bson_append_int64() with a NULL key should fail");
  lmc_error_reset (b);

  ok (bson_append_int64 (NULL, "i64", l) == NULL,
      "bson_append_int64() without a BSON object should fail");

  bson_finish (b);
  cmp_ok (bson_size (b), "==", 5,
	  "BSON object should be empty");

  b = bson_append_int64 (b, "i64", l);
  ok (lmc_error_get_errn (b) == EBUSY,
      "Appending to a finished element should fail");

  bson_free (b);
}

RUN_TEST (7, bson_int64);
