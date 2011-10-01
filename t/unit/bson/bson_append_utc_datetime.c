#include "test.h"

static void
test_bson_utc_datetime (void)
{
  bson_t *b;

  b = bson_finish
    (bson_append_utc_datetime (bson_new (), "date", 1294860709000));

  ok (lmc_error_isok (b),
      "bson_append_utc_datetime() works");

  cmp_ok (bson_size (b), "==", 19, "BSON UTC datetime element size check");
  ok (memcmp (bson_data (b),
	      "\023\000\000\000\011\144\141\164\145\000\210\154\266\173\055"
	      "\001\000\000\000",
	      bson_size (b)) == 0,
      "BSON UTC datetime element contents check");

  bson_free (b);

  b = bson_new ();

  b = bson_append_utc_datetime (b, NULL, 1294860709000);

  ok (!lmc_error_isok (b),
      "bson_append_utc_datetime() with a NULL key should fail");
  lmc_error_reset (b);

  ok (bson_append_utc_datetime (NULL, "date", 1294860709000) == NULL,
      "bson_append_utc_datetime() without a BSON object should fail");

  bson_finish (b);
  cmp_ok (bson_size (b), "==", 5,
	  "BSON object should be empty");

  b = bson_append_utc_datetime (b, "date", 1294860709000);
  ok (lmc_error_get_errn (b) == EBUSY,
      "Appending to a finished element should fail");

  bson_free (b);
}

RUN_TEST (7, bson_utc_datetime);
