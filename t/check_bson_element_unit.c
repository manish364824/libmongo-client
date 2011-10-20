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

START_TEST (test_bson_element_ref)
{
  bson_element_t *e, *e2;

  e = bson_element_new ();
  e2 = bson_element_ref (e);

  fail_unless (e == e2);

  mark_point ();
  bson_element_unref (e);
  bson_element_unref (e);
}
END_TEST

Suite *
bson_element_suite (void)
{
  Suite *s;
  TCase *tc_creat;

  s = suite_create ("BSON Elements");

  tc_creat = tcase_create ("Constructor / Destructor");
  tcase_add_test (tc_creat, test_bson_element_new);
  tcase_add_test (tc_creat, test_bson_element_ref);
  suite_add_tcase (s, tc_creat);

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
