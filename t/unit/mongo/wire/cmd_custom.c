#include "test.h"

static void
test_mongo_wire_cmd_custom (void)
{
  bson_t *cmd;
  mongo_wire_packet_t *p;

  mongo_wire_packet_header_t hdr;
  const uint8_t *data;
  int32_t data_size;

  bson_cursor_t *c;
  int32_t pos;

  cmd = bson_new ();
  bson_append_int32 (cmd, "getnonce", 1);

  ok (mongo_wire_cmd_custom (1, "test", 0, NULL) == NULL,
      "mongo_wire_cmd_custom() fails with a NULL command");
  ok (mongo_wire_cmd_custom (1, "test", 0, cmd) == NULL,
      "mongo_wire_cmd_custom() fails with an unfinished command");
  bson_finish (cmd);
  ok (mongo_wire_cmd_custom (1, NULL, 0, cmd) == NULL,
      "mongo_wire_cmd_custom() fails with a NULL db");

  ok ((p = mongo_wire_cmd_custom (1, "test", 0, cmd)) != NULL,
      "mongo_wire_cmd_custom() works");
  bson_free (cmd);

  /* Verify the header */
  mongo_wire_packet_get_header (p, &hdr);
  cmp_ok ((data_size = mongo_wire_packet_get_data (p, &data)), "!=", -1,
	  "Packet data size looks fine");
  cmp_ok (hdr.length, "==", sizeof (mongo_wire_packet_header_t) + data_size,
	  "Packet header length is OK");
  cmp_ok (hdr.id, "==", 1, "Packet request ID is ok");
  cmp_ok (hdr.resp_to, "==", 0, "Packet reply ID is ok");

  /*
   * Test the created request
   */

  /* pos = zero + collection_name + NULL + skip + ret */
  pos = sizeof (int32_t) + strlen ("test.$cmd") + 1 + sizeof (int32_t) * 2;
  ok ((cmd = bson_new_from_data (data + pos,
				 bson_stream_doc_size (data, pos) - 1)) != NULL,
      "Packet contains a BSON document");
  bson_finish (cmd);

  c = bson_cursor_find (bson_cursor_new (cmd), "getnonce");
  ok (lmc_error_isok (c),
      "BSON object contains a 'getnonce' key");
  cmp_ok (bson_cursor_type (c), "==", BSON_TYPE_INT32,
	  "'getnonce' key has the correct type");
  ok (!lmc_error_isok (bson_cursor_next (c)),
      "'getnonce' key is the last in the object");

  bson_cursor_free (c);
  bson_free (cmd);
  mongo_wire_packet_free (p);
}

RUN_TEST (12, mongo_wire_cmd_custom);
