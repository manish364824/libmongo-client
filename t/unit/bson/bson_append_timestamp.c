#include "test.h"

static void
test_bson_timestamp (void)
{
  bson_t *b;
  bson_timestamp_t t;

  t.i = 10;
  t.t = 12345678;

  b = bson_finish
    (bson_append_timestamp (bson_new (), "ts", &t));
  ok (lmc_error_isok (b),
      "bson_append_timestamp() works");

  cmp_ok (bson_size (b), "==", 17, "BSON timestamp element size check");
  ok (memcmp (bson_data (b),
	      "\021\000\000\000\021\164\163\000\012\000\000\000\116\141\274"
	      "\000\000",
	      bson_size (b)) == 0,
      "BSON timestamp element contents check");

  bson_free (b);

  b = bson_new ();

  b = bson_append_timestamp (b, NULL, &t);
  ok (!lmc_error_isok (b),
      "bson_append_timestamp() with a NULL key should fail");
  lmc_error_reset (b);

  ok (bson_append_timestamp (NULL, "ts", &t) == NULL,
      "bson_append_timestamp() without a BSON object should fail");

  bson_finish (b);
  cmp_ok (bson_size (b), "==", 5,
	  "BSON object should be empty");

  b = bson_append_timestamp (b, "ts", &t);
  ok (lmc_error_get_errn (b) == EBUSY,
      "Appending to a finished element should fail");

  bson_free (b);
}

RUN_TEST (7, bson_timestamp);
