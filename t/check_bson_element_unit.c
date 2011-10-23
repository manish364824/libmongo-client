#include <check.h>
#include <stdlib.h>

#include <lmc/bson-element.h>

START_TEST (test_bson_element_new)
{
  bson_element_t *e;

  e = bson_element_new ();
  fail_if (e == NULL);
  mark_point ();
  bson_element_unref (e);
}
END_TEST

START_TEST (test_bson_element_new_sized)
{
  bson_element_t *e;

  e = bson_element_new_sized (1024);
  fail_if (e == NULL);
  mark_point ();
  bson_element_unref (e);
}
END_TEST

START_TEST (test_bson_element_ref)
{
  bson_element_t *e, *e2;

  e = bson_element_ref (NULL);
  fail_unless (e == NULL);

  e = bson_element_new ();
  e2 = bson_element_ref (e);

  fail_unless (e == e2);

  fail_if (bson_element_unref (e) == NULL);
  fail_unless (bson_element_unref (e) == NULL);

  fail_unless (bson_element_unref (NULL) == NULL);
}
END_TEST

START_TEST (test_bson_element_type_get)
{
  bson_element_t *e;

  fail_unless (bson_element_type_get (NULL) == BSON_TYPE_NONE);

  e = bson_element_new ();
  fail_unless (bson_element_type_get (e) == BSON_TYPE_NONE);
  bson_element_unref (e);
}
END_TEST

START_TEST (test_bson_element_type_set)
{
  bson_element_t *e;

  fail_unless (bson_element_type_set (NULL, BSON_TYPE_STRING) == NULL);

  e = bson_element_new ();
  fail_unless (bson_element_type_set (e, BSON_TYPE_STRING) == e);
  fail_unless (bson_element_type_get (e) == BSON_TYPE_STRING);
  bson_element_unref (e);
}
END_TEST

START_TEST (test_bson_element_name_set)
{
  bson_element_t *e;

  fail_unless (bson_element_name_set (NULL, "test-name") == NULL);

  e = bson_element_new ();

  e = bson_element_name_set (e, NULL);
  fail_if (e == NULL);

  e = bson_element_name_set (e, "test-name");
  fail_if (e == NULL);

  bson_element_unref (e);
}
END_TEST

START_TEST (test_bson_element_name_get)
{
  bson_element_t *e;

  fail_unless (bson_element_name_get (NULL) == NULL);

  e = bson_element_new ();
  ck_assert_str_eq (bson_element_name_get (e), "");

  e = bson_element_name_set (e, "test-name");
  ck_assert_str_eq (bson_element_name_get (e), "test-name");

  e = bson_element_name_set (e, "foo");
  ck_assert_str_eq (bson_element_name_get (e), "foo");

  e = bson_element_name_set (e, "really-long-name");
  ck_assert_str_eq (bson_element_name_get (e), "really-long-name");

  e = bson_element_name_set (e, NULL);
  ck_assert_str_eq (bson_element_name_get (e), "");
  bson_element_unref (e);
}
END_TEST

START_TEST (test_bson_element_data_set)
{
  bson_element_t *e;
  char *str = "test string";

  fail_unless (bson_element_data_set (NULL, (uint8_t *)str,
				      strlen (str) + 1) == NULL);

  e = bson_element_new ();

  e = bson_element_data_set (e, NULL, 1);
  ck_assert (e != NULL);

  e = bson_element_data_set (e, (uint8_t *)str, 0);
  ck_assert (e != NULL);

  e = bson_element_data_set (e, (uint8_t *)str, strlen (str) + 1);
  ck_assert (e != NULL);

  bson_element_unref (e);
}
END_TEST

START_TEST (test_bson_element_data_reset)
{
  bson_element_t *e;
  char *str = "test string";

  ck_assert (bson_element_data_reset (NULL) == NULL);

  e = bson_element_new ();
  e = bson_element_data_set (e, (uint8_t *)str, strlen (str) + 1);

  e = bson_element_data_reset (e);
  ck_assert_int_eq (bson_element_data_get_size (e), 0);

  bson_element_unref (e);
}
END_TEST

START_TEST (test_bson_element_data_append)
{
  bson_element_t *e;
  char *str1 = "test ", *str2 = "string";

  ck_assert (bson_element_data_append (NULL, (uint8_t *)str1,
				       strlen (str1)) == NULL);

  e = bson_element_new ();

  e = bson_element_data_append (e, (uint8_t *)str1, strlen (str1));
  ck_assert (e != NULL);
  ck_assert_int_eq (bson_element_data_get_size (e), (int32_t)strlen (str1));

  e = bson_element_data_append (e, (uint8_t *)str2, strlen (str2) + 1);
  ck_assert (e != NULL);
  ck_assert_int_eq (bson_element_data_get_size (e),
		    (int32_t)(strlen (str1) + strlen (str2) + 1));

  bson_element_unref (e);
}
END_TEST

START_TEST (test_bson_element_data_get)
{
  bson_element_t *e;
  char *str = "test string";

  fail_unless (bson_element_data_get (NULL) == NULL);
  ck_assert_int_eq (bson_element_data_get_size (NULL), -1);

  e = bson_element_new ();

  ck_assert (bson_element_data_get (e) == NULL);
  ck_assert_int_eq (bson_element_data_get_size (e), 0);

  e = bson_element_data_set (e, (uint8_t *)str, strlen (str) + 1);

  ck_assert_str_eq ((char *)bson_element_data_get (e), str);
  ck_assert_int_eq (bson_element_data_get_size (e), (int32_t)strlen (str) + 1);

  bson_element_data_set (e, NULL, 0);

  ck_assert_int_eq (bson_element_data_get_size (e), 0);

  bson_element_unref (e);
}
END_TEST

START_TEST (test_bson_element_stream)
{
  bson_element_t *e;

  ck_assert (bson_element_stream_get (NULL) == NULL);
  ck_assert (bson_element_stream_get_size (NULL) == -1);

  e = bson_element_new ();

  ck_assert (bson_element_stream_get (e) != NULL);
  ck_assert (bson_element_stream_get_size (e) > 0);

  bson_element_unref (e);
}
END_TEST

START_TEST (test_bson_element_value_double)
{
  bson_element_t *e;
  double d = 3.14, v = 0;

  ck_assert (bson_element_value_set_double (NULL, d) == NULL);
  ck_assert (bson_element_value_get_double (NULL, &v) == FALSE);

  e = bson_element_new ();

  ck_assert (bson_element_value_get_double (e, &v) == FALSE);

  e = bson_element_value_set_double (e, d);
  ck_assert (e != NULL);
  ck_assert (bson_element_type_get (e) == BSON_TYPE_DOUBLE);

  ck_assert (bson_element_value_get_double (e, &v) == TRUE);
  ck_assert (d == v);

  ck_assert (bson_element_value_get_double (e, NULL) == FALSE);

  bson_element_unref (e);
}
END_TEST

START_TEST (test_bson_element_value_int32)
{
  bson_element_t *e;
  int32_t d = 42, v = 0;

  ck_assert (bson_element_value_set_int32 (NULL, d) == NULL);
  ck_assert (bson_element_value_get_int32 (NULL, &v) == FALSE);

  e = bson_element_new ();

  ck_assert (bson_element_value_get_int32 (e, &v) == FALSE);

  e = bson_element_value_set_int32 (e, d);
  ck_assert (e != NULL);
  ck_assert (bson_element_type_get (e) == BSON_TYPE_INT32);

  ck_assert (bson_element_value_get_int32 (e, &v) == TRUE);
  ck_assert (d == v);

  ck_assert (bson_element_value_get_int32 (e, NULL) == FALSE);

  bson_element_unref (e);
}
END_TEST

START_TEST (test_bson_element_value_string)
{
  bson_element_t *e;
  const char *d = "test-string", *v = NULL;

  ck_assert (bson_element_value_set_string (NULL, d,
					    BSON_LENGTH_AUTO) == NULL);
  ck_assert (bson_element_value_get_string (NULL, &v) == FALSE);

  e = bson_element_new ();

  ck_assert (bson_element_value_get_string (e, &v) == FALSE);

  e = bson_element_value_set_string (e, d, BSON_LENGTH_AUTO);
  ck_assert (e != NULL);
  ck_assert (bson_element_type_get (e) == BSON_TYPE_STRING);

  ck_assert (bson_element_value_get_string (e, &v) == TRUE);
  ck_assert_str_eq (d, v);

  e = bson_element_value_set_string (e, d, 4);
  ck_assert (bson_element_value_get_string (e, &v) == TRUE);
  ck_assert_str_eq (v, "test");

  ck_assert (bson_element_value_get_string (e, NULL) == FALSE);

  bson_element_unref (e);
}
END_TEST

START_TEST (test_bson_element_value)
{
  bson_element_t *e;
  const char *str = NULL;
  int32_t i = 0;
  double d = 0;

  ck_assert (bson_element_value_set (NULL, BSON_TYPE_INT32, 42) == NULL);

  e = bson_element_new ();

  e = bson_element_value_set (e, BSON_TYPE_MAX);
  ck_assert (bson_element_type_get (e) == BSON_TYPE_NONE);
  e = bson_element_value_set (e, 42);
  ck_assert (bson_element_type_get (e) == BSON_TYPE_NONE);

  e = bson_element_value_set (e, BSON_TYPE_STRING, "hello world",
			      strlen ("hello world"));
  ck_assert (e != NULL);
  ck_assert (bson_element_type_get (e) == BSON_TYPE_STRING);
  ck_assert (bson_element_value_get_string (e, &str) == TRUE);
  ck_assert_str_eq (str, "hello world");

  e = bson_element_value_set (e, BSON_TYPE_INT32, 42);
  ck_assert (bson_element_type_get (e) == BSON_TYPE_INT32);
  ck_assert (bson_element_value_get_int32 (e, &i) == TRUE);
  ck_assert_int_eq (i, 42);

  e = bson_element_value_set (e, BSON_TYPE_DOUBLE, 3.14);
  ck_assert (bson_element_type_get (e) == BSON_TYPE_DOUBLE);
  ck_assert (bson_element_value_get_double (e, &d) == TRUE);
  ck_assert_int_eq (d, 3.14);

  bson_element_unref (e);
}
END_TEST

START_TEST (test_bson_element_set)
{
  bson_element_t *e;
  int32_t i = 0;

  ck_assert (bson_element_set (NULL, "test-field",
			       BSON_TYPE_INT32, 42) == NULL);

  e = bson_element_new ();

  e = bson_element_set (e, "test-field", BSON_TYPE_INT32, 42);
  ck_assert_str_eq (bson_element_name_get (e), "test-field");
  ck_assert (bson_element_type_get (e) == BSON_TYPE_INT32);
  ck_assert (bson_element_value_get_int32 (e, &i) == TRUE);
  ck_assert_int_eq (i, 42);

  bson_element_unref (e);
}
END_TEST

START_TEST (test_bson_element_create)
{
  bson_element_t *e;
  int32_t i = 0;

  ck_assert (bson_element_create ("test-field",
				  BSON_TYPE_UNDEFINED) == NULL);

  e = bson_element_create ("test-field", BSON_TYPE_INT32, 42);
  ck_assert_str_eq (bson_element_name_get (e), "test-field");
  ck_assert (bson_element_type_get (e) == BSON_TYPE_INT32);
  ck_assert (bson_element_value_get_int32 (e, &i) == TRUE);
  ck_assert_int_eq (i, 42);

  bson_element_unref (e);
}
END_TEST

static void
_test_copy (bson_element_t *orig)
{
  bson_element_t *copy;
  uint32_t size;

  copy = bson_element_new_from_data (bson_element_stream_get (orig));
  ck_assert (copy != NULL);
  ck_assert (bson_element_type_get (orig) == bson_element_type_get (copy));
  ck_assert_int_eq (bson_element_stream_get_size (orig),
		    bson_element_stream_get_size (copy));
  size = bson_element_stream_get_size (orig);
  ck_assert (memcmp (bson_element_stream_get (orig),
		     bson_element_stream_get (copy),
		     size) == 0);

  bson_element_unref (orig);
  bson_element_unref (copy);
}

START_TEST (test_bson_element_new_from_data)
{
  bson_element_t *e;

  ck_assert (bson_element_new_from_data (NULL) == NULL);

  _test_copy
    (bson_element_create ("hello", BSON_TYPE_STRING,
			  "world", BSON_LENGTH_AUTO));
  _test_copy
    (bson_element_create ("answer", BSON_TYPE_INT32, 42));
  _test_copy
    (bson_element_create ("pi", BSON_TYPE_DOUBLE, 3.14));

  e = bson_element_new ();
  e = bson_element_type_set (e, 142);
  e = bson_element_name_set (e, "invalid-type");
  ck_assert (bson_element_new_from_data (bson_element_stream_get (e)) == NULL);

  e = bson_element_type_set (e, 0);
  ck_assert (bson_element_new_from_data (bson_element_stream_get (e)) == NULL);

  bson_element_unref (e);
}
END_TEST

START_TEST (test_bson_element_name_validate)
{
  ck_assert (bson_element_name_validate (NULL, BSON_ELEMENT_NAME_OK) == FALSE);

  ck_assert (bson_element_name_validate
	     ("simple", BSON_ELEMENT_NAME_OK) == TRUE);
  ck_assert (bson_element_name_validate
	     ("simple", BSON_ELEMENT_NAME_FORBID_DOTS) == TRUE);
  ck_assert (bson_element_name_validate
	     ("simple", BSON_ELEMENT_NAME_FORBID_DOLLAR) == TRUE);
  ck_assert (bson_element_name_validate
	     ("simple", BSON_ELEMENT_NAME_STRICT) == TRUE);

  ck_assert (bson_element_name_validate
	     ("dotted.notation", BSON_ELEMENT_NAME_OK) == TRUE);
  ck_assert (bson_element_name_validate
	     ("dotted.notation", BSON_ELEMENT_NAME_FORBID_DOTS) == FALSE);
  ck_assert (bson_element_name_validate
	     ("dotted.notation", BSON_ELEMENT_NAME_FORBID_DOLLAR) == TRUE);
  ck_assert (bson_element_name_validate
	     ("dotted.notation", BSON_ELEMENT_NAME_STRICT) == FALSE);

  ck_assert (bson_element_name_validate
	     ("$foo", BSON_ELEMENT_NAME_OK) == TRUE);
  ck_assert (bson_element_name_validate
	     ("$foo", BSON_ELEMENT_NAME_FORBID_DOTS) == TRUE);
  ck_assert (bson_element_name_validate
	     ("$foo", BSON_ELEMENT_NAME_FORBID_DOLLAR) == FALSE);
  ck_assert (bson_element_name_validate
	     ("$foo", BSON_ELEMENT_NAME_STRICT) == FALSE);

  ck_assert (bson_element_name_validate
	     ("foo$bar", BSON_ELEMENT_NAME_OK) == TRUE);
  ck_assert (bson_element_name_validate
	     ("foo$bar", BSON_ELEMENT_NAME_FORBID_DOTS) == TRUE);
  ck_assert (bson_element_name_validate
	     ("foo$bar", BSON_ELEMENT_NAME_FORBID_DOLLAR) == TRUE);
  ck_assert (bson_element_name_validate
	     ("foo$bar", BSON_ELEMENT_NAME_STRICT) == TRUE);
}
END_TEST

Suite *
bson_element_suite (void)
{
  Suite *s;
  TCase *tc_core, *tc_access, *tc_builder;

  s = suite_create ("BSON Elements unit tests");

  tc_core = tcase_create ("Core");
  tcase_add_test (tc_core, test_bson_element_new);
  tcase_add_test (tc_core, test_bson_element_new_sized);
  tcase_add_test (tc_core, test_bson_element_ref);
  tcase_add_test (tc_core, test_bson_element_type_get);
  tcase_add_test (tc_core, test_bson_element_type_set);
  tcase_add_test (tc_core, test_bson_element_name_set);
  tcase_add_test (tc_core, test_bson_element_name_get);
  tcase_add_test (tc_core, test_bson_element_name_validate);
  tcase_add_test (tc_core, test_bson_element_data_set);
  tcase_add_test (tc_core, test_bson_element_data_reset);
  tcase_add_test (tc_core, test_bson_element_data_append);
  tcase_add_test (tc_core, test_bson_element_data_get);
  tcase_add_test (tc_core, test_bson_element_stream);
  suite_add_tcase (s, tc_core);

  tc_access = tcase_create ("Accessors");
  tcase_add_test (tc_access, test_bson_element_value_double);
  tcase_add_test (tc_access, test_bson_element_value_int32);
  tcase_add_test (tc_access, test_bson_element_value_string);
  suite_add_tcase (s, tc_access);

  tc_builder = tcase_create ("Builder");
  tcase_add_test (tc_builder, test_bson_element_value);
  tcase_add_test (tc_builder, test_bson_element_set);
  tcase_add_test (tc_builder, test_bson_element_create);
  tcase_add_test (tc_builder, test_bson_element_new_from_data);
  suite_add_tcase (s, tc_builder);

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
