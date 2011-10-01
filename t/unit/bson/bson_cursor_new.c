#include "test.h"

static void
test_bson_cursor_new (void)
{
  bson_t *b;
  bson_cursor_t *c;

  ok (bson_cursor_new (NULL) == NULL,
      "bson_cursor_new(NULL) should fail");

  b = bson_new ();
  ok (bson_cursor_new (b) == NULL,
      "bson_cursor_new() should fail with an unfinished BSON object");
  bson_free (b);

  b = test_bson_generate_full ();
  ok ((c = bson_cursor_new (b)) != NULL,
      "bson_cursor_new() works");
  bson_cursor_free (c);
  bson_free (b);
}

RUN_TEST (3, bson_cursor_new);
