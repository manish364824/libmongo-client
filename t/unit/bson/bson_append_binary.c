#include "test.h"

static void
test_bson_binary (void)
{
  bson_t *b;

  b = bson_new ();

  b = bson_append_binary (b, "binary0", BSON_BINARY_SUBTYPE_GENERIC,
			  (uint8_t *)"foo\0bar", 7);
  ok (lmc_error_isok (b),
      "bson_append_binary(), type 0 works");

  b = bson_append_binary (b, "binary2", BSON_BINARY_SUBTYPE_BINARY,
			  (uint8_t *)"\0\0\0\7foo\0bar", 11);
  ok (lmc_error_isok (b),
      "bson_append_binary(), type 2 works");
  bson_finish (b);

  cmp_ok (bson_size (b), "==", 51, "BSON binary element size check");
  ok (memcmp (bson_data (b),
	      "\063\000\000\000\005\142\151\156\141\162\171\060\000\007\000"
	      "\000\000\000\146\157\157\000\142\141\162\005\142\151\156\141"
	      "\162\171\062\000\013\000\000\000\002\000\000\000\007\146\157"
	      "\157\000\142\141\162\000",
	      bson_size (b)) == 0,
      "BSON binary element contents check");

  bson_free (b);

  b = bson_new ();

  b = bson_append_binary (b, NULL, BSON_BINARY_SUBTYPE_GENERIC,
			  (uint8_t *)"foo\0bar", 7);
  ok (lmc_error_get_errn (b) == EINVAL,
      "bson_append_binary() without a key name should fail");
  lmc_error_reset (b);

  b = bson_append_binary (b, "binary1", BSON_BINARY_SUBTYPE_GENERIC,
			  NULL, 10);
  ok (lmc_error_get_errn (b) == EINVAL,
      "bson_append_binary () without binary data should fail");
  lmc_error_reset (b);

  b = bson_append_binary (b, "binary3", BSON_BINARY_SUBTYPE_GENERIC,
			  (uint8_t *)"foo\0bar", -1);
  ok (lmc_error_get_errn (b) == ERANGE,
      "bson_append_binary () with an invalid length should fail");
  lmc_error_reset (b);

  ok (bson_append_binary (NULL, "binary1", BSON_BINARY_SUBTYPE_GENERIC,
			  (uint8_t *)"foo\0bar", 7) == NULL,
      "bson_append_binary () without a BSON object should fail");

  bson_finish (b);
  cmp_ok (bson_size (b), "==", 5,
	  "BSON object should be empty");

  b = bson_append_binary (b, "binary", BSON_BINARY_SUBTYPE_GENERIC,
			  (uint8_t *)"foo\0bar", 7);
  ok (lmc_error_get_errn (b) == EBUSY,
      "Appending to a finished element should fail");

  bson_free (b);
}

RUN_TEST (10, bson_binary);
