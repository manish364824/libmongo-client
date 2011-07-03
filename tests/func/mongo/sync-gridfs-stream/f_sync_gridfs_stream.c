#include "test.h"
#include "mongo.h"

#define FILE_SIZE 1024 * 1024 + 12345
#define BUFFER_SIZE 64 * 1024

gchar *write_md5 = NULL;

void
test_func_sync_gridfs_stream_write (void)
{
  mongo_sync_connection *conn;
  mongo_sync_gridfs *gfs;
  mongo_sync_gridfs_stream *stream;
  bson *meta;
  guint8 *data, *oid;
  gint pos = 0;
  gint filler = 0;
  gboolean write_ok = TRUE;
  GChecksum *chk;

  conn = mongo_sync_connect (config.primary_host, config.primary_port, FALSE);
  gfs = mongo_sync_gridfs_new (conn, config.gfs_prefix);

  stream = mongo_sync_gridfs_stream_new (gfs, NULL);
  ok (stream == NULL,
      "mongo_sync_gridfs_stream_new() fails without mongo_util_oid_init()");

  mongo_util_oid_init (0);

  oid = mongo_util_oid_new (1);
  meta = bson_build (BSON_TYPE_STRING, "filename", "libmongo-test-stream", -1,
		     BSON_TYPE_OID, "_id", oid,
		     BSON_TYPE_NONE);
  bson_finish (meta);
  g_free (oid);

  stream = mongo_sync_gridfs_stream_new (gfs, meta);
  ok (stream != NULL,
      "mongo_sync_gridfs_stream_new() works");
  bson_free (meta);

  data = g_malloc (BUFFER_SIZE);

  chk = g_checksum_new (G_CHECKSUM_MD5);

  while (pos < FILE_SIZE)
    {
      gint csize = BUFFER_SIZE;

      if (csize + pos > FILE_SIZE)
	csize = FILE_SIZE - pos;

      memset (data, filler++, BUFFER_SIZE);

      g_checksum_update (chk, data, csize);

      write_ok &= mongo_sync_gridfs_stream_write (stream, data, csize);
      pos += csize;
    }
  ok (write_ok == TRUE,
      "All stream_write()s succeeded");

  write_md5 = g_strdup (g_checksum_get_string (chk));
  g_checksum_free (chk);

  note ("File MD5: %s\n", write_md5);

  g_free (data);
  ok (mongo_sync_gridfs_stream_close (stream) == TRUE,
      "mongo_sync_gridfs_stream_close() works");

  mongo_sync_gridfs_free (gfs, TRUE);
}

void
test_func_sync_gridfs_stream_write_invalid (void)
{
  mongo_sync_connection *conn;
  mongo_sync_gridfs *gfs;
  mongo_sync_gridfs_stream *stream;
  bson *meta;

  conn = mongo_sync_connect (config.primary_host, config.primary_port, FALSE);
  gfs = mongo_sync_gridfs_new (conn, config.gfs_prefix);

  meta = bson_build (BSON_TYPE_STRING, "filename", "lmc-invalid-id", -1,
		     BSON_TYPE_STRING, "_id", "Short and stout", -1,
		     BSON_TYPE_NONE);
  bson_finish (meta);

  stream = mongo_sync_gridfs_stream_new (gfs, meta);
  ok (stream == NULL,
      "mongo_sync_gridfs_stream_new() should fail if meta has an invalid _id");

  mongo_sync_gridfs_free (gfs, TRUE);
}

void
test_func_sync_gridfs_stream_read (void)
{
  mongo_sync_connection *conn;
  mongo_sync_gridfs *gfs;
  mongo_sync_gridfs_stream *stream;
  guint8 data[12345];
  gint64 pos = 0;
  bson *meta;

  GChecksum *chk;

  conn = mongo_sync_connect (config.primary_host, config.primary_port, FALSE);
  gfs = mongo_sync_gridfs_new (conn, config.gfs_prefix);
  meta = bson_build (BSON_TYPE_STRING, "filename", "libmongo-test-stream", -1,
		     BSON_TYPE_NONE);
  bson_finish (meta);

  stream = mongo_sync_gridfs_stream_find (gfs, meta);
  ok (stream != NULL,
      "mongo_sync_gridfs_stream_find() works");
  bson_free (meta);

  chk = g_checksum_new (G_CHECKSUM_MD5);

  while (pos < FILE_SIZE)
    {
      gint64 r;

      r = mongo_sync_gridfs_stream_read (stream, data, sizeof (data));
      if (r == -1)
	break;

      g_checksum_update (chk, data, r);
      pos += r;
    }

  cmp_ok (pos, "==", FILE_SIZE,
	  "mongo_sync_gridfs_stream_read() works");
  is (g_checksum_get_string (chk), write_md5,
      "md5sums match");

  g_checksum_free (chk);
  ok (mongo_sync_gridfs_stream_close (stream) == TRUE,
      "mongo_sync_gridfs_stream_close() works");
  mongo_sync_gridfs_free (gfs, TRUE);
}

void
test_func_sync_gridfs_stream_seek (void)
{
  mongo_sync_connection *conn;
  mongo_sync_gridfs *gfs;
  mongo_sync_gridfs_stream *stream;
  bson *meta;
  guint8 *chunk1, *chunk2, *chunk3;

  conn = mongo_sync_connect (config.primary_host, config.primary_port, FALSE);
  gfs = mongo_sync_gridfs_new (conn, config.gfs_prefix);
  meta = bson_build (BSON_TYPE_STRING, "filename", "libmongo-test-stream", -1,
		     BSON_TYPE_NONE);
  bson_finish (meta);

  stream = mongo_sync_gridfs_stream_find (gfs, meta);
  bson_free (meta);

  chunk1 = g_malloc (300 * 1024);
  chunk2 = g_malloc (300 * 1024);
  chunk3 = g_malloc (300 * 1024);

  cmp_ok (mongo_sync_gridfs_stream_read (stream, chunk1, 300 * 1024), "==",
	  300 * 1024,
	  "reading the first chunk works");
  cmp_ok (mongo_sync_gridfs_stream_read (stream, chunk2, 300 * 1024), "==",
	  300 * 1024,
	  "reading the second chunk works");
  ok (memcmp (chunk1, chunk2, 300 * 1024) != 0,
      "The two chunks differ, as they should");

  ok (mongo_sync_gridfs_stream_seek (stream, 0, SEEK_END) == TRUE,
      "mongo_sync_gridfs_stream_seek() works, with SEEK_END");
  cmp_ok (stream->file.offset, "==", stream->file.length,
	  "mongo_sync_gridfs_stream_seek() can seek to the end");

  ok (mongo_sync_gridfs_stream_seek (stream, 1, SEEK_SET) == TRUE,
      "mongo_sync_gridfs_stream_seek() works, with SEEK_SET");
  cmp_ok (stream->file.offset, "==", 1,
	  "mongo_sync_gridfs_stream_seek()'s SEEK_SET works");
  ok (mongo_sync_gridfs_stream_seek (stream, 1, SEEK_SET) == TRUE,
      "mongo_sync_gridfs_stream_seek() works, with SEEK_SET");

  ok (mongo_sync_gridfs_stream_seek (stream, -1, SEEK_CUR) == TRUE,
      "mongo_sync_gridfs_stream_seek() works, with SEEK_CUR");
  cmp_ok (stream->file.offset, "==", 0,
	  "mongo_sync_gridfs_stream_seek()'s SEEK_CUR works");
  ok (mongo_sync_gridfs_stream_seek (stream, 0, SEEK_CUR) == TRUE,
      "mongo_sync_gridfs_stream_seek() works, with SEEK_CUR");

  cmp_ok (mongo_sync_gridfs_stream_read (stream, chunk3, 300 * 1024), "==",
	  300 * 1024,
	  "reading after seeking works");

  ok (memcmp (chunk1, chunk3, 300 * 1024) == 0,
      "After seeking, we're at the beginning");

  mongo_sync_gridfs_stream_close (stream);
  g_free (chunk3);
  g_free (chunk2);
  g_free (chunk1);

  mongo_sync_gridfs_free (gfs, TRUE);
}

void
test_func_sync_gridfs_stream (void)
{
  test_func_sync_gridfs_stream_write ();
  test_func_sync_gridfs_stream_write_invalid ();
  test_func_sync_gridfs_stream_read ();
  test_func_sync_gridfs_stream_seek ();

  g_free (write_md5);
}

RUN_NET_TEST (22, func_sync_gridfs_stream);
