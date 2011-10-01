#include "test.h"

static void
test_bson_oid (void)
{
  bson_t *b;
  bson_oid_t oid;

  memcpy (oid.bytes, "1234567890ab", 12);

  b = bson_finish (bson_append_oid (bson_new (), "_id", &oid));
  ok (lmc_error_isok (b),
      "bson_append_oid() works");

  cmp_ok (bson_size (b), "==", 22, "BSON OID element size check");
  ok (memcmp (bson_data (b),
	      "\026\000\000\000\007\137\151\144\000\061\062\063\064\065\066"
	      "\067\070\071\060\141\142\000",
	      bson_size (b)) == 0,
      "BSON OID element contents check");

  bson_free (b);

  b = bson_new ();

  b = bson_append_oid (b, "_id", NULL);
  ok (!lmc_error_isok (b),
      "bson_append_oid() should fail without an OID");
  lmc_error_reset (b);

  b = bson_append_oid (b, NULL, &oid);
  ok (!lmc_error_isok (b),
      "bson_append_oid() should fail without a key name");
  lmc_error_reset (b);

  ok (bson_append_oid (NULL, "_id", &oid) == NULL,
      "bson_append_oid() should fail without a BSON object");

  bson_finish (b);
  cmp_ok (bson_size (b), "==", 5,
	  "BSON object should be empty");

  b = bson_append_oid (b, "_id", &oid);
  ok (lmc_error_get_errn (b) == EBUSY,
      "Appending to a finished element should fail");

  bson_free (b);
}

RUN_TEST (8, bson_oid);
