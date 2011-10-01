#include "test.h"

static void
test_bson_cursor_next (void)
{
  bson_t *b;
  bson_cursor_t *c;

  ok (bson_cursor_next (NULL) == NULL,
      "bson_cursor_next (NULL) should fail");

  b = bson_finish (bson_new ());
  c = bson_cursor_new (b);

  ok (!lmc_error_isok (bson_cursor_next (c)),
      "bson_cursor_next() should fail with an empty document");

  bson_cursor_free (c);
  bson_free (b);

  b = test_bson_generate_full ();
  c = bson_cursor_new (b);

  ok (lmc_error_isok (bson_cursor_next (c)),
      "initial bson_cursor_next() works");
  ok (lmc_error_isok (bson_cursor_next (c)),
      "subsequent bson_cursor_next() works too");

  while (lmc_error_isok (bson_cursor_next (c))) ;
  lmc_error_reset (c);

  ok (!lmc_error_isok (bson_cursor_next (c)),
      "bson_cursor_next() fails after the end of the BSON object");

  bson_cursor_free (c);
  bson_free (b);
}

RUN_TEST (5, bson_cursor_next);
