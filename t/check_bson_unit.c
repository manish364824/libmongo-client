#include <check.h>
#include <stdlib.h>

#include <lmc/bson.h>

START_TEST (test_bson_new)
{
  bson_t *b;

  b = bson_new ();
  ck_assert (b != NULL);
  bson_unref (b);
}
END_TEST

START_TEST (test_bson_ref)
{
  bson_t *b;

  ck_assert (bson_ref (NULL) == NULL);
  ck_assert (bson_unref (NULL) == NULL);

  b = bson_new ();
  ck_assert (bson_ref (b) != NULL);

  ck_assert (bson_unref (b) == b);
  ck_assert (bson_unref (b) == NULL);
}
END_TEST

START_TEST (test_bson_open_close)
{
  bson_t *b;

  ck_assert (bson_open (NULL) == NULL);
  ck_assert (bson_close (NULL) == NULL);

  b = bson_new ();

  ck_assert (bson_open (b) == b);
  ck_assert (bson_open (b) == b);
  ck_assert (bson_close (b) == b);
  ck_assert (bson_close (b) == b);

  bson_unref (b);
}
END_TEST

START_TEST (test_bson_length)
{
  bson_t *b;

  ck_assert_int_eq (bson_length (NULL), 0);

  b = bson_new ();
  ck_assert_int_eq (bson_length (b), 0);
  bson_unref (b);
}
END_TEST

START_TEST (test_bson_append)
{
  bson_t *b;
  bson_element_t *e;

  e = bson_element_create ("hello", BSON_TYPE_STRING, "world", -1);

  b = bson_new ();

  ck_assert (bson_append (NULL, e, BSON_END) == NULL);

  b = bson_append (b, e, bson_element_ref (e), BSON_END);
  ck_assert_int_eq (bson_length (b), 2);

  b = bson_append (b, bson_element_ref (e), BSON_END);
  ck_assert_int_eq (bson_length (b), 3);

  bson_unref (b);
}
END_TEST

START_TEST (test_bson_new_build)
{
  bson_t *b;

  ck_assert (bson_new_build (BSON_END) == NULL);

  b = bson_new_build
    (bson_element_create ("hello", BSON_TYPE_STRING, "world", -1),
     bson_element_create ("answer", BSON_TYPE_INT32, 42),
     BSON_END);

  ck_assert_int_eq (bson_length (b), 2);

  bson_unref (b);
}
END_TEST

Suite *
bson_suite (void)
{
  Suite *s;

  TCase *tc_core, *tc_manip;

  s = suite_create ("BSON unit tests");

  tc_core = tcase_create ("Core");
  tcase_add_test (tc_core, test_bson_new);
  tcase_add_test (tc_core, test_bson_ref);
  tcase_add_test (tc_core, test_bson_open_close);
  tcase_add_test (tc_core, test_bson_length);
  suite_add_tcase (s, tc_core);

  tc_manip = tcase_create ("BSON manipulation");
  tcase_add_test (tc_manip, test_bson_append);
  tcase_add_test (tc_manip, test_bson_new_build);
  suite_add_tcase (s, tc_manip);

  return s;
}

int
main (void)
{
  int nfailed;
  Suite *s;
  SRunner *sr;

  s = bson_suite ();
  sr = srunner_create (s);

  srunner_run_all (sr, CK_ENV);
  nfailed = srunner_ntests_failed (sr);
  srunner_free (sr);

  return (nfailed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
