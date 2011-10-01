#include "test.h"

static void
test_bson_js_code_w_scope (void)
{
  bson_t *b, *scope;

  scope = bson_finish (bson_append_string (bson_new (), "foo", "bar", -1));

  /* Test #1: A single JS element, with default size. */
  b = bson_finish
    (bson_append_javascript_w_scope (bson_new (), "f",
				     "alert ('hello');", -1,
				     scope));
  ok (lmc_error_isok (b),
      "bson_append_javascript_w_scope() works");

  cmp_ok (bson_size (b), "==", 51, "BSON javascript w/ element size check");
  ok (memcmp (bson_data (b),
	      "\063\000\000\000\017\146\000\053\000\000\000\021\000\000\000"
	      "\141\154\145\162\164\040\050\047\150\145\154\154\157\047\051"
	      "\073\000\022\000\000\000\002\146\157\157\000\004\000\000\000"
	      "\142\141\162\000\000\000",
	      bson_size (b)) == 0,
      "BSON javascript w/ scope element contents check");
  bson_free (b);

  /* Test #2: A single javascript element, with explicit length. */
  b = bson_finish
    (bson_append_javascript_w_scope (bson_new (), "f",
				     "alert ('hello'); garbage",
				     strlen ("alert ('hello');"),
				     scope));
  ok (lmc_error_isok (b),
      "bson_append_javascript_w_scope() with explicit length works");
  bson_finish (b);

  cmp_ok (bson_size (b), "==", 51, "BSON javascript w/ element size check");
  ok (memcmp (bson_data (b),
	      "\063\000\000\000\017\146\000\053\000\000\000\021\000\000\000"
	      "\141\154\145\162\164\040\050\047\150\145\154\154\157\047\051"
	      "\073\000\022\000\000\000\002\146\157\157\000\004\000\000\000"
	      "\142\141\162\000\000\000",
	      bson_size (b)) == 0,
      "BSON javascript w/ scope element contents check");
  bson_free (b);

  /* Test #3: Negative test, passing an invalid arguments. */
  b = bson_new ();

  b = bson_append_javascript_w_scope (b, "hello", "print();", -42, scope);
  ok (!lmc_error_isok (b),
      "bson_append_javascript_w_scope() with an invalid length should fail");
  lmc_error_reset (b);

  b= bson_append_javascript_w_scope (b, NULL, "print();", -1, scope);
  ok (!lmc_error_isok (b),
      "bson_append_javascript_w_scope() should fail without a key name");
  lmc_error_reset (b);

  b = bson_append_javascript_w_scope (b, "hello", NULL, -1, scope);
  ok (!lmc_error_isok (b),
      "bson_append_javascript_w_scope() should fail without javascript code");
  lmc_error_reset (b);

  b = bson_append_javascript_w_scope (b, "hello", "print();", -1, NULL);
  ok (!lmc_error_isok (b),
      "bson_append_javascript_w_scope() should fail without a scope object");
  lmc_error_reset (b);

  ok (bson_append_javascript_w_scope (NULL, "hello", "print();",
				      -1, scope) == NULL,
      "bson_append_javascript_w_scope() should fail without a BSON object");

  bson_finish (b);
  cmp_ok (bson_size (b), "==", 5,
	  "BSON object should be empty");

  b = bson_append_javascript_w_scope (b, "js", "print();", -1, scope);
  ok (lmc_error_get_errn (b) == EBUSY,
      "Appending to a finished element should fail");

  bson_free (b);
}

RUN_TEST (13, bson_js_code_w_scope);
