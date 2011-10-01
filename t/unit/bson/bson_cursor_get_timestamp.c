#include "test.h"

void
test_bson_cursor_get_timestamp (void)
{
  bson_t *b;
  bson_cursor_t *c;
  bson_timestamp_t t;

  t.i = 1;
  t.t = 2;

  ok (bson_cursor_get_timestamp (NULL, &t) == FALSE,
      "bson_cursor_get_timestamp() with a NULL cursor fails");

  b = test_bson_generate_full ();
  c = bson_cursor_new (b);

  ok (bson_cursor_get_timestamp (c, NULL) == FALSE,
      "bson_cursor_get_timestamp() with a NULL destination fails");
  ok (bson_cursor_get_timestamp (c, &t) == FALSE,
      "bson_cursor_get_timestamp() at the initial position fails");
  ok (t.i == 1 && t.t == 2,
	  "destination remains unchanged after failed cursor operations");
  bson_cursor_free (c);

  c = bson_cursor_find (bson_cursor_new (b), "ts");
  ok (bson_cursor_get_timestamp (c, &t),
      "bson_cursor_get_timestamp() works");
  ok (t.i = 10 && t.t == 12345678,
      "bson_cursor_get_timestamp() returns the correct result");

  bson_cursor_next (c);
  ok (bson_cursor_get_timestamp (c, &t) == FALSE,
      "bson_cursor_get_timestamp() should fail when the cursor points to "
      "non-timestamp data");

  bson_cursor_free (c);
  bson_free (b);
}

RUN_TEST (7, bson_cursor_get_timestamp);
