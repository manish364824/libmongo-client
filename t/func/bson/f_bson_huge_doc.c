#include "test.h"

#ifndef HUGE_DOC_SIZE
#define HUGE_DOC_SIZE (1024 * 1024)
#endif

static void
test_bson_huge_doc (void)
{
  bson_t *b, *s;
  bson_cursor_t *c;
  char *buffer;
  int32_t ds1;

  buffer = (char *)malloc (HUGE_DOC_SIZE);
  memset (buffer, 'a', HUGE_DOC_SIZE);
  buffer[HUGE_DOC_SIZE - 1] = '\0';

  b = bson_new ();
  bson_append_int32 (b, "preamble", 1);
  bson_append_string (b, "huge", buffer, -1);
  bson_append_int32 (b, "post", 1234);
  bson_finish (b);
  ds1 = bson_size (b);

  free (buffer);

  s = bson_new ();
  bson_append_document (s, "hugedoc", b);
  bson_finish (s);
  bson_free (b);

  cmp_ok (bson_size (s), ">", ds1,
	  "Document embedding another huge one, has bigger size");

  c = bson_cursor_find (bson_cursor_new (s), "hugedoc");

  bson_cursor_get_document (c, &b);

  cmp_ok (bson_size (b), "==", ds1,
	  "The embedded document has the correct, huge size");

  bson_cursor_free (c);
  bson_free (s);
  bson_free (b);
}

RUN_TEST (2, bson_huge_doc);
