#include <check.h>
#include <stdlib.h>
#include <stdio.h>

#include <lmc/bson.h>

START_TEST (test_perf_flatten)
{
  bson_t *b;
  uint32_t n;
  char s[16];

  b = bson_new ();

  for (n = 0; n < 60; n++)
    {
      snprintf (s, 15, "i%d-%d", n, _i);

      bson_add_elements
	(b, bson_element_create (s, BSON_TYPE_INT32, n), BSON_END);
    }
  ck_assert_int_eq (bson_length (b), 60);

  b = bson_close (b);
  _ck_assert_int (bson_data_get_size (b), >, 64);

  b = bson_open (b);
  for (n = 0; n < 256; n++)
    {
      snprintf (s, 15, "i%d-%d", n + 64, _i);

      bson_add_elements
	(b, bson_element_create (s, BSON_TYPE_INT32, n), BSON_END);
    }
  ck_assert_int_eq (bson_length (b), 256 + 60);

  b = bson_close (b);

  bson_unref (b);
}
END_TEST

Suite *
bson_perf_suite (void)
{
  Suite *s;

  TCase *tc_perf;

  s = suite_create ("BSON performance tests");

  tc_perf = tcase_create ("Core");
  tcase_add_loop_test (tc_perf, test_perf_flatten, 0, 1000);
  suite_add_tcase (s, tc_perf);

  return s;
}

int
main (void)
{
  int nfailed;
  Suite *s;
  SRunner *sr;

  s = bson_perf_suite ();
  sr = srunner_create (s);

  srunner_run_all (sr, CK_ENV);
  nfailed = srunner_ntests_failed (sr);
  srunner_free (sr);

  return (nfailed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
