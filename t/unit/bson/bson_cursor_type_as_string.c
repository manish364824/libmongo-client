#include "test.h"

static void
test_bson_cursor_type_as_string (void)
{
  bson_t *b;
  bson_cursor_t *c;

  is (bson_cursor_type_as_string (NULL), NULL,
      "bson_cursor_type_as_string(NULL) should fail");

  b = test_bson_generate_full ();
  c = bson_cursor_new (b);

  is (bson_cursor_type_as_string (c), NULL,
      "bson_cursor_type_as_string() should fail at the initial position");
  bson_cursor_next (c);

  is (bson_cursor_type_as_string (c),
      bson_type_as_string (bson_cursor_type (c)),
      "bson_cursor_type_as_string() works");

  bson_cursor_free (c);
  bson_free (b);
}

RUN_TEST (3, bson_cursor_type_as_string);
