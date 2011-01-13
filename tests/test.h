#ifndef LIBMONGO_CLIENT_TEST_H
#define LIBMONGO_CLIENT_TEST_H 1

#include "bson.h"

static gchar *current_test = NULL;

#define TEST(s) current_test = #s
#define PASS()					\
  {						\
    printf ("PASS: %s\n", current_test);	\
    current_test = NULL;			\
  }
#define FAIL() \
  {	       \
    printf ("FAIL: %s\n", current_test);	\
    current_test = NULL;			\
    abort ();					\
  }

gboolean test_bson_dump (bson *b);
gboolean dump_bson (bson *b);

#endif
