#include "test.h"

static void
test_bson_validate_key (void)
{
  lmc_bool_t valid;

  valid = bson_validate_key (NULL, BSON_VALID_CONTEXT_ALL);
  ok (valid == FALSE && errno == EINVAL,
      "bson_validate_key() sets errno when the key is NULL");

  valid = bson_validate_key ("$foo.bar", BSON_VALID_CONTEXT_ALL);
  ok (valid == TRUE,
      "bson_validate_key() returns success if both checks are off");

  valid = bson_validate_key ("$foo.bar", BSON_VALID_CONTEXT_NO_DOLLAR);
  ok (valid == FALSE,
      "bson_validate_key() returns failiure if the key starts with a $");
  valid = bson_validate_key ("foo.bar$", BSON_VALID_CONTEXT_NO_DOLLAR);
  ok (valid == TRUE,
      "bson_validate_key() returns success if the key does not start with a $");

  valid = bson_validate_key ("foo.bar", BSON_VALID_CONTEXT_NO_DOTS);
  ok (valid == FALSE,
      "bson_validate_key() returns failiure if the key contains a dot");
  valid = bson_validate_key ("foobar", BSON_VALID_CONTEXT_NO_DOTS);
  ok (valid == TRUE,
      "bson_validate_key() returns success if the key does not contain a dot");
}

RUN_TEST (6, bson_validate_key)
