#include "test.h"

static void
test_bson_array (void)
{
  bson_t *b, *e1, *e2;

  e1 = bson_finish
    (bson_append_string
     (bson_append_int32
      (bson_new (), "0", 1984),
      "1", "hello world", -1));

  e2 = bson_finish
    (bson_append_array
     (bson_append_string (bson_new (), "0", "bar", -1),
      "1", e1));

  ok (lmc_error_isok (e2), "bson_append_array() works");
  bson_free (e1);

  b = bson_finish
    (bson_append_array (bson_new (), "0", e2));
  ok (lmc_error_isok (b), "bson_append_array() works still");
  bson_free (e2);

  cmp_ok (bson_size (b), "==", 58, "BSON array element size check");
  ok (memcmp (bson_data (b),
	      "\072\000\000\000\004\060\000\062\000\000\000\002\060\000\004"
	      "\000\000\000\142\141\162\000\004\061\000\037\000\000\000\020"
	      "\060\000\300\007\000\000\002\061\000\014\000\000\000\150\145"
	      "\154\154\157\040\167\157\162\154\144\000\000\000\000",
	      bson_size (b)) == 0,
      "BSON array element contents check");

  bson_free (b);

  e1 = bson_append_int32 (bson_new (), "0", 1984);

  b = bson_append_array (bson_new (), "array", e1);
  ok (lmc_error_get_errn (b) == EINVAL,
      "bson_append_array() with an unfinished array should fail");
  lmc_error_reset (b);
  bson_finish (e1);

  b = bson_append_array (b, NULL, e1);
  ok (lmc_error_get_errn (b) == EINVAL,
      "bson_append_array() with a NULL name should fail");
  lmc_error_reset (b);

  b = bson_append_array (b, "foo", NULL);
  ok (lmc_error_get_errn (b) == EINVAL,
      "bson_append_array() with a NULL array should fail");
  lmc_error_reset (b);

  ok (bson_append_array (NULL, "foo", e1) == FALSE,
      "bson_append_array() with a NULL BSON should fail");
  bson_finish (b);
  lmc_error_reset (b);

  cmp_ok (bson_size (b), "==", 5,
	  "BSON object should be empty");

  b = bson_append_array (b, "array", e1);
  ok (lmc_error_get_errn (b) == EBUSY,
      "Appending to a finished element should fail");

  bson_free (e1);
  bson_free (b);
}

RUN_TEST (10, bson_array);
