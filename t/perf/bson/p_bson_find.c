#include "test.h"

#define MAX_KEYS 1024 * 16

static void
test_p_bson_find (void)
{
  bson_t *b;
  bson_cursor_t *c;
  int32_t i;
  char **keys;
  lmc_bool_t ret = TRUE;

  keys = calloc (MAX_KEYS, sizeof (char *));

  b = bson_new ();
  for (i = 0; i < MAX_KEYS; i++)
    {
      keys[i] = (char *)malloc (128);
      snprintf (keys[i], 127, "tmp_key_%d", i);
      bson_append_int32 (b, keys[i], i);
    }
  bson_finish (b);

  for (i = 0; i < MAX_KEYS; i++)
    {
      int32_t j;

      c = bson_cursor_find (bson_cursor_new (b), keys[i]);
      if (!lmc_error_isok (c))
	ret = FALSE;
      bson_cursor_get_int32 (c, &j);
      if (j != i)
	ret = FALSE;

      bson_cursor_free (c);
      free (keys[i]);
    }

  bson_free (b);
  free (keys);

  ok (ret == TRUE,
      "bson_find() performance test ok");
}

RUN_TEST (1, p_bson_find);
