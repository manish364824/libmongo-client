#include "test.h"

static void
test_bson_cursor_get_array (void)
{
  bson_t *b, *a = NULL;
  bson_cursor_t *c;

  ok (bson_cursor_get_array (NULL, &a) == FALSE,
      "bson_cursor_get_array() with a NULL cursor fails");

  b = test_bson_generate_full ();
  c = bson_cursor_new (b);

  ok (bson_cursor_get_array (c, NULL) == FALSE,
      "bson_cursor_get_array() with a NULL destination fails");
  ok (bson_cursor_get_array (c, &a) == FALSE,
      "bson_cursor_get_array() at the initial position fails");
  ok (a == NULL,
      "destination remains unchanged after failed cursor operations");
  bson_cursor_free (c);

  c = bson_cursor_find (bson_cursor_new (b), "array");
  ok (bson_cursor_get_array (c, &a),
      "bson_cursor_get_array() works");
  cmp_ok (bson_size (a), ">", 0,
	  "the returned document is finished");
  bson_free (a);

  bson_cursor_next (c);

  ok (bson_cursor_get_array (c, &a) == FALSE,
      "bson_cursor_get_array() fails if the cursor points to "
      "non-array data");

  bson_cursor_free (c);
  bson_free (b);
}

RUN_TEST (7, bson_cursor_get_array);
