#include "test.h"

static void
test_bson_boolean (void)
{
  bson_t *b;

  b = bson_finish
    (bson_append_boolean
     (bson_append_boolean (bson_new (), "FALSE", FALSE),
      "TRUE", TRUE));

  ok (lmc_error_isok (b),
      "bson_append_boolean() works");

  cmp_ok (bson_size (b), "==", 20, "BSON boolean element size check");
  ok (memcmp (bson_data (b),
	      "\024\000\000\000\010\106\101\114\123\105\000\000\010\124\122"
	      "\125\105\000\001\000",
	      bson_size (b)) == 0,
      "BSON boolean element contents check");

  bson_free (b);

  b = bson_new ();

  b = bson_append_boolean (b, NULL, TRUE);
  ok (!lmc_error_isok (b),
      "bson_append_boolean() with a NULL key should fail");
  lmc_error_reset (b);

  ok (bson_append_boolean (NULL, "TRUE", TRUE) == NULL,
      "bson_append_boolean() without a BSON object should fail");

  bson_finish (b);
  cmp_ok (bson_size (b), "==", 5,
	  "BSON object should be empty");

  b = bson_append_boolean (b, "b", TRUE);
  ok (lmc_error_get_errn (b) == EBUSY,
      "Appending to a finished element should fail");

  bson_free (b);
}

RUN_TEST (7, bson_boolean);
