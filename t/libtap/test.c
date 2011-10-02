#include "test.h"

#include "lmc/private.h"

#include <string.h>

func_config_t config;

bson_t *
test_bson_generate_full (void)
{
  bson_t *b, *d, *a, *scope;
  bson_oid_t oid;
  bson_timestamp_t t;

  t.i = 10;
  t.t = 12345678;

  memcpy (oid.bytes, "1234567890ab", 12);

  a = bson_new ();
  bson_append_int32 (a, "0", 32);
  bson_append_int64 (a, "1", (int64_t)-42);
  bson_finish (a);

  d = bson_new ();
  bson_append_string (d, "name", "sub-document", -1);
  bson_append_int32 (d, "answer", 42);
  bson_finish (d);

  scope = bson_new ();
  bson_append_string (scope, "v", "hello world", -1);
  bson_finish (scope);

  b = bson_new ();
  bson_append_double (b, "double", 3.14);
  bson_append_string (b, "str", "hello world", -1);

  bson_append_document (b, "doc", d);
  bson_append_array (b, "array", a);
  bson_append_binary (b, "binary0", BSON_BINARY_SUBTYPE_GENERIC,
		      (uint8_t *)"foo\0bar", 7);
  bson_append_oid (b, "_id", &oid);
  bson_append_boolean (b, "TRUE", FALSE);
  bson_append_utc_datetime (b, "date", 1294860709000);
  bson_append_timestamp (b, "ts", &t);
  bson_append_null (b, "null");
  bson_append_regex (b, "foobar", "s/foo.*bar/", "i");
  bson_append_javascript (b, "alert", "alert (\"hello world!\");", -1);
  bson_append_symbol (b, "sex", "Marilyn Monroe", -1);
  bson_append_javascript_w_scope (b, "print", "alert (v);", -1, scope);
  bson_append_int32 (b, "int32", 32);
  bson_append_int64 (b, "int64", (int64_t)-42);

  bson_finish (b);

  bson_free (d);
  bson_free (a);
  bson_free (scope);

  return b;
}

mongo_wire_packet_t *
test_mongo_wire_generate_reply (lmc_bool_t valid, int32_t nreturn,
				lmc_bool_t with_docs)
{
  mongo_wire_reply_packet_header_t rh;
  mongo_wire_packet_header_t h;
  mongo_wire_packet_t *p;
  uint8_t *data;
  int data_size = sizeof (mongo_wire_reply_packet_header_t);
  bson_t *b1 = NULL, *b2 = NULL;

  p = mongo_wire_packet_new ();

  h.opcode = (valid) ? LMC_INT32_TO_LE (1) : LMC_INT32_TO_LE (42);
  h.id = LMC_INT32_TO_LE (1984);
  h.resp_to = LMC_INT32_TO_LE (42);
  if (with_docs)
    {
      b1 = test_bson_generate_full ();
      b2 = test_bson_generate_full ();
      data_size += bson_size (b1) + bson_size (b2);
    }
  h.length = LMC_INT32_TO_LE (sizeof (mongo_wire_packet_header_t) + data_size);

  mongo_wire_packet_set_header (p, &h);

  data = lmc_alloc (uint8_t, data_size);

  rh.flags = 0;
  rh.cursor_id = LMC_INT64_TO_LE ((int64_t)12345);
  rh.start = 0;
  rh.returned = LMC_INT32_TO_LE (nreturn);

  memcpy (data, &rh, sizeof (mongo_wire_reply_packet_header_t));
  if (with_docs)
    {
      memcpy (data + sizeof (mongo_wire_reply_packet_header_t),
	      bson_data (b1), bson_size (b1));
      memcpy (data + sizeof (mongo_wire_reply_packet_header_t) +
	      bson_size (b1),
	      bson_data (b2), bson_size (b2));
    }

  mongo_wire_packet_set_data (p, data, data_size);
  free (data);
  bson_free (b1);
  bson_free (b2);

  return p;
}

lmc_bool_t
test_env_setup (void)
{
#if 0
  config.primary_host = config.secondary_host = NULL;
  config.primary_port = config.secondary_port = 27017;
  config.db = g_strdup ("test");
  config.coll = g_strdup ("libmongo");

  if (getenv ("TEST_DB"))
    {
      g_free (config.db);
      config.db = g_strdup (getenv ("TEST_DB"));
    }
  if (getenv ("TEST_COLLECTION"))
    {
      g_free (config.coll);
      config.coll = g_strdup (getenv ("TEST_COLLECTION"));
    }
  config.ns = g_strconcat (config.db, ".", config.coll, NULL);

  config.gfs_prefix = g_strconcat (config.ns, ".", "grid", NULL);

  if (!getenv ("TEST_PRIMARY") || strlen (getenv ("TEST_PRIMARY")) == 0)
    return FALSE;

  if (!mongo_util_parse_addr (getenv ("TEST_PRIMARY"), &config.primary_host,
			      &config.primary_port))
    return FALSE;

  if (getenv ("TEST_SECONDARY") && strlen (getenv ("TEST_SECONDARY")) > 0)
    mongo_util_parse_addr (getenv ("TEST_SECONDARY"), &config.secondary_host,
			   &config.secondary_port);
#endif
  return TRUE;
}

void
test_env_free (void)
{
#if 0
  g_free (config.primary_host);
  g_free (config.secondary_host);
  g_free (config.db);
  g_free (config.coll);
  g_free (config.ns);
  g_free (config.gfs_prefix);
#endif
}
