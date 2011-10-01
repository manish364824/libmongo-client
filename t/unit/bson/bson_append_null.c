#include "test.h"

static void
test_bson_null (void)
{
  bson_t *b;

  b = bson_finish (bson_append_null (bson_new (), "null"));
  ok (lmc_error_isok (b),
      "bson_append_null() works");
  bson_finish (b);

  cmp_ok (bson_size (b), "==", 11, "BSON NULL element size check");
  ok (memcmp (bson_data (b),
	      "\013\000\000\000\012\156\165\154\154\000\000",
	      bson_size (b)) == 0,
      "BSON NULL element contents check");

  bson_free (b);

  b = bson_new ();

  b = bson_append_null (b, NULL);
  ok (!lmc_error_isok (b),
      "bson_append_null() should fail without a key name");
  lmc_error_reset (b);

  ok (bson_append_null (NULL, "null") == NULL,
      "bson_append_null() should fail without a BSON object");

  bson_finish (b);
  cmp_ok (bson_size (b), "==", 5,
	  "BSON object should be empty");

  b = bson_append_null (b, "null");
  ok (lmc_error_get_errn (b) == EBUSY,
      "Appending to a finished element should fail");

  bson_free (b);
}

RUN_TEST (7, bson_null);
