#include <check.h>
#include <stdlib.h>

#include <lmc/bson-element.h>

START_TEST (test_func_bson_element)
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

Suite *
bson_element_suite (void)
{
  Suite *s;
  TCase *tc;

  s = suite_create ("BSON Elements");

  tc = tcase_create ("Functional");
  tcase_add_test (tc, test_func_bson_element);
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
