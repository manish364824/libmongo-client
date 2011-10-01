#include "test.h"

static void
test_bson_new (void)
{
  bson_t *b;
  const uint8_t *data;
  int32_t size;

  /* bson_new() */
  b = bson_new ();

  ok (b != NULL, "bson_new() works");

  data = bson_data (b);
  ok (data == NULL && errno == EAGAIN,
      "bson_data() with an unifinished object should fail");
  errno = 0;

  size = bson_size (b);
  ok (size == -1 && errno == EAGAIN,
      "bson_size() with an unfinished object should fail");

  b = bson_finish (b);
  ok (b != NULL && lmc_error_isok (b),
      "bson_finish() works");
  b = bson_finish (b);
  ok (b != NULL && lmc_error_isok (b),
      "bson_finish() works on an already finished object too");

  bson_free (b);

  /* bson_new_sized() */
  b = bson_new_sized (120);
  ok (b != NULL, "bson_new_sized() works");

  b = bson_finish (b);
  ok (b != NULL && lmc_error_isok (b) && bson_size (b) == 5,
      "bson_finish() works on a bson_new_sized() object too");

  bson_free (b);

  /* meta-data accessor negative tests */
  ok (bson_size (NULL) == -1,
      "bson_size(NULL) works correctly");
  ok (bson_data (NULL) == NULL,
      "bson_data(NULL) works correctly");
  ok (bson_finish (NULL) == NULL,
      "bson_finish(NULL) works correctly");
  ok (bson_free (NULL) == NULL,
      "bson_free(NULL) works correctly");
}

RUN_TEST (11, bson_new);
