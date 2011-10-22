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

Suite *
bson_func_suite (void)
{
  Suite *s;

  TCase *tc_manip;

  s = suite_create ("BSON functional tests");

  tc_manip = tcase_create ("BSON manipulation");
  tcase_add_test (tc_manip, test_func_bson_close);
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
