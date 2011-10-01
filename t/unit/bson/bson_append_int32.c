#include "test.h"

static void
test_bson_int32 (void)
{
  bson_t *b;
  int32_t i = 1984;

  b = bson_finish (bson_append_int32 (bson_new (), "i32", i));
  ok (lmc_error_isok (b),
      "bson_append_int32() works");

  cmp_ok (bson_size (b), "==", 14, "BSON int32 element size check");
  ok (memcmp (bson_data (b),
	      "\016\000\000\000\020\151\063\062\000\300\007\000\000\000",
	      bson_size (b)) == 0,
      "BSON int32 element contents check");

  bson_free (b);

  b = bson_new ();

  b = bson_append_int32 (b, NULL, i);
  ok (lmc_error_get_errn (b) == EINVAL,
      "bson_append_int32() with a NULL key should fail");
  lmc_error_reset (b);

  ok (bson_append_int32 (NULL, "i32", i) == NULL,
      "bson_append_int32() without a BSON object should fail");

  bson_finish (b);
  cmp_ok (bson_size (b), "==", 5,
	  "BSON object should be empty");

  b = bson_append_int32 (b, "i32", i);
  ok (lmc_error_get_errn (b) == EBUSY,
      "Appending to a finished element should fail");

  bson_free (b);
}

RUN_TEST (7, bson_int32);
