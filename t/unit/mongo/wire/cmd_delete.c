#include "test.h"

static void
test_mongo_wire_cmd_delete (void)
{
  mongo_wire_packet_t *p;
  bson_t *s, *tmp;

  mongo_wire_packet_header_t hdr;
  const uint8_t *data;
  int32_t data_size;

  int32_t pos;
  bson_cursor_t *c;

  s = test_bson_generate_full ();
  tmp = bson_new ();

  ok (mongo_wire_cmd_delete (1, NULL, 0, s) == NULL,
      "mongo_wire_cmd_delete() fails with a NULL namespace");
  ok (mongo_wire_cmd_delete (1, "test.ns", 0, NULL) == NULL,
      "mongo_wire_cmd_delete() fails with a NULL selector");
  ok (mongo_wire_cmd_delete (1, "test.ns", 0, tmp) == NULL,
      "mongo_wire_cmd_delete() fails with an unfinished selector");
  bson_free (tmp);

  ok ((p = mongo_wire_cmd_delete (1, "test.ns", 0, s)) != NULL,
      "mongo_wire_cmd_delete() works");
  bson_free (s);

  /* Test basic header data */
  mongo_wire_packet_get_header (p, &hdr);
  cmp_ok ((data_size = mongo_wire_packet_get_data (p, &data)), "!=", -1,
	  "Packet data size appears fine");

  cmp_ok (hdr.length, "==", sizeof (mongo_wire_packet_header_t) + data_size,
	  "Packet header length is correct");
  cmp_ok (hdr.id, "==", 1, "Header ID is ok");
  cmp_ok (hdr.resp_to, "==", 0, "Response ID is ok");

  /*
   * Test the constructed request
   */

  /* pos = zero + ns + NULL + flags */
  pos = sizeof (int32_t) + strlen ("test.ns") + 1 + sizeof (int32_t);

  ok ((s = bson_new_from_data (data + pos,
			       bson_stream_doc_size (data, pos) - 1)) != NULL,
      "Packet contains a valid BSON update document");
  bson_finish (s);

  c = bson_cursor_find (bson_cursor_new (s), "int32");
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
  bson_free (s);

  mongo_wire_packet_free (p);
}

RUN_TEST (13, mongo_wire_cmd_delete);
