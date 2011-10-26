#include <check.h>
#include <stdlib.h>

#include <lmc/bson-element.h>

START_TEST (test_func_bson_element_data_move)
{
  bson_element_t *e;
  char *v1 = "test-value";
  char *v2 = "foo";

  e = bson_element_new ();
  e = bson_element_type_set (e, BSON_TYPE_STRING);
  e = bson_element_name_set (e, "test-string");

  e = bson_element_data_set (e, (uint8_t *)v1, strlen (v1) + 1);
  ck_assert_str_eq ((char *)bson_element_data_get (e), v1);
  ck_assert_int_eq (bson_element_data_get_size (e), (int32_t)strlen (v1) + 1);

  e = bson_element_data_set (e, (uint8_t *)v2, strlen (v2) + 1);
  ck_assert_str_eq ((char *)bson_element_data_get (e), v2);
  ck_assert_int_eq (bson_element_data_get_size (e), (int32_t)strlen (v2) + 1);

  e = bson_element_name_set (e, "quux");
  ck_assert_str_eq ((char *)bson_element_data_get (e), v2);
  ck_assert_int_eq (bson_element_data_get_size (e), (int32_t)strlen (v2) + 1);

  e = bson_element_name_set (e, "really-long-field-name");
  ck_assert_str_eq ((char *)bson_element_data_get (e), v2);
  ck_assert_int_eq (bson_element_data_get_size (e), (int32_t)strlen (v2) + 1);

  bson_element_unref (e);
}
END_TEST

START_TEST (test_func_bson_element_data_manip)
{
  bson_element_t *e1, *e2;
  char *v1 = "test ", *v2 = "value", *v = "test value";

  e1 = bson_element_new ();
  e1 = bson_element_type_set (e1, BSON_TYPE_STRING);
  e1 = bson_element_name_set (e1, "test-name");

  e2 = bson_element_new ();
  e2 = bson_element_type_set (e2, BSON_TYPE_STRING);
  e2 = bson_element_name_set (e2, "test-name");

  e1 = bson_element_data_append (e1, (uint8_t *)v1, strlen (v1));
  e1 = bson_element_data_append (e1, (uint8_t *)v2, strlen (v2) + 1);

  e2 = bson_element_data_append (e2, (uint8_t *)v, strlen (v) + 1);

  ck_assert_int_eq (bson_element_data_get_size (e1),
		    bson_element_data_get_size (e2));
  ck_assert_str_eq ((char *)bson_element_data_get (e1),
		    (char *)bson_element_data_get (e2));

  bson_element_unref (e1);
  bson_element_unref (e2);
}
END_TEST

START_TEST (test_func_bson_element_value_manip)
{
  bson_element_t *e;
  double d;
  const char *s;

  e = bson_element_new ();

  e = bson_element_type_set (e, BSON_TYPE_DOUBLE);
  ck_assert (bson_element_value_get_double (e, &d) == FALSE);

  e = bson_element_type_set (e, BSON_TYPE_STRING);
  ck_assert (bson_element_value_get_string (e, &s) == FALSE);

  bson_element_unref (e);
}
END_TEST

static void
_bson_element_test_stream (bson_element_t *e, int32_t size,
			   char *data)
{
  bson_element_t *n;

  ck_assert (e != NULL);

  ck_assert_int_eq (bson_element_stream_get_size (e), size);
  ck_assert (memcmp (bson_element_stream_get (e),
		     data, size) == 0 );

  n = bson_element_new_from_data (bson_element_stream_get (e));
  ck_assert (bson_element_type_get (e) == bson_element_type_get (n));
  ck_assert_str_eq (bson_element_name_get (e), bson_element_name_get (n));
  ck_assert (memcmp (bson_element_data_get (e),
		     bson_element_data_get (n),
		     bson_element_data_get_size (n)) == 0);

  bson_element_unref (n);
  bson_element_unref (e);
}

START_TEST (test_func_bson_element_double)
{
  _bson_element_test_stream
    (bson_element_create ("d", BSON_TYPE_DOUBLE, 3.14), 11,
     "\x01\x64\x00\x1f\x85\xeb\x51\xb8\x1e\x09\x40");
}
END_TEST

START_TEST (test_func_bson_element_int32)
{
  _bson_element_test_stream
    (bson_element_create ("i", BSON_TYPE_INT32, 42), 7,
     "\x10\x69\x00\x2a\x00\x00\x00");
}
END_TEST

START_TEST (test_func_bson_element_string)
{
  _bson_element_test_stream
    (bson_element_create ("s", BSON_TYPE_STRING, "hello", BSON_LENGTH_AUTO),
     13,
     "\x02\x73\x00\x06\x00\x00\x00" "hello" "\x00");
}
END_TEST

START_TEST (test_func_bson_element_boolean)
{
  _bson_element_test_stream
    (bson_element_create ("b", BSON_TYPE_BOOLEAN, TRUE),
     4,
     "\x08" "b\x00\x01");
}
END_TEST

START_TEST (test_func_bson_element_null)
{
  _bson_element_test_stream
    (bson_element_create ("null", BSON_TYPE_NULL),
     6,
     "\x0a" "null\x00");
}
END_TEST

START_TEST (test_func_bson_element_int64)
{
  _bson_element_test_stream
    (bson_element_create ("i64", BSON_TYPE_INT64, (int64_t)42),
     13,
     "\x12" "i64\x00" "\x2a\x00\x00\x00\x00\x00\x00\x00");
}
END_TEST

START_TEST (test_func_bson_element_datetime)
{
  _bson_element_test_stream
    (bson_element_create ("date", BSON_TYPE_UTC_DATETIME, (int64_t)42),
     14,
     "\x09" "date\x00" "\x2a\x00\x00\x00\x00\x00\x00\x00");
}
END_TEST

START_TEST (test_func_bson_element_js_code)
{
  _bson_element_test_stream
    (bson_element_create ("js", BSON_TYPE_JS_CODE,
			  "alert('hello world!')", BSON_LENGTH_AUTO),
     30,
     "\x0d" "js\x00" "\x16\x00\x00\x00" "alert('hello world!')\x00");
}
END_TEST

Suite *
bson_element_suite (void)
{
  Suite *s;
  TCase *tc_core, *tc_value, *tc_enc;

  s = suite_create ("BSON Elements functional tests");

  tc_core = tcase_create ("Core");
  tcase_add_test (tc_core, test_func_bson_element_data_move);
  tcase_add_test (tc_core, test_func_bson_element_data_manip);
  suite_add_tcase (s, tc_core);

  tc_enc = tcase_create ("Encoding");
  tcase_add_test (tc_enc, test_func_bson_element_double);
  tcase_add_test (tc_enc, test_func_bson_element_int32);
  tcase_add_test (tc_enc, test_func_bson_element_int64);
  tcase_add_test (tc_enc, test_func_bson_element_string);
  tcase_add_test (tc_enc, test_func_bson_element_js_code);
  tcase_add_test (tc_enc, test_func_bson_element_boolean);
  tcase_add_test (tc_enc, test_func_bson_element_null);
  tcase_add_test (tc_enc, test_func_bson_element_datetime);
  suite_add_tcase (s, tc_enc);

  tc_value = tcase_create ("Manipulations");
  tcase_add_test (tc_value, test_func_bson_element_value_manip);
  suite_add_tcase (s, tc_value);

  return s;
}

int
main (void)
{
  int nfailed;
  Suite *s;
  SRunner *sr;

  s = bson_element_suite ();
  sr = srunner_create (s);

  srunner_run_all (sr, CK_ENV);
  nfailed = srunner_ntests_failed (sr);
  srunner_free (sr);

  return (nfailed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
