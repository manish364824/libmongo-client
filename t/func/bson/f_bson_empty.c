#include "test.h"

void
test_f_bson_empty (void)
{
  bson_t *b;

  b = bson_new ();
  bson_finish (b);

  cmp_ok (bson_size (b), "==", 5, "Empty BSON size check");
  ok (memcmp (bson_data (b), "\005\000\000\000\000", bson_size (b)) == 0,
      "Empty BSON contents check");

  bson_free (b);
}

RUN_TEST (2, f_bson_empty)
