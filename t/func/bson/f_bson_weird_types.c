#include "test.h"

static void
test_f_bson_weird_types (void)
{
  bson_t *base, *doc;
  bson_cursor_t *c;
  uint8_t type = BSON_TYPE_DBPOINTER;
  int32_t slen, blen;
  uint8_t *data;

  base = bson_finish (bson_append_int32 (bson_new (), "int32", 42));
  data = (uint8_t *)malloc (bson_size (base) + 1024);

  memcpy (data, bson_data (base), bson_size (base));
  blen = bson_size (base) - 1;

  /* Append weird stuff */
  memcpy (data + blen, (const uint8_t *)&type, sizeof (type));
  blen++;

  memcpy (data + blen, (const uint8_t *)"dbpointer", strlen ("dbpointer") + 1);
  blen += strlen ("dbpointer") + 1;

  slen = LMC_INT32_TO_LE (strlen ("refname") + 1);
  memcpy (data + blen, (const uint8_t *)&slen, sizeof (int32_t));
  blen += sizeof (int32_t);

  memcpy (data + blen, (const uint8_t *)"refname", strlen ("refname") + 1);
  blen += strlen ("refname") + 1;

  memcpy (data + blen, (const uint8_t *)"0123456789ABCDEF", 12);
  blen += 12;

  doc = bson_new_from_data (data, blen);
  bson_append_boolean (doc, "Here be dragons?", TRUE);
  bson_finish (doc);

  c = bson_cursor_find (bson_cursor_new (doc), "Here be dragons?");
  ok (lmc_error_isok (c),
      "bson_find() can find elements past unsupported BSON types");
  bson_cursor_free (c);
  bson_free (doc);

  /* Now do it again, but append a type we can't iterate over */
  blen = bson_size (base) - 1;
  memcpy (data, bson_data (base), bson_size (base));

  /* Append BSON_TYPE_NONE */
  type = BSON_TYPE_NONE;

  memcpy (data + blen, (const uint8_t *)&type, sizeof (type));
  blen++;

  memcpy (data + blen, (const uint8_t *)"no-type",
	  strlen ("no-type") + 1);
  blen += strlen ("no-type") + 1;

  memcpy (data + blen, (const uint8_t *)"01234567890ABCDEF", 12);
  blen += 12;

  doc = bson_new_from_data (data, blen);
  bson_append_boolean (doc, "Here be dragons?", TRUE);
  bson_finish (doc);

  c = bson_cursor_find (bson_cursor_new (doc), "Here be dragons?");
  ok (!lmc_error_isok (c),
      "bson_find() should bail out when encountering an invalid element.");
  bson_cursor_free (c);

  c = bson_cursor_new (doc);
  bson_cursor_next (c); /* This will find the first element, and
			   position us there. */
  bson_cursor_next (c); /* This positions after the first element. */
  ok (!lmc_error_isok (bson_cursor_next (c)),
      "bson_cursor_next() should bail out when encountering an invalid element.");
  bson_cursor_free (c);

  bson_free (doc);
  bson_free (base);

  free (data);
}

RUN_TEST (3, f_bson_weird_types);
