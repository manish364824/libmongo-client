#include "test.h"

static void
test_bson_cursor_get_int64 (void)
{
  bson_t *b;
  bson_cursor_t *c;
  int64_t d = (int64_t)987654;

  ok (bson_cursor_get_int64 (NULL, &d) == FALSE,
      "bson_cursor_get_int64() with a NULL cursor fails");

  b = test_bson_generate_full ();
  c = bson_cursor_new (b);

  ok (bson_cursor_get_int64 (c, NULL) == FALSE,
      "bson_cursor_get_int64() with a NULL destination fails");
  ok (bson_cursor_get_int64 (c, &d) == FALSE,
      "bson_cursor_get_int64() at the initial position fails");
  cmp_ok (d, "==", 987654,
	  "destination remains unchanged after failed cursor operations");
  bson_cursor_free (c);

  c = bson_cursor_find (bson_cursor_new (b), "int64");
  ok (bson_cursor_get_int64 (c, &d),
      "bson_cursor_get_int64() works");
  cmp_ok (d, "==", (int64_t)-42,
	  "bson_cursor_get_int64() returns the correct result");

  bson_cursor_free (c);

  c = bson_cursor_find (bson_cursor_new (b), "double");
  ok (bson_cursor_get_int64 (c, &d) == FALSE,
      "bson_cursor_get_int64() should fail when the cursor points to "
      "non-int64 data");

  bson_cursor_free (c);
  bson_free (b);
}

RUN_TEST (7, bson_cursor_get_int64);
