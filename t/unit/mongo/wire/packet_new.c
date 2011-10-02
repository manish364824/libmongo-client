#include "test.h"

static void
test_mongo_wire_packet_new (void)
{
  mongo_wire_packet_t *p;

  ok ((p = mongo_wire_packet_new ()) != NULL,
      "mongo_wire_packet_new() works");
  mongo_wire_packet_free (NULL);
  pass ("mongo_wire_packet_free(NULL) works");
  mongo_wire_packet_free (p);
  pass ("mongo_wire_packet_free() works");
}

RUN_TEST (3, mongo_wire_packet_new);
