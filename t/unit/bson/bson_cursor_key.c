#include "test.h"

static void
test_bson_cursor_key (void)
{
  bson_t *b;
  bson_cursor_t *c;

  is (bson_cursor_key (NULL), NULL,
      "bson_cursor_key(NULL) should fail");

  b = test_bson_generate_full ();
  c = bson_cursor_new (b);

  is (bson_cursor_key (c), NULL,
      "bson_cursor_key() should fail at the initial position");
  bson_cursor_next (c);

  is (bson_cursor_key (c), "double",
      "bson_cursor_key() works");

  bson_cursor_free (c);
  bson_free (b);
}

RUN_TEST (3, bson_cursor_key);
