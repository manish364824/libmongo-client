#include "test.h"

static void
test_mongo_wire_cmd_update (void)
{
  bson_t *sel, *upd, *tmp;
  mongo_wire_packet_t *p;

  mongo_wire_packet_header_t hdr;
  const uint8_t *data;
  int32_t data_size;

  bson_cursor_t *c;
  int32_t pos;

  sel = bson_new ();
  bson_append_null (sel, "_id");
  bson_finish (sel);

  upd = test_bson_generate_full ();

  ok (mongo_wire_cmd_update (1, NULL, 0, sel, upd) == NULL,
      "mongo_wire_cmd_update() with a NULL namespace should fail");
  ok (mongo_wire_cmd_update (1, "test.ns", 0, NULL, upd) == NULL,
      "mongo_wire_cmd_update() with a NULL selector should fail");
  ok (mongo_wire_cmd_update (1, "test.ns", 0, sel, NULL) == NULL,
      "mongo_wire_cmd_update() with a NULL update should fail");

  tmp = bson_new ();
  ok (mongo_wire_cmd_update (1, "test.ns", 0, tmp, upd) == NULL,
      "mongo_wire_cmd_update() fails with an unfinished selector");
  ok (mongo_wire_cmd_update (1, "test.ns", 0, sel, tmp) == NULL,
      "mongo_wire_cmd_update() fails with an unfinished update");
  bson_free (tmp);

  ok ((p = mongo_wire_cmd_update (1, "test.ns", 0, sel, upd)) != NULL,
      "mongo_wire_cmd_update() works");

  bson_free (sel);

  mongo_wire_packet_get_header (p, &hdr);
  cmp_ok ((data_size = mongo_wire_packet_get_data (p, &data)), "!=", -1,
	  "Packet data size looks fine");
  cmp_ok (hdr.length, "==", sizeof (mongo_wire_packet_header_t) + data_size,
	  "Packet header length is OK");
  cmp_ok (hdr.id, "==", 1, "Packet request ID is ok");
  cmp_ok (hdr.resp_to, "==", 0, "Packet reply ID is ok");

  /*
   * Verify the selector object.
   */

  /* pos = zero + collection_name + NULL + flags */
  pos = sizeof (int32_t) + strlen ("test.ns") + 1 + sizeof (int32_t);
  ok ((sel = bson_new_from_data (data + pos, (int32_t)data[pos] - 1)) != NULL,
      "Packet contains a valid BSON selector document");
  bson_finish (sel);

  c = bson_cursor_find (bson_cursor_new (sel), "_id");
  ok (lmc_error_isok (c),
      "BSON contains an _id");
  cmp_ok (bson_cursor_type (c), "==", BSON_TYPE_NULL,
	  "_id has correct type");
  bson_cursor_free (c);
  bson_free (sel);

  /*
   * Verify the update object
   */
  pos += (int32_t)data[pos];
  ok ((tmp = bson_new_from_data (data + pos,
				 bson_stream_doc_size (data, pos) - 1)) != NULL,
      "Packet contains a valid BSON update document");
  bson_finish (tmp);
  cmp_ok (bson_size (upd), "==", bson_size (tmp),
	  "Packet's update document has the correct size");

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
  bson_free (tmp);
  bson_free (upd);
  mongo_wire_packet_free (p);
}

RUN_TEST (19, mongo_wire_cmd_update);
