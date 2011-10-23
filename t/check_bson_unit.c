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

START_TEST (test_bson_new_sized)
{
  bson_t *b;

  b = bson_new_sized (1024);
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
  b = bson_close (b);
  ck_assert (b != NULL);
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

START_TEST (test_bson_data)
{
  bson_t *b;

  ck_assert (bson_data_get (NULL) == NULL);
  ck_assert_int_eq (bson_data_get_size (NULL), 0);

  b = bson_new ();

  ck_assert (bson_data_get (b) == NULL);
  ck_assert_int_eq (bson_data_get_size (NULL), 0);

  b = bson_close (b);
  ck_assert (bson_data_get (b) != NULL);
  ck_assert_int_eq (bson_data_get_size (b), 5);

  bson_unref (b);
}
END_TEST

START_TEST (test_bson_add_elements)
{
  bson_t *b;
  bson_element_t *e;

  e = bson_element_create ("hello", BSON_TYPE_STRING,
			   "world", BSON_LENGTH_AUTO);

  b = bson_new ();

  ck_assert (bson_add_elements (NULL, e, BSON_END) == NULL);

  b = bson_add_elements (b, e, bson_element_ref (e), BSON_END);
  ck_assert_int_eq (bson_length (b), 2);

  b = bson_add_elements (b, bson_element_ref (e), BSON_END);
  ck_assert_int_eq (bson_length (b), 3);

  bson_unref (b);
}
END_TEST

START_TEST (test_bson_new_build)
{
  bson_t *b;

  ck_assert (bson_new_build (BSON_END) == NULL);

  b = bson_new_build
    (bson_element_create ("hello", BSON_TYPE_STRING,
			  "world", BSON_LENGTH_AUTO),
     bson_element_create ("answer", BSON_TYPE_INT32, 42),
     BSON_END);

  ck_assert_int_eq (bson_length (b), 2);

  bson_unref (b);
}
END_TEST

START_TEST (test_bson_reset_elements)
{
  bson_t *b;

  ck_assert (bson_reset_elements (NULL) == NULL);

  b = bson_new ();
  ck_assert (bson_reset_elements (b) == b);

  b = bson_add_elements
    (b, bson_element_create ("hello", BSON_TYPE_STRING,
			     "world", BSON_LENGTH_AUTO),
     BSON_END);
  b = bson_close (b);

  ck_assert (bson_reset_elements (b) == b);
  ck_assert_int_eq (bson_length (b), 0);

  bson_unref (b);
}
END_TEST

START_TEST (test_bson_get_set_nth_element)
{
  bson_t *b;
  bson_element_t *e, *ne;

  ck_assert (bson_get_nth_element (NULL, 1) == NULL);

  b = bson_new_build
    (bson_element_create ("hello", BSON_TYPE_STRING,
			  "world", BSON_LENGTH_AUTO),
     bson_element_create ("answer", BSON_TYPE_INT32, 42),
     BSON_END);

  e = bson_get_nth_element (b, 1);
  ck_assert (bson_element_type_get (e) == BSON_TYPE_STRING ||
	     bson_element_type_get (e) == BSON_TYPE_INT32);

  ck_assert (bson_get_nth_element (b, 10) == NULL);

  /* set */
  ne = bson_element_create ("pi", BSON_TYPE_DOUBLE, 3.14);

  ck_assert (bson_set_nth_element (NULL, 1, ne) == NULL);

  b = bson_set_nth_element (b, 10, ne);
  ck_assert (b != NULL);

  b = bson_set_nth_element (b, 1, NULL);
  ck_assert (b != NULL);
  e = bson_get_nth_element (b, 1);
  ck_assert (bson_element_type_get (e) == BSON_TYPE_STRING ||
	     bson_element_type_get (e) == BSON_TYPE_INT32);

  b = bson_set_nth_element (b, 1, ne);
  ck_assert (b != NULL);
  e = bson_get_nth_element (b, 1);
  ck_assert (bson_element_type_get (e) == BSON_TYPE_DOUBLE);

  bson_unref (b);
}
END_TEST

START_TEST (test_bson_key_find)
{
  bson_t *b;
  bson_element_t *hello, *answer, *pi, *e;
  uint32_t h_i, a_i, p_i;

  hello = bson_element_create ("hello", BSON_TYPE_STRING,
			       "world", BSON_LENGTH_AUTO);
  answer = bson_element_create ("answer", BSON_TYPE_INT32, 42);
  pi = bson_element_create ("pi", BSON_TYPE_DOUBLE, 3.14);

  b = bson_new_build (hello, answer, pi, BSON_END);

  ck_assert (bson_key_find (NULL, "hello") == 0);
  ck_assert (bson_key_find (b, "does-not-exist") == 0);
  ck_assert (bson_key_find (b, "hell") == 0);
  ck_assert (bson_key_find (b, "olleh") == 0);
  ck_assert (bson_key_find (b, NULL) == 0);

  h_i = bson_key_find (b, "hello");
  a_i = bson_key_find (b, "answer");
  p_i = bson_key_find (b, "pi");
  ck_assert_int_ne (h_i, 0);
  ck_assert_int_ne (a_i, 0);
  ck_assert_int_ne (p_i, 0);

  ck_assert_int_ne (h_i, a_i);
  ck_assert_int_ne (a_i, p_i);

  e = bson_get_nth_element (b, h_i);
  ck_assert (e == hello);
  e = bson_get_nth_element (b, a_i);
  ck_assert (e == answer);
  e = bson_get_nth_element (b, p_i);
  ck_assert (e == pi);

  bson_unref (b);
}
END_TEST

START_TEST (test_bson_key_get)
{
  bson_t *b;
  bson_element_t *hello, *answer, *pi;

  hello = bson_element_create ("hello", BSON_TYPE_STRING,
			       "world", BSON_LENGTH_AUTO);
  answer = bson_element_create ("answer", BSON_TYPE_INT32, 42);
  pi = bson_element_create ("pi", BSON_TYPE_DOUBLE, 3.14);

  b = bson_new_build (hello, answer, pi, BSON_END);

  ck_assert (bson_key_get (NULL, "hello") == NULL);
  ck_assert (bson_key_get (b, "does-not-exist") == NULL);
  ck_assert (bson_key_get (b, "hell") == NULL);
  ck_assert (bson_key_get (b, "olleh") == NULL);
  ck_assert (bson_key_get (b, NULL) == NULL);

  ck_assert (bson_key_get (b, "hello") == hello);
  ck_assert (bson_key_get (b, "answer") == answer);
  ck_assert (bson_key_get (b, "pi") == pi);

  bson_unref (b);
}
END_TEST

Suite *
bson_suite (void)
{
  Suite *s;

  TCase *tc_core, *tc_manip, *tc_access;

  s = suite_create ("BSON unit tests");

  tc_core = tcase_create ("Core");
  tcase_add_test (tc_core, test_bson_new);
  tcase_add_test (tc_core, test_bson_new_sized);
  tcase_add_test (tc_core, test_bson_ref);
  tcase_add_test (tc_core, test_bson_open_close);
  tcase_add_test (tc_core, test_bson_length);
  tcase_add_test (tc_core, test_bson_data);
  suite_add_tcase (s, tc_core);

  tc_manip = tcase_create ("BSON manipulation");
  tcase_add_test (tc_manip, test_bson_add_elements);
  tcase_add_test (tc_manip, test_bson_new_build);
  tcase_add_test (tc_manip, test_bson_reset_elements);
  tcase_add_test (tc_manip, test_bson_get_set_nth_element);
  suite_add_tcase (s, tc_manip);

  tc_access = tcase_create ("BSON accessors");
  tcase_add_test (tc_access, test_bson_key_find);
  tcase_add_test (tc_access, test_bson_key_get);
  suite_add_tcase (s, tc_access);

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
