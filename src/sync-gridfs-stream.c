/* sync-gridfs-stream.c - libmongo-client GridFS streaming implementation
 * Copyright 2011 Gergely Nagy <algernon@balabit.hu>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/** @file src/sync-gridfs-stream.c
 * MongoDB GridFS Streaming API implementation.
 */

#include "sync-gridfs-stream.h"
#include "libmongo-private.h"

#include <unistd.h>
#include <errno.h>

mongo_sync_gridfs_stream *
mongo_sync_gridfs_stream_from_file (const mongo_sync_gridfs_file *gfile)
{
  mongo_sync_gridfs_stream *stream;
  bson_cursor *c;
  mongo_sync_gridfs_file *f;

  if (!gfile)
    {
      errno = ENOENT;
      return NULL;
    }

  stream = g_new0 (mongo_sync_gridfs_stream, 1);

  stream->super.gfs = gfile->gfs;
  stream->super.meta.metadata = bson_new_from_data
    (bson_data (gfile->meta.metadata),
     bson_size (gfile->meta.metadata) - 1);
  bson_finish (stream->super.meta.metadata);

  f = (mongo_sync_gridfs_file *)stream;

  /* Copy metadata from the new file object */
  c = bson_find (f->meta.metadata, "_id");
  if (!bson_cursor_get_oid (c, &f->meta.oid))
    {
      mongo_sync_gridfs_file_free (f);
      bson_cursor_free (c);
      errno = EPROTO;
      return NULL;
    }

  bson_cursor_find (c, "length");
  bson_cursor_get_int64 (c, &f->meta.length);

  if (f->meta.length == 0)
    {
      gint32 i = 0;

      bson_cursor_get_int32 (c, &i);
      f->meta.length = i;
    }

  bson_cursor_find (c, "chunkSize");
  bson_cursor_get_int32 (c, &f->meta.chunk_size);

  if (f->meta.length == 0 || f->meta.chunk_size == 0)
    {
      bson_cursor_free (c);
      mongo_sync_gridfs_file_free (f);
      errno = EPROTO;
      return NULL;
    }

  bson_cursor_find (c, "uploadDate");
  if (!bson_cursor_get_utc_datetime (c, &f->meta.date))
    {
      mongo_sync_gridfs_file_free (f);
      bson_cursor_free (c);
      errno = EPROTO;
      return NULL;
    }

  bson_cursor_find (c, "md5");
  if (!bson_cursor_get_string (c, &f->meta.md5))
    {
      mongo_sync_gridfs_file_free (f);
      bson_cursor_free (c);
      errno = EPROTO;
      return NULL;
    }
  bson_cursor_free (c);

  stream->write_stream = FALSE;

  stream->state.buffer = NULL;
  stream->state.buffer_offset = 0;
  stream->state.file_offset = 0;
  stream->state.current_chunk = 0;

  return stream;
}

mongo_sync_gridfs_stream *
mongo_sync_gridfs_stream_find (mongo_sync_gridfs *gfs,
			       const bson *query)
{
  mongo_sync_gridfs_file *gf;
  mongo_sync_gridfs_stream *stream;

  gf = mongo_sync_gridfs_find (gfs, query);
  if (!gf)
    return NULL;

  stream = mongo_sync_gridfs_stream_from_file (gf);
  mongo_sync_gridfs_file_free (gf);
  return stream;
}

mongo_sync_gridfs_stream *
mongo_sync_gridfs_stream_new (mongo_sync_gridfs *gfs,
			      const bson *metadata)
{
  mongo_sync_gridfs_stream *stream;
  bson_cursor *c;

  if (!gfs)
    {
      errno = ENOTCONN;
      return FALSE;
    }

  stream = g_new0 (mongo_sync_gridfs_stream, 1);
  stream->write_stream = TRUE;

  /* Set up stream->super here */
  stream->super.gfs = gfs;
  stream->super.meta.metadata = bson_new_from_data (bson_data (metadata),
						    bson_size (metadata) - 1);
  stream->super.meta.chunk_size = gfs->chunk_size;
  stream->super.meta.length = 0;
  stream->super.meta.date = 0;
  stream->super.meta.md5 = NULL;
  c = bson_find (metadata, "_id");
  if (!c)
    {
      guint8 *oid;

      oid = mongo_util_oid_new
	(mongo_connection_get_requestid ((mongo_connection *)gfs->conn));
      if (!oid)
	{
	  bson_free (stream->super.meta.metadata);
	  g_free (stream);

	  errno = EFAULT;
	  return NULL;
	}

      bson_append_oid (stream->super.meta.metadata, "_id", oid);
      g_free (oid);
    }
  bson_cursor_free (c);
  bson_finish (stream->super.meta.metadata);

  c = bson_find (stream->super.meta.metadata, "_id");
  bson_cursor_get_oid (c, &stream->super.meta.oid);
  bson_cursor_free (c);

  stream->state.buffer = g_malloc (stream->super.meta.chunk_size);
  stream->state.buffer_offset = 0;
  stream->state.file_offset = 0;
  stream->state.current_chunk = 0;

  stream->checksum = g_checksum_new (G_CHECKSUM_MD5);

  return stream;
}

static inline gboolean
_stream_seek_chunk (mongo_sync_gridfs_stream *stream,
		    gint64 chunk)
{
  bson *b;
  mongo_packet *p;
  bson_cursor *c;
  bson_binary_subtype subt;

  b = bson_new_sized (32);
  bson_append_oid (b, "files_id", stream->super.meta.oid);
  bson_append_int64 (b, "n", chunk);
  bson_finish (b);

  p = mongo_sync_cmd_query (stream->super.gfs->conn,
			    stream->super.gfs->ns.chunks, 0,
			    0, 1, b, NULL);
  if (!p)
    {
      int e = errno;

      bson_free (b);
      errno = e;
      return FALSE;
    }
  bson_free (b);

  bson_free (stream->chunk.bson);
  stream->chunk.bson = NULL;
  stream->chunk.data = NULL;

  if (!mongo_wire_reply_packet_get_nth_document (p, 1, &stream->chunk.bson))
    {
      int e = errno;

      mongo_wire_packet_free (p);
      errno = e;
      return FALSE;
    }
  mongo_wire_packet_free (p);
  bson_finish (stream->chunk.bson);

  c = bson_find (stream->chunk.bson, "data");
  if (!bson_cursor_get_binary (c, &subt, &stream->chunk.data,
			       &stream->chunk.size))
    {
      bson_cursor_free (c);
      bson_free (stream->chunk.bson);
      stream->chunk.bson = NULL;
      stream->chunk.data = NULL;

      errno = EPROTO;
      return FALSE;
    }
  bson_cursor_free (c);

  stream->chunk.offset = 0;
  return TRUE;
}

gint64
mongo_sync_gridfs_stream_read (mongo_sync_gridfs_stream *stream,
			       guint8 *buffer,
			       gint64 size)
{
  gint64 pos = 0;

  if (!stream)
    {
      errno = ENOENT;
      return -1;
    }
  if (!buffer || size <= 0)
    {
      errno = EINVAL;
      return -1;
    }

  if (!stream->chunk.data)
    {
      if (!_stream_seek_chunk (stream, 0))
	return -1;
    }

  while (pos < size && stream->state.file_offset < stream->super.meta.length)
    {
      gint32 csize = stream->chunk.size - stream->chunk.offset;

      if (size - pos < csize)
	csize = size - pos;

      memcpy (buffer + pos, stream->chunk.data + stream->chunk.offset,
	      csize);

      stream->chunk.offset += csize;
      stream->state.file_offset += csize;
      pos += csize;

      if (stream->chunk.offset >= stream->chunk.size &&
	  stream->state.file_offset < stream->super.meta.length)
	{
	  stream->state.current_chunk++;
	  if (!_stream_seek_chunk (stream, stream->state.current_chunk))
	    return -1;
	}
    }

  return pos;
}

static gboolean
_stream_chunk_write (mongo_sync_gridfs *gfs,
		     const guint8 *oid, gint64 n,
		     const guint8 *buffer, gint32 size)
{
  bson *chunk;

  chunk = bson_new_sized (size + 128);
  bson_append_oid (chunk, "files_id", oid);
  bson_append_int64 (chunk, "n", n);
  bson_append_binary (chunk, "data", BSON_BINARY_SUBTYPE_GENERIC,
		      buffer, size);
  bson_finish (chunk);

  if (!mongo_sync_cmd_insert (gfs->conn, gfs->ns.chunks, chunk, NULL))
    {
      int e = errno;

      bson_free (chunk);
      errno = e;
      return FALSE;
    }
  bson_free (chunk);

  return TRUE;
}

gboolean
mongo_sync_gridfs_stream_write (mongo_sync_gridfs_stream *stream,
				const guint8 *buffer,
				gint64 size)
{
  gint64 pos = 0;

  if (!stream)
    {
      errno = ENOENT;
      return FALSE;
    }
  if (!buffer || size <= 0)
    {
      errno = EINVAL;
      return FALSE;
    }

  while (pos < size)
    {
      gint32 csize = stream->super.meta.chunk_size -
	stream->state.buffer_offset;

      if (size - pos < csize)
	csize = size - pos;

      memcpy (stream->state.buffer + stream->state.buffer_offset,
	      buffer + pos, csize);
      stream->state.buffer_offset += csize;
      stream->state.file_offset += csize;
      stream->super.meta.length += csize;
      pos += csize;

      if (stream->state.buffer_offset == stream->super.meta.chunk_size)
	{
	  _stream_chunk_write (stream->super.gfs,
			       stream->super.meta.oid,
			       stream->state.current_chunk,
			       stream->state.buffer,
			       stream->super.meta.chunk_size);
	  g_checksum_update (stream->checksum, stream->state.buffer,
			     stream->super.meta.chunk_size);

	  stream->state.buffer_offset = 0;
	  stream->state.current_chunk++;
	}
    }

  return TRUE;
}

gboolean
mongo_sync_gridfs_stream_seek (mongo_sync_gridfs_stream *stream,
			       gint64 pos,
			       gint whence)
{
  gint64 real_pos = 0;
  gint64 chunk;
  gint32 offs;

  if (!stream)
    {
      errno = ENOENT;
      return FALSE;
    }
  if (stream->write_stream)
    {
      errno = EOPNOTSUPP;
      return FALSE;
    }

  switch (whence)
    {
    case SEEK_SET:
      if (pos == stream->state.file_offset)
	return TRUE;
      if (pos < 0 || pos > stream->super.meta.length)
	{
	  errno = ERANGE;
	  return FALSE;
	}
      real_pos = pos;
      break;
    case SEEK_CUR:
      if (pos + stream->state.file_offset < 0 ||
	  pos + stream->state.file_offset > stream->super.meta.length)
	{
	  errno = ERANGE;
	  return FALSE;
	}
      if (pos == 0)
	return TRUE;
      real_pos = pos + stream->state.file_offset;
      break;
    case SEEK_END:
      if (pos > 0 ||
	  pos + stream->super.meta.length < 0)
	{
	  errno = ERANGE;
	  return FALSE;
	}
      real_pos = pos + stream->super.meta.length;
      break;
    default:
      errno = EINVAL;
      return FALSE;
    }

  chunk = real_pos / stream->super.meta.chunk_size;
  offs = real_pos % stream->super.meta.chunk_size;

  if (!_stream_seek_chunk (stream, chunk))
    return FALSE;

  stream->chunk.offset = offs;
  stream->state.current_chunk = chunk;
  stream->state.file_offset = real_pos;

  return TRUE;
}

gboolean
mongo_sync_gridfs_stream_close (mongo_sync_gridfs_stream *stream)
{
  if (!stream)
    {
      errno = ENOENT;
      return FALSE;
    }

  if (stream->write_stream)
    {
      bson *meta;
      gint64 upload_date;
      GTimeVal tv;

      if (stream->state.buffer_offset > 0)
	{
	  _stream_chunk_write (stream->super.gfs,
			       stream->super.meta.oid,
			       stream->state.current_chunk,
			       stream->state.buffer,
			       stream->state.buffer_offset);
	  g_checksum_update (stream->checksum, stream->state.buffer,
			     stream->state.buffer_offset);
	}

      g_get_current_time (&tv);
      upload_date =  (((gint64) tv.tv_sec) * 1000) +
	(gint64)(tv.tv_usec / 1000);

      if (stream->super.meta.metadata)
	meta = bson_new_from_data (bson_data (stream->super.meta.metadata),
				   bson_size (stream->super.meta.metadata) - 1);
      else
	meta = bson_new_sized (128);
      bson_append_int64 (meta, "length", stream->super.meta.length);
      bson_append_int32 (meta, "chunkSize", stream->super.meta.chunk_size);
      bson_append_utc_datetime (meta, "uploadDate", upload_date);
      if (stream->super.meta.length)
	bson_append_string (meta, "md5",
			    g_checksum_get_string (stream->checksum), -1);
      bson_finish (meta);

      if (!mongo_sync_cmd_insert (stream->super.gfs->conn,
				  stream->super.gfs->ns.files, meta, NULL))
	{
	  int e = errno;

	  bson_free (meta);
	  errno = e;
	  return FALSE;
	}
      bson_free (meta);
    }

  g_checksum_free (stream->checksum);
  g_free (stream->state.buffer);
  bson_free (stream->chunk.bson);
  mongo_sync_gridfs_file_free ((mongo_sync_gridfs_file *)stream);
  return TRUE;
}
