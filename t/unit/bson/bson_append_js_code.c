#include "test.h"

static void
test_bson_js_code (void)
{
  bson_t *b;

  /* Test #1: A single JS element, with default size. */
  b = bson_append_javascript (bson_new (), "hello",
			      "function () { print (\"hello world!\"); }", -1);
  bson_finish (b);
  ok (lmc_error_isok (b),
      "bson_append_javascript() works");

  cmp_ok (bson_size (b), "==", 56, "BSON javascript element size check");
  ok (memcmp (bson_data (b),
	      "\070\000\000\000\015\150\145\154\154\157\000\050\000\000\000"
	      "\146\165\156\143\164\151\157\156\040\050\051\040\173\040\160"
	      "\162\151\156\164\040\050\042\150\145\154\154\157\040\167\157"
	      "\162\154\144\041\042\051\073\040\175\000\000",
	      bson_size (b)) == 0,
      "BSON javascript element contents check");
  bson_free (b);

  /* Test #2: A single javascript element, with explicit length. */
  b = bson_append_javascript (bson_new (), "hello",
			      "print (\"hello world!\"); garbage is gone.",
			      strlen ("print (\"hello world!\");"));
  bson_finish (b);

  ok (lmc_error_isok (b),
    "bson_append_javascript() with explicit length works");
  bson_finish (b);

  cmp_ok (bson_size (b), "==", 40, "BSON javascript element size check, #2");
  ok (memcmp (bson_data (b),
	      "\050\000\000\000\015\150\145\154\154\157\000\030\000\000\000"
	      "\160\162\151\156\164\040\050\042\150\145\154\154\157\040\167"
	      "\157\162\154\144\041\042\051\073\000\000",
	      bson_size (b)) == 0,
      "BSON javascript element contents check, #2");
  bson_free (b);

  /* Test #3: Negative test, passing an invalid arguments. */
  b = bson_new ();

  b = bson_append_javascript (b, "hello", "print();", -42);
  ok (!lmc_error_isok (b),
      "bson_append_javascript() with an invalid length should fail");
  lmc_error_reset (b);

  b = bson_append_javascript (b, NULL, "print();", -1);
  ok (!lmc_error_isok (b),
      "bson_append_javascript() should fail without a key name");
  lmc_error_reset (b);

  b = bson_append_javascript (b, "hello", NULL, -1);
  ok (!lmc_error_isok (b),
      "bson_append_javascript() should fail without javascript code");
  lmc_error_reset (b);

  ok (bson_append_javascript (NULL, "hello", "print();", -1) == NULL,
      "bson_append_javascript() should fail without a BSON object");

  bson_finish (b);
  cmp_ok (bson_size (b), "==", 5,
	  "BSON object should be empty");

  b = bson_append_javascript (b, "js", "print();", -1);
  ok (lmc_error_get_errn (b) == EBUSY,
      "Appending to a finished element should fail");

  bson_free (b);
}

RUN_TEST (12, bson_js_code);
