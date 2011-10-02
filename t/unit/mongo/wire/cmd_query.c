#include "test.h"

static void
test_mongo_wire_cmd_query (void)
{
  bson_t *q, *s, *tmp;
  mongo_wire_packet_t *p;

  mongo_wire_packet_header_t hdr;
  const uint8_t *data;
  int32_t data_size;

  bson_cursor_t *c;
  int32_t pos;

  q = test_bson_generate_full ();
  s = bson_new ();
  bson_append_boolean (s, "_id", TRUE);
  bson_append_boolean (s, "double", TRUE);
  bson_finish (s);

  tmp = bson_new ();

  ok (mongo_wire_cmd_query (1, NULL, 0, 0, 0, q, s) == NULL,
      "mongo_wire_cmd_query() fails whith a NULL namespace");
  ok (mongo_wire_cmd_query (1, "test.ns", 0, 0, 0, NULL, s) == NULL,
      "mongo_wire_cmd_query() fails with a NULL query");
  ok (mongo_wire_cmd_query (1, "test.ns", 0, 0, 0, tmp, s) == NULL,
      "mongo_wire_cmd_query() fails with an unfinished query");
  ok (mongo_wire_cmd_query (1, "test.ns", 0, 0, 0, q, tmp) == NULL,
      "mongo_wire_cmd_query() fails with an unfinished selector");
  bson_free (tmp);

  ok ((p = mongo_wire_cmd_query (1, "test.ns", 0, 0, 10, q, NULL)) != NULL,
      "mongo_wire_cmd_query() works with a NULL selector");

  mongo_wire_packet_get_header (p, &hdr);
  cmp_ok ((data_size = mongo_wire_packet_get_data (p, &data)), "!=", -1,
	  "Packet data size looks fine");
  cmp_ok (hdr.length, "==", sizeof (mongo_wire_packet_header_t) + data_size,
	  "Packet header length is OK");
  cmp_ok (hdr.id, "==", 1, "Packet request ID is ok");
  cmp_ok (hdr.resp_to, "==", 0, "Packet reply ID is ok");

  /* pos = zero + collection_name + NULL + skip + ret */
  pos = sizeof (int32_t) + strlen ("test.ns") + 1 + sizeof (int32_t) * 2;
  ok ((tmp = bson_new_from_data (data + pos,
				 bson_stream_doc_size (data, pos) - 1)) != NULL,
      "Packet contains a valid BSON query document");
  bson_finish (tmp);

  c = bson_cursor_find (bson_cursor_new (tmp), "int32");
  ok (lmc_error_isok (c),
      "BSON contains 'int32'");
  cmp_ok (bson_cursor_type (c), "==", BSON_TYPE_INT32,
	  "int32 has correct type");
  bson_cursor_next (c);
  cmp_ok (bson_cursor_type (c), "==", BSON_TYPE_INT64,
	  "next element has correct type too");
  ok (!lmc_error_isok (bson_cursor_next (c)),
      "No more data after the update BSON object");
  bson_cursor_free (c);

  cmp_ok (hdr.length, "==", sizeof (mongo_wire_packet_header_t) + pos +
	  bson_size (q),
	  "Packet header lenght is correct");
  bson_free (tmp);
  mongo_wire_packet_free (p);

  /*
   * Test again with a selector document
   */

  ok ((p = mongo_wire_cmd_query (1, "test.ns", 0, 0, 10, q, s)) != NULL,
      "mongo_wire_cmd_query() works with a NULL selector");

  mongo_wire_packet_get_header (p, &hdr);
  cmp_ok ((data_size = mongo_wire_packet_get_data (p, &data)), "!=", -1,
	  "Packet data size looks fine");
  cmp_ok (hdr.length, "==", sizeof (mongo_wire_packet_header_t) + data_size,
	  "Packet header length is OK");
  cmp_ok (hdr.id, "==", 1, "Packet request ID is ok");
  cmp_ok (hdr.resp_to, "==", 0, "Packet reply ID is ok");

  /* pos = zero + collection_name + NULL + skip + ret */
  pos = sizeof (int32_t) + strlen ("test.ns") + 1 + sizeof (int32_t) * 2;
  ok ((tmp = bson_new_from_data (data + pos,
				 bson_stream_doc_size (data, pos) - 1)) != NULL,
      "Packet contains a valid BSON query document");
  bson_finish (tmp);
  pos += bson_size (tmp);
  bson_free (tmp);
  bson_free (q);
  bson_free (s);

  ok ((s = bson_new_from_data (data + pos,
			       bson_stream_doc_size (data, pos) - 1)) != NULL,
      "Packet contains a valid BSON selector document");
  bson_finish (s);

  c = bson_cursor_find (bson_cursor_new (s), "_id");
  ok (lmc_error_isok (c),
      "BSON contains '_id'");
  cmp_ok (bson_cursor_type (c), "==", BSON_TYPE_BOOLEAN,
	  "_id has correct type");
  bson_cursor_next (c);
  cmp_ok (bson_cursor_type (c), "==", BSON_TYPE_BOOLEAN,
	  "next element has correct type too");

  bson_cursor_free (c);
  bson_free (s);
  mongo_wire_packet_free (p);
}

RUN_TEST (25, mongo_wire_cmd_query);
