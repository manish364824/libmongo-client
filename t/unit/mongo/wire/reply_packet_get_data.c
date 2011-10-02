#include "test.h"

static void
test_mongo_wire_reply_packet_get_data (void)
{
  mongo_wire_packet_t *p;
  mongo_wire_packet_header_t h;
  const uint8_t *data;
  bson_t *b;

  p = mongo_wire_packet_new ();
  memset (&h, 0, sizeof (mongo_wire_packet_header_t));
  h.opcode = 0;
  h.length = sizeof (mongo_wire_packet_header_t);
  mongo_wire_packet_set_header (p, &h);

  ok (mongo_wire_reply_packet_get_data (NULL, &data) == FALSE,
      "mongo_wire_reply_packet_get_data() fails with a NULL packet");
  ok (mongo_wire_reply_packet_get_data (p, NULL) == FALSE,
      "mongo_wire_reply_packet_get_data() fails with a NULL destination");
  ok (mongo_wire_reply_packet_get_data (p, &data) == FALSE,
      "mongo_wire_reply_packet_get_data() fails with a non-reply packet");

  h.opcode = 1;
  mongo_wire_packet_set_header (p, &h);

  ok (mongo_wire_reply_packet_get_data (p, &data) == FALSE,
      "mongo_wire_reply_packet_get_data() fails if the packet has "
      "no data");

  mongo_wire_packet_free (p);

  p = test_mongo_wire_generate_reply (TRUE, 2, TRUE);

  ok (mongo_wire_reply_packet_get_data (p, &data),
      "mongo_wire_reply_packet_get_data() works");

  b = test_bson_generate_full ();

  ok (memcmp (data, bson_data (b), bson_size (b)) == 0,
      "The returned data is correct");

  bson_free (b);
  mongo_wire_packet_free (p);
}

RUN_TEST (6, mongo_wire_reply_packet_get_data);
