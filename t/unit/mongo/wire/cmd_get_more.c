#include "test.h"

static void
test_mongo_wire_cmd_get_more (void)
{
  mongo_wire_packet_t *p;

  mongo_wire_packet_header_t hdr;
  const uint8_t *data;
  int32_t data_size;

  int32_t pos;
  int64_t cid = 9876543210;

  ok (mongo_wire_cmd_get_more (1, NULL, 1, cid) == NULL,
      "mongo_wire_cmd_get_more() fails with a NULL namespace");
  ok ((p = mongo_wire_cmd_get_more (1, "test.ns", 1, cid)) != NULL,
      "mongo_wire_cmd_get_more() works");

  /* Test basic header data */
  mongo_wire_packet_get_header (p, &hdr);
  cmp_ok ((data_size = mongo_wire_packet_get_data (p, &data)), "!=", -1,
	  "Packet data size appears fine");

  cmp_ok (hdr.length, "==", sizeof (mongo_wire_packet_header_t) + data_size,
	  "Packet header length is correct");
  cmp_ok (hdr.id, "==", 1, "Header ID is ok");
  cmp_ok (hdr.resp_to, "==", 0, "Response ID is ok");

  /*
   * Test the request itself.
   */

  /* pos = zero + ns + NULL + ret */
  pos = sizeof (int32_t) + strlen ("test.ns") + 1 + sizeof (int32_t);
  cid = 0;
  memcpy (&cid, data + pos, sizeof (cid));
  cid = LMC_INT64_FROM_LE (cid);

  ok (cid == 9876543210,
      "Included CID is correct");

  mongo_wire_packet_free (p);
}

RUN_TEST (7, mongo_wire_cmd_get_more);
