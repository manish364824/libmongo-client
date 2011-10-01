#include "test.h"

void
test_bson_regex (void)
{
  bson_t *b;

  b = bson_finish
    (bson_append_regex (bson_new (), "regex", "foo.*bar", "i"));
  ok (lmc_error_isok (b),
      "bson_append_regex() works");

  cmp_ok (bson_size (b), "==", 23, "BSON regex element size check");
  ok (memcmp (bson_data (b),
	      "\027\000\000\000\013\162\145\147\145\170\000\146\157\157\056"
	      "\052\142\141\162\000\151\000\000",
	      bson_size (b)) == 0,
      "BSON regex element contents check");

  bson_free (b);

  b = bson_new ();

  b = bson_append_regex (b, "regex", "foo.*bar", NULL);
  ok (!lmc_error_isok (b),
      "bson_append_regex() without options should fail");
  lmc_error_reset (b);

  b = bson_append_regex (b, "regex", NULL, "i");
  ok (!lmc_error_isok (b),
      "bson_append_regex() without a regex should fail");
  lmc_error_reset (b);

  b = bson_append_regex (b, NULL, "foo.*bar", "i");
  ok (!lmc_error_isok (b),
      "bson_append_regex() should fail without a key name");
  lmc_error_reset (b);

  ok (bson_append_regex (NULL, "regex", "foo.*bar", "i") == NULL,
      "bson_append_regex() should fail without a BSON object");

  bson_finish (b);
  cmp_ok (bson_size (b), "==", 5,
	  "BSON object should be empty");

  b = bson_append_regex (b, "regex", "foo.*bar", "i");
  ok (lmc_error_get_errn (b) == EBUSY,
      "Appending to a finished element should fail");

  bson_free (b);
}

RUN_TEST (9, bson_regex);
