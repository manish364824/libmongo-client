#include "test.h"

static void
test_f_bson_chained (void)
{
  bson_t *b;

  b = bson_finish
    (bson_append_string
     (bson_append_string (bson_new_sized (1024), "hello", "world", -1),
      "goodbye", "blue sky", -1));
  ok (b && lmc_error_isok (b),
      "Chaining bson commands works");
  bson_free (b);

  b = bson_finish
    (bson_append_string
     (bson_finish (bson_new ()), "hello", "world", -1));
  ok (b && lmc_error_get_errn (b) == EBUSY,
      "Trying to append to a finished element properly propagates EBUSY up");
  bson_free (b);
}

RUN_TEST (2, f_bson_chained);
