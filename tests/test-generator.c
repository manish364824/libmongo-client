#include "test.h"
#include "test-generator.h"

#include <glib.h>
#include <string.h>

bson *
test_bson_generate_flat (void)
{
  bson *b;

  b = bson_new ();
  g_assert (bson_append_int32 (b, "int32", 1984));
  g_assert (bson_append_boolean (b, "FALSE", FALSE));
  g_assert (bson_append_string
	    (b, "goodbye", "cruel world, this garbage is gone.",
	     strlen ("cruel world")));
  g_assert (bson_append_utc_datetime (b, "date", 1294860709000));
  g_assert (bson_append_double (b, "double", 3.14));
  g_assert (bson_append_int64 (b, "int64", 9876543210));
  g_assert (bson_append_null (b, "null"));
  g_assert (bson_append_boolean (b, "TRUE", TRUE));
  g_assert (bson_append_string (b, "hello", "world", -1));

  bson_finish (b);

  return b;
}

bson *
test_bson_generate_nested (void)
{
  bson *d, *user, *posts;
  bson *p1, *p2, *comments;

  user = bson_new ();
  g_assert (bson_append_string (user, "name", "V.A. Lucky", -1));
  g_assert (bson_append_int32 (user, "id", 12345));
  bson_finish (user);

  comments = bson_new ();
  g_assert (bson_append_string (comments, "0", "first!", -1));
  g_assert (bson_append_string (comments, "1", "2nd!", -1));
  g_assert (bson_append_string (comments, "2", "last!", -1));
  bson_finish (comments);

  p1 = bson_new ();
  g_assert (bson_append_string (p1, "title", "Post #1", -1));
  g_assert (bson_append_utc_datetime (p1, "date", 1294860709000));
  g_assert (bson_append_array (p1, "comments", comments));
  bson_finish (p1);
  bson_free (comments);

  p2 = bson_new ();
  g_assert (bson_append_string (p2, "title", "Post #2", -1));
  g_assert (bson_append_utc_datetime (p2, "date", 1294860709000));
  bson_finish (p2);

  posts = bson_new ();
  g_assert (bson_append_document (posts, "0", p1));
  g_assert (bson_append_document (posts, "1", p2));
  bson_finish (posts);
  bson_free (p1);
  bson_free (p2);

  d = bson_new ();
  g_assert (bson_append_document (d, "user", user));
  g_assert (bson_append_array (d, "posts", posts));
  bson_finish (d);

  bson_free (user);
  bson_free (posts);

  return d;
}