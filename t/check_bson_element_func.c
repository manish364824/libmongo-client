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

Suite *
bson_element_suite (void)
{
  Suite *s;
  TCase *tc;

  s = suite_create ("BSON Elements functional tests");

  tc = tcase_create ("Core");
  tcase_add_test (tc, test_func_bson_element_data_move);
  tcase_add_test (tc, test_func_bson_element_data_manip);
  suite_add_tcase (s, tc);

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
