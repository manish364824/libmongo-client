#include "test.h"

static void
test_bson_document (void)
{
  bson_t *b, *e1, *e2;

  e1 = bson_finish
    (bson_append_string
     (bson_append_int32 (bson_new (), "i32", 1984),
      "str", "hello world", -1));

  e2 = bson_finish
    (bson_append_document
     (bson_append_string (bson_new (), "foo", "bar", -1),
      "subd", e1));

  ok (lmc_error_isok (e2),
      "bson_append_document() works");
  bson_free (e1);

  b = bson_finish
    (bson_append_document (bson_new (), "doc", e2));
  ok (lmc_error_isok (b),
      "bson_append_document() works still");
  bson_free (e2);

  cmp_ok (bson_size (b), "==", 69, "BSON document element size check");
  ok (memcmp (bson_data (b),
	      "\105\000\000\000\003\144\157\143\000\073\000\000\000\002\146"
	      "\157\157\000\004\000\000\000\142\141\162\000\003\163\165\142"
	      "\144\000\043\000\000\000\020\151\063\062\000\300\007\000\000"
	      "\002\163\164\162\000\014\000\000\000\150\145\154\154\157\040"
	      "\167\157\162\154\144\000\000\000\000",
	      bson_size (b)) == 0,
      "BSON document element contents check");

  bson_free (b);

  e1 = bson_append_int32 (bson_new (), "foo", 42);

  b = bson_append_document (bson_new (), "doc", e1);
  ok (!lmc_error_isok (b),
      "bson_append_document() with an unfinished document should fail");
  lmc_error_reset (b);
  bson_finish (e1);

  b = bson_append_document (b, NULL, e1);
  ok (lmc_error_get_errn (b) == EINVAL,
      "bson_append_document() with a NULL key should fail");
  lmc_error_reset (b);

  b = bson_append_document (b, "doc", NULL);
  ok (!lmc_error_isok (b),
      "bson_append_document() with a NULL document should fail");
  lmc_error_reset (b);

  ok (bson_append_document (NULL, "doc", e1) == NULL,
      "bson_append_document() without a BSON object should fail");

  bson_finish (b);
  cmp_ok (bson_size (b), "==", 5,
	  "BSON object should be empty");

  b = bson_append_document (b, "doc", e1);
  ok (lmc_error_get_errn (b) == EBUSY,
      "Appending to a finished element should fail");

  bson_free (e1);
  bson_free (b);
}

RUN_TEST (10, bson_document);
