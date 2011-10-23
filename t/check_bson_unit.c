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

START_TEST (test_bson_stream_open_close)
{
  bson_t *b;

  ck_assert (bson_stream_open (NULL) == NULL);
  ck_assert (bson_stream_close (NULL) == NULL);

  b = bson_new ();

  ck_assert (bson_stream_open (b) == b);
  ck_assert (bson_stream_open (b) == b);
  b = bson_stream_close (b);
  ck_assert (b != NULL);
  ck_assert (bson_stream_close (b) == b);

  bson_unref (b);
}
END_TEST

START_TEST (test_bson_elements_length)
{
  bson_t *b;

  ck_assert_int_eq (bson_elements_length (NULL), 0);

  b = bson_new ();
  ck_assert_int_eq (bson_elements_length (b), 0);
  bson_unref (b);
}
END_TEST

START_TEST (test_bson_stream)
{
  bson_t *b;

  ck_assert (bson_stream_get_data (NULL) == NULL);
  ck_assert_int_eq (bson_stream_get_size (NULL), 0);

  b = bson_new ();

  ck_assert (bson_stream_get_data (b) == NULL);
  ck_assert_int_eq (bson_stream_get_size (NULL), 0);

  b = bson_stream_close (b);
  ck_assert (bson_stream_get_data (b) != NULL);
  ck_assert_int_eq (bson_stream_get_size (b), 5);

  bson_unref (b);
}
END_TEST

START_TEST (test_bson_elements_add)
{
  bson_t *b;
  bson_element_t *e;

  e = bson_element_create ("hello", BSON_TYPE_STRING,
			   "world", BSON_LENGTH_AUTO);

  b = bson_new ();

  ck_assert (bson_elements_add (NULL, e, BSON_END) == NULL);

  b = bson_elements_add (b, e, bson_element_ref (e), BSON_END);
  ck_assert_int_eq (bson_elements_length (b), 2);

  b = bson_elements_add (b, bson_element_ref (e), BSON_END);
  ck_assert_int_eq (bson_elements_length (b), 3);

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

  ck_assert_int_eq (bson_elements_length (b), 2);

  bson_unref (b);
}
END_TEST

START_TEST (test_bson_elements_reset)
{
  bson_t *b;

  ck_assert (bson_elements_reset (NULL) == NULL);

  b = bson_new ();
  ck_assert (bson_elements_reset (b) == b);

  b = bson_elements_add
    (b, bson_element_create ("hello", BSON_TYPE_STRING,
			     "world", BSON_LENGTH_AUTO),
     BSON_END);
  b = bson_stream_close (b);

  ck_assert (bson_elements_reset (b) == b);
  ck_assert_int_eq (bson_elements_length (b), 0);

  bson_unref (b);
}
END_TEST

START_TEST (test_bson_elements_nth_get_set)
{
  bson_t *b;
  bson_element_t *e, *ne;

  ck_assert (bson_elements_nth_get (NULL, 1) == NULL);

  b = bson_new_build
    (bson_element_create ("hello", BSON_TYPE_STRING,
			  "world", BSON_LENGTH_AUTO),
     bson_element_create ("answer", BSON_TYPE_INT32, 42),
     BSON_END);

  e = bson_elements_nth_get (b, 1);
  ck_assert (bson_element_type_get (e) == BSON_TYPE_STRING ||
	     bson_element_type_get (e) == BSON_TYPE_INT32);

  ck_assert (bson_elements_nth_get (b, 10) == NULL);
  ck_assert (bson_elements_nth_get (b, 0) == NULL);

  /* set */
  ne = bson_element_create ("pi", BSON_TYPE_DOUBLE, 3.14);

  ck_assert (bson_elements_nth_set (NULL, 1, ne) == NULL);

  b = bson_elements_nth_set (b, 0, ne);
  ck_assert (b != NULL);

  b = bson_elements_nth_set (b, 10, ne);
  ck_assert (b != NULL);

  b = bson_elements_nth_set (b, 1, NULL);
  ck_assert (b != NULL);
  e = bson_elements_nth_get (b, 1);
  ck_assert (bson_element_type_get (e) == BSON_TYPE_STRING ||
	     bson_element_type_get (e) == BSON_TYPE_INT32);

  b = bson_elements_nth_set (b, 1, ne);
  ck_assert (b != NULL);
  e = bson_elements_nth_get (b, 1);
  ck_assert (bson_element_type_get (e) == BSON_TYPE_DOUBLE);

  bson_unref (b);
}
END_TEST

START_TEST (test_bson_elements_key_find)
{
  bson_t *b;
  bson_element_t *hello, *answer, *pi, *e;
  uint32_t h_i, a_i, p_i;

  hello = bson_element_create ("hello", BSON_TYPE_STRING,
			       "world", BSON_LENGTH_AUTO);
  answer = bson_element_create ("answer", BSON_TYPE_INT32, 42);
  pi = bson_element_create ("pi", BSON_TYPE_DOUBLE, 3.14);

  b = bson_new_build (hello, answer, pi, BSON_END);

  ck_assert (bson_elements_key_find (NULL, "hello") == 0);
  ck_assert (bson_elements_key_find (b, "does-not-exist") == 0);
  ck_assert (bson_elements_key_find (b, "hell") == 0);
  ck_assert (bson_elements_key_find (b, "olleh") == 0);
  ck_assert (bson_elements_key_find (b, NULL) == 0);

  h_i = bson_elements_key_find (b, "hello");
  a_i = bson_elements_key_find (b, "answer");
  p_i = bson_elements_key_find (b, "pi");
  ck_assert_int_ne (h_i, 0);
  ck_assert_int_ne (a_i, 0);
  ck_assert_int_ne (p_i, 0);

  ck_assert_int_ne (h_i, a_i);
  ck_assert_int_ne (a_i, p_i);

  e = bson_elements_nth_get (b, h_i);
  ck_assert (e == hello);
  e = bson_elements_nth_get (b, a_i);
  ck_assert (e == answer);
  e = bson_elements_nth_get (b, p_i);
  ck_assert (e == pi);

  bson_unref (b);
}
END_TEST

START_TEST (test_bson_elements_key_get)
{
  bson_t *b;
  bson_element_t *hello, *answer, *pi;

  hello = bson_element_create ("hello", BSON_TYPE_STRING,
			       "world", BSON_LENGTH_AUTO);
  answer = bson_element_create ("answer", BSON_TYPE_INT32, 42);
  pi = bson_element_create ("pi", BSON_TYPE_DOUBLE, 3.14);

  b = bson_new_build (hello, answer, pi, BSON_END);

  ck_assert (bson_elements_key_get (NULL, "hello") == NULL);
  ck_assert (bson_elements_key_get (b, "does-not-exist") == NULL);
  ck_assert (bson_elements_key_get (b, "hell") == NULL);
  ck_assert (bson_elements_key_get (b, "olleh") == NULL);
  ck_assert (bson_elements_key_get (b, NULL) == NULL);

  ck_assert (bson_elements_key_get (b, "hello") == hello);
  ck_assert (bson_elements_key_get (b, "answer") == answer);
  ck_assert (bson_elements_key_get (b, "pi") == pi);

  bson_unref (b);
}
END_TEST

START_TEST (test_bson_stream_set)
{
  bson_t *old, *new;
  bson_element_t *e;

  old = bson_new_build
    (bson_element_create ("hello", BSON_TYPE_STRING,
			  "world", BSON_LENGTH_AUTO),
     bson_element_create ("answer", BSON_TYPE_INT32, 42),
     bson_element_create ("pi", BSON_TYPE_DOUBLE, 3.14),
     BSON_END);
  old = bson_stream_close (old);

  ck_assert (bson_stream_set (NULL, bson_stream_get_data (old)) == NULL);

  new = bson_new_sized (bson_stream_get_size (old));
  bson_elements_add
    (new, bson_element_create ("i", BSON_TYPE_INT32, 1), BSON_END);
  ck_assert (bson_stream_set (new, NULL) != NULL);
  ck_assert_int_eq (bson_elements_length (new), 0);

  new = bson_stream_set (new, bson_stream_get_data (old));
  new = bson_stream_close (new);

  ck_assert_int_eq (bson_stream_get_size (old),
		    bson_stream_get_size (new));

  bson_unref (new);

  /* Add an invalid element... */
  old = bson_stream_open (old);
  e = bson_element_new ();
  e = bson_element_type_set (e, 42);
  e = bson_element_name_set (e, "invalid");
  e = bson_element_data_set (e, (uint8_t *)"foobar", strlen ("foobar"));

  old = bson_elements_add (old, e,
			   bson_element_create ("valid", BSON_TYPE_INT32, 1),
			   BSON_END);
  old = bson_stream_close (old);

  new = bson_new_sized (bson_stream_get_size (old));
  new = bson_stream_set (new, bson_stream_get_data (old));

  _ck_assert_int (bson_elements_length (old), >,
		  bson_elements_length (new));

  bson_unref (old);
  bson_unref (new);
}
END_TEST

START_TEST (test_bson_stream_merge)
{
  bson_t *b1, *b2;

  b1 = bson_new_build
    (bson_element_create ("hello", BSON_TYPE_STRING,
			  "world", BSON_LENGTH_AUTO),
     BSON_END);

  b2 = bson_new_build
    (bson_element_create ("answer", BSON_TYPE_INT32, 42),
     BSON_END);
  b2 = bson_stream_close (b2);

  b1 = bson_stream_merge (b1, bson_stream_get_data (b2));
  ck_assert_int_eq (bson_elements_length (b1), 2);

  b1 = bson_stream_merge (b1, NULL);
  ck_assert_int_eq (bson_elements_length (b1), 2);

  ck_assert (bson_stream_merge (NULL, bson_stream_get_data (b2)) == NULL);

  b1 = bson_stream_close (b1);
  b1 = bson_stream_merge (b1, bson_stream_get_data (b2));
  ck_assert_int_eq (bson_elements_length (b1), 2);

  bson_unref (b1);
  bson_unref (b2);
}
END_TEST

START_TEST (test_bson_new_from_data)
{
  bson_t *old, *new;
  bson_element_t *e;

  old = bson_new_build
    (bson_element_create ("hello", BSON_TYPE_STRING,
			  "world", BSON_LENGTH_AUTO),
     bson_element_create ("answer", BSON_TYPE_INT32, 42),
     bson_element_create ("pi", BSON_TYPE_DOUBLE, 3.14),
     BSON_END);
  old = bson_stream_close (old);

  new = bson_new_from_data (bson_stream_get_data (old));
  new = bson_stream_close (new);

  ck_assert_int_eq (bson_stream_get_size (old),
		    bson_stream_get_size (new));

  bson_unref (new);

  /* Add an invalid element... */
  old = bson_stream_open (old);
  e = bson_element_new ();
  e = bson_element_type_set (e, 42);
  e = bson_element_name_set (e, "invalid");
  e = bson_element_data_set (e, (uint8_t *)"foobar", strlen ("foobar"));

  old = bson_elements_add (old, e,
			   bson_element_create ("valid", BSON_TYPE_INT32, 1),
			   BSON_END);
  old = bson_stream_close (old);

  new = bson_new_from_data (bson_stream_get_data (old));

  _ck_assert_int (bson_elements_length (old), >,
		  bson_elements_length (new));

  bson_unref (old);
  bson_unref (new);
}
END_TEST

START_TEST (test_bson_elements_merge)
{
  bson_t *b1, *b2;

  b1 = bson_new_build
    (bson_element_create ("hello", BSON_TYPE_STRING,
			  "world", BSON_LENGTH_AUTO),
     BSON_END);

  b2 = bson_new_build
    (bson_element_create ("answer", BSON_TYPE_INT32, 42),
     BSON_END);

  ck_assert (bson_elements_merge (NULL, b2) == NULL);

  b1 = bson_elements_merge (b1, NULL);
  ck_assert_int_eq (bson_elements_length (b1), 1);

  b1 = bson_elements_merge (b1, bson_ref (b2));
  ck_assert_int_eq (bson_elements_length (b1), 2);

  b2 = bson_stream_close (b2);
  b1 = bson_elements_merge (b1, bson_ref (b2));
  ck_assert_int_eq (bson_elements_length (b1), 3);

  b1 = bson_stream_close (b1);
  b1 = bson_elements_merge (b1, b2);
  ck_assert_int_eq (bson_elements_length (b1), 3);

  bson_unref (b1);
}
END_TEST

START_TEST (test_bson_elements_nth_remove)
{
  bson_t *b;
  bson_element_type_t type;

  ck_assert (bson_elements_nth_remove (NULL, 1) == NULL);

  b = bson_new_build
    (bson_element_create ("hello", BSON_TYPE_STRING,
			  "world", BSON_LENGTH_AUTO),
     bson_element_create ("answer", BSON_TYPE_INT32, 42),
     bson_element_create ("pi", BSON_TYPE_DOUBLE, 3.14),
     BSON_END);

  b = bson_elements_nth_remove (b, 0);
  ck_assert_int_eq (bson_elements_length (b), 3);

  b = bson_elements_nth_remove (b, 10);
  ck_assert_int_eq (bson_elements_length (b), 3);

  type = bson_element_type_get (bson_elements_nth_get (b, 1));
  b = bson_elements_nth_remove (b, 1);
  ck_assert_int_eq (bson_elements_length (b), 2);
  ck_assert (bson_element_type_get (bson_elements_nth_get (b, 1)) != type);

  b = bson_elements_nth_remove (b, 2);
  ck_assert_int_eq (bson_elements_length (b), 1);
  ck_assert (bson_element_type_get (bson_elements_nth_get (b, 1)) != type);

  bson_unref (b);
}
END_TEST

START_TEST (test_bson_elements_key_remove)
{
  bson_t *b;

  ck_assert (bson_elements_key_remove (NULL, "hello") == NULL);

  b = bson_new_build
    (bson_element_create ("hello", BSON_TYPE_STRING,
			  "world", BSON_LENGTH_AUTO),
     bson_element_create ("answer", BSON_TYPE_INT32, 42),
     BSON_END);

  b = bson_elements_key_remove (b, "invalid-key");
  ck_assert_int_eq (bson_elements_length (b), 2);

  b = bson_elements_key_remove (b, "hell");
  ck_assert_int_eq (bson_elements_length (b), 2);

  b = bson_elements_key_remove (b, "olleh");
  ck_assert_int_eq (bson_elements_length (b), 2);

  b = bson_elements_key_remove (b, "hello");
  ck_assert_int_eq (bson_elements_length (b), 1);
  ck_assert (bson_element_type_get
	     (bson_elements_key_get (b, "answer")) == BSON_TYPE_INT32);

  bson_unref (b);
}
END_TEST

Suite *
bson_suite (void)
{
  Suite *s;

  TCase *tc_core, *tc_ele, *tc_stream;

  s = suite_create ("BSON unit tests");

  tc_core = tcase_create ("Core");
  tcase_add_test (tc_core, test_bson_new);
  tcase_add_test (tc_core, test_bson_new_sized);
  tcase_add_test (tc_core, test_bson_new_build);
  tcase_add_test (tc_core, test_bson_new_from_data);
  tcase_add_test (tc_core, test_bson_ref);
  suite_add_tcase (s, tc_core);

  tc_ele = tcase_create ("Elements");
  tcase_add_test (tc_ele, test_bson_elements_length);
  tcase_add_test (tc_ele, test_bson_elements_add);
  tcase_add_test (tc_ele, test_bson_elements_reset);
  tcase_add_test (tc_ele, test_bson_elements_nth_get_set);
  tcase_add_test (tc_ele, test_bson_elements_nth_remove);
  tcase_add_test (tc_ele, test_bson_elements_key_find);
  tcase_add_test (tc_ele, test_bson_elements_key_get);
  tcase_add_test (tc_ele, test_bson_elements_key_remove);
  tcase_add_test (tc_ele, test_bson_elements_merge);
  suite_add_tcase (s, tc_ele);

  tc_stream = tcase_create ("Stream");
  tcase_add_test (tc_stream, test_bson_stream_open_close);
  tcase_add_test (tc_stream, test_bson_stream);
  tcase_add_test (tc_stream, test_bson_stream_merge);
  tcase_add_test (tc_stream, test_bson_stream_set);
  suite_add_tcase (s, tc_stream);

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
