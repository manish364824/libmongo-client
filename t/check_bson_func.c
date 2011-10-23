#include <check.h>
#include <stdlib.h>

#include <lmc/bson.h>

START_TEST (test_func_bson_close)
{
  bson_t *b;
  bson_element_t *e;

  b = bson_new ();
  e = bson_element_create ("hello", BSON_TYPE_STRING, "world", -1);

  b = bson_add_elements (b, e, bson_element_ref (e), BSON_END);
  ck_assert_int_eq (bson_length (b), 2);

  b = bson_close (b);
  b = bson_add_elements (b, e, BSON_END);
  ck_assert_int_eq (bson_length (b), 2);

  b = bson_open (b);
  b = bson_add_elements (b, bson_element_ref (e), BSON_END);
  ck_assert_int_eq (bson_length (b), 3);

  bson_unref (b);
}
END_TEST

START_TEST (test_func_bson_flatten)
{
  bson_t *b;
  uint32_t size, size2;

  b = bson_new_build
    (bson_element_create ("hello", BSON_TYPE_STRING, "world", -1),
     bson_element_create ("answer", BSON_TYPE_INT32, 42),
     BSON_END);
  b = bson_close (b);
  size = bson_data_get_size (b);
  _ck_assert_int (size, >, 16);

  b = bson_open (b);
  b = bson_add_elements (b, bson_element_create ("pi", BSON_TYPE_DOUBLE, 3.14),
			 BSON_END);
  b = bson_close (b);
  size2 = bson_data_get_size (b);
  _ck_assert_int (size2, >, size);

  b = bson_reset_elements (b);
  b = bson_add_elements (b, bson_element_create ("pi", BSON_TYPE_DOUBLE, 3.14),
			 BSON_END);
  b = bson_close (b);

  _ck_assert_int (bson_data_get_size (b), <, size2);

  bson_unref (b);
}
END_TEST

Suite *
bson_func_suite (void)
{
  Suite *s;

  TCase *tc_manip;

  s = suite_create ("BSON functional tests");

  tc_manip = tcase_create ("BSON manipulation");
  tcase_add_test (tc_manip, test_func_bson_close);
  tcase_add_test (tc_manip, test_func_bson_flatten);
  suite_add_tcase (s, tc_manip);

  return s;
}

int
main (void)
{
  int nfailed;
  Suite *s;
  SRunner *sr;

  s = bson_func_suite ();
  sr = srunner_create (s);

  srunner_run_all (sr, CK_ENV);
  nfailed = srunner_ntests_failed (sr);
  srunner_free (sr);

  return (nfailed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
