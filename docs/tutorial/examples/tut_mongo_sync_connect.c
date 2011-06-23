#include <mongo.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

void
tut_connect (void)
{
  mongo_sync_connection *conn;

  conn = mongo_sync_connect ("localhost", 27017, TRUE);
  if (!conn)
    {
      perror ("mongo_sync_connect()");
      exit (1);
    }
  mongo_sync_disconnect (conn);
}

void
tut_connect_replica (void)
{
  mongo_sync_connection *conn;

  conn = mongo_sync_connect ("mongo-master", 27017, TRUE);
  if (!conn)
    {
      perror ("mongo_sync_connect()");
      exit (1);
    }

  if (!mongo_sync_conn_set_auto_reconnect (conn, TRUE))
    {
      perror ("mongo_sync_conn_set_auto_reconnect()");
      exit (1);
    }

  if (!mongo_sync_conn_seed_add (conn, "mongo-replica", 27017))
    {
      perror ("mongo_sync_conn_seed_add()");
      exit (1);
    }
  if (!mongo_sync_conn_seed_add (conn, "mongo-replica-2", 27017))
    {
      perror ("mongo_sync_conn_seed_add()");
      exit (1);
    }

  mongo_sync_disconnect (conn);
}

int
main (int argc, char *argv[])
{
  tut_connect ();
  tut_connect_replica ();

  return 0;
}
