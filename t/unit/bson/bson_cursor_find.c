#include "test.h"

static void
test_bson_cursor_find (void)
{
  bson_t *b;
  bson_cursor_t *c;

  b = test_bson_generate_full ();

  c = bson_cursor_find (bson_cursor_new (b), "TRUE");
  ok (lmc_error_isok (c),
      "bson_cursor_find() works on a newly created cursor");

  c = bson_cursor_find (c, NULL);
  ok (!lmc_error_isok (c),
      "bson_cursor_find() should fail with a NULL key");
  lmc_error_reset (c);

  ok (bson_cursor_find (NULL, "int32") == NULL,
      "bson_cursor_find() should fail with a NULL cursor");
  lmc_error_reset (c);

  ok (lmc_error_isok (bson_cursor_find (c, "sex")),
      "bson_cursor_find() works");

  ok (lmc_error_isok (bson_cursor_find (c, "str")),
      "bson_cursor_find() should wrap over if neccessary");

  ok (!lmc_error_isok (bson_cursor_find (c, "-invalid-key-")),
      "bson_cursor_find() should fail when the key is not found");
  lmc_error_reset (c);

  ok (lmc_error_isok (bson_cursor_find (c, "int64")),
      "bson_cursor_find() works, even after a previous failure");

  bson_cursor_free (c);
  bson_free (b);
}

RUN_TEST (7, bson_cursor_find);
