/* mongo-wire.c - libmongo-client's MongoDB wire protocoll implementation.
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

/** @file src/mongo-wire.c
 * Implementation of the MongoDB Wire Protocol.
 */

#include <lmc.h>
#include "lmc/private.h"

#include <errno.h>
#include <stdarg.h>

/** @internal Constant zero value. */
static const int zero = 0;

/** @internal A MongoDB command, as it appears on the wire.
 *
 * For the sake of clarity, and sanity of the library, the header and
 * data parts are stored separately, and as such, will need to be sent
 * separately aswell.
 */
struct _mongo_wire_packet_t
{
  mongo_wire_packet_header_t header; /**< The packet header. */
  uint8_t *data; /**< The actual data of the packet. */
  int32_t data_size; /**< Size of the data payload. */
};

/** @internal Mongo command opcodes. */
typedef enum
{
  OP_REPLY = 1, /**< Message is a reply. Only sent by the server. */
  OP_MSG = 1000, /**< Message is a generic message. */
  OP_UPDATE = 2001, /**< Message is an update command. */
  OP_INSERT = 2002, /**< Message is an insert command. */
  OP_RESERVED = 2003, /**< Reserved and unused. */
  OP_QUERY = 2004, /**< Message is a query command. */
  OP_GET_MORE = 2005, /**< Message is a get more command. */
  OP_DELETE = 2006, /**< Message is a delete command. */
  OP_KILL_CURSORS = 2007 /**< Message is a kill cursors command. */
} mongo_wire_opcode;

mongo_wire_packet_t *
mongo_wire_packet_new (void)
{
  mongo_wire_packet_t *p = lmc_new0 (mongo_wire_packet_t);

  p->header.length = LMC_INT32_TO_LE (sizeof (mongo_wire_packet_header_t));
  return p;
}

lmc_bool_t
mongo_wire_packet_get_header (const mongo_wire_packet_t *p,
			      mongo_wire_packet_header_t *header)
{
  if (!p || !header)
    {
      errno = EINVAL;
      return FALSE;
    }

  header->length = LMC_INT32_FROM_LE (p->header.length);
  header->id = LMC_INT32_FROM_LE (p->header.id);
  header->resp_to = LMC_INT32_FROM_LE (p->header.resp_to);
  header->opcode = LMC_INT32_FROM_LE (p->header.opcode);

  return TRUE;
}

lmc_bool_t
mongo_wire_packet_get_header_raw (const mongo_wire_packet_t *p,
				  mongo_wire_packet_header_t *header)
{
  if (!p || !header)
    {
      errno = EINVAL;
      return FALSE;
    }

  header->length = p->header.length;
  header->id = p->header.id;
  header->resp_to = p->header.resp_to;
  header->opcode = p->header.opcode;

  return TRUE;
}

lmc_bool_t
mongo_wire_packet_set_header (mongo_wire_packet_t *p,
			      const mongo_wire_packet_header_t *header)
{
  if (!p || !header)
    {
      errno = EINVAL;
      return FALSE;
    }
  if (LMC_INT32_FROM_LE (header->length) <
      (int32_t)sizeof (mongo_wire_packet_header_t))
    {
      errno = ERANGE;
      return FALSE;
    }

  p->header.length = LMC_INT32_TO_LE (header->length);
  p->header.id = LMC_INT32_TO_LE (header->id);
  p->header.resp_to = LMC_INT32_TO_LE (header->resp_to);
  p->header.opcode = LMC_INT32_TO_LE (header->opcode);

  p->data_size = header->length - sizeof (mongo_wire_packet_header_t);

  return TRUE;
}

lmc_bool_t
mongo_wire_packet_set_header_raw (mongo_wire_packet_t *p,
				  const mongo_wire_packet_header_t *header)
{
  if (!p || !header)
    {
      errno = EINVAL;
      return FALSE;
    }

  p->header.length = header->length;
  p->header.id = header->id;
  p->header.resp_to = header->resp_to;
  p->header.opcode = header->opcode;

  p->data_size = header->length - sizeof (mongo_wire_packet_header_t);

  return TRUE;
}

int32_t
mongo_wire_packet_get_data (const mongo_wire_packet_t *p, const uint8_t **data)
{
  if (!p || !data)
    {
      errno = EINVAL;
      return -1;
    }
  if (p->data == NULL)
    {
      errno = EINVAL;
      return -1;
    }

  *data = (const uint8_t *)p->data;
  return p->data_size;
}

lmc_bool_t
mongo_wire_packet_set_data (mongo_wire_packet_t *p,
			    const uint8_t *data, int32_t size)
{
  if (!p || !data || size <= 0)
    {
      errno = EINVAL;
      return FALSE;
    }

  if (p->data)
    free (p->data);
  p->data = lmc_alloc (uint8_t, size);
  memcpy (p->data, data, size);

  p->data_size = size;
  p->header.length =
    LMC_INT32_TO_LE (p->data_size + sizeof (mongo_wire_packet_header_t));

  return TRUE;
}

void
mongo_wire_packet_free (mongo_wire_packet_t *p)
{
  if (!p)
    {
      errno = EINVAL;
      return;
    }

  if (p->data)
    free (p->data);
  free (p);
}

mongo_wire_packet_t *
mongo_wire_cmd_update (int32_t id, const char *ns, int32_t flags,
		       const bson_t *selector, const bson_t *update)
{
  mongo_wire_packet_t *p;
  int32_t t_flags = LMC_INT32_TO_LE (flags);
  int nslen;

  if (!ns || !selector || !update)
    {
      errno = EINVAL;
      return NULL;
    }

  if (bson_size (selector) < 0 ||
      bson_size (update) < 0)
    {
      errno = EINVAL;
      return NULL;
    }

  p = lmc_new0 (mongo_wire_packet_t);
  p->header.id = LMC_INT32_TO_LE (id);
  p->header.opcode = LMC_INT32_TO_LE (OP_UPDATE);

  nslen = strlen (ns) + 1;
  p->data_size = bson_size (selector) + bson_size (update) +
    sizeof (int32_t) * 2 + nslen;

  p->data = lmc_alloc (uint8_t, p->data_size);

  memcpy (p->data, (void *)&zero, sizeof (int32_t));
  memcpy (p->data + sizeof (int32_t), (void *)ns, nslen);
  memcpy (p->data + sizeof (int32_t) + nslen, (void *)&t_flags,
	  sizeof (int32_t));
  memcpy (p->data + sizeof (int32_t) * 2 + nslen,
	  bson_data (selector), bson_size (selector));
  memcpy (p->data + sizeof (int32_t) * 2 + nslen + bson_size (selector),
	  bson_data (update), bson_size (update));

  p->header.length = LMC_INT32_TO_LE (sizeof (p->header) + p->data_size);

  return p;
}

mongo_wire_packet_t *
mongo_wire_cmd_insert_n (int32_t id, const char *ns, int32_t n,
			 const bson_t **docs)
{
  mongo_wire_packet_t *p;
  int32_t i, pos, dsize = 0;

  if (!ns || !docs)
    {
      errno = EINVAL;
      return NULL;
    }

  if (n <= 0)
    {
      errno = ERANGE;
      return NULL;
    }

  for (i = 0; i < n; i++)
    {
      if (bson_size (docs[i]) <= 0)
	{
	  errno = EINVAL;
	  return NULL;
	}
      dsize += bson_size (docs[i]);
    }

  p = lmc_new0 (mongo_wire_packet_t);
  p->header.id = LMC_INT32_TO_LE (id);
  p->header.opcode = LMC_INT32_TO_LE (OP_INSERT);

  pos = sizeof (int32_t) + strlen (ns) + 1;
  p->data_size = pos + dsize;
  p->data = lmc_alloc (uint8_t, p->data_size);

  memcpy (p->data, (void *)&zero, sizeof (int32_t));
  memcpy (p->data + sizeof (int32_t), (void *)ns, strlen (ns) + 1);

  for (i = 0; i < n; i++)
    {
      memcpy (p->data + pos, bson_data (docs[i]), bson_size (docs[i]));
      pos += bson_size (docs[i]);
    }

  p->header.length = LMC_INT32_TO_LE (sizeof (p->header) + p->data_size);

  return p;
}

mongo_wire_packet_t *
mongo_wire_cmd_insert (int32_t id, const char *ns, ...)
{
  mongo_wire_packet_t *p;
  bson_t **docs, *d;
  int32_t n = 0;
  va_list ap;

  if (!ns)
    {
      errno = EINVAL;
      return NULL;
    }

  docs = lmc_new0 (bson_t *);

  va_start (ap, ns);
  while ((d = (bson_t *)va_arg (ap, void *)))
    {
      if (bson_size (d) < 0)
	{
	  free (docs);
	  errno = EINVAL;
	  return NULL;
	}

      docs = (bson_t **)lmc_realloc (docs, (n + 1) * sizeof (bson_t *));
      docs[n++] = d;
    }
  va_end (ap);

  p = mongo_wire_cmd_insert_n (id, ns, n, (const bson_t **)docs);
  free (docs);
  return p;
}

mongo_wire_packet_t *
mongo_wire_cmd_query (int32_t id, const char *ns, int32_t flags,
		      int32_t skip, int32_t ret, const bson_t *query,
		      const bson_t *sel)
{
  mongo_wire_packet_t *p;
  int32_t tmp, nslen;

  if (!ns || !query)
    {
      errno = EINVAL;
      return NULL;
    }

  if (bson_size (query) < 0 || (sel && bson_size (sel) < 0))
    {
      errno = EINVAL;
      return NULL;
    }

  p = lmc_new0 (mongo_wire_packet_t);
  p->header.id = LMC_INT32_TO_LE (id);
  p->header.opcode = LMC_INT32_TO_LE (OP_QUERY);

  nslen = strlen (ns) + 1;
  p->data_size =
    sizeof (int32_t) + nslen + sizeof (int32_t) * 2 + bson_size (query);

  if (sel)
    p->data_size += bson_size (sel);
  p->data = lmc_alloc (uint8_t, p->data_size);

  tmp = LMC_INT32_TO_LE (flags);
  memcpy (p->data, (void *)&tmp, sizeof (int32_t));
  memcpy (p->data + sizeof (int32_t), (void *)ns, nslen);
  tmp = LMC_INT32_TO_LE (skip);
  memcpy (p->data + sizeof (int32_t) + nslen, (void *)&tmp, sizeof (int32_t));
  tmp = LMC_INT32_TO_LE (ret);
  memcpy (p->data + sizeof (int32_t) * 2 + nslen,
	  (void *)&tmp, sizeof (int32_t));
  memcpy (p->data + sizeof (int32_t) * 3 + nslen, bson_data (query),
	  bson_size (query));

  if (sel)
    memcpy (p->data + sizeof (int32_t) * 3 + nslen + bson_size (query),
	    bson_data (sel), bson_size (sel));

  p->header.length = LMC_INT32_TO_LE (sizeof (p->header) + p->data_size);

  return p;
}

mongo_wire_packet_t *
mongo_wire_cmd_get_more (int32_t id, const char *ns,
			 int32_t ret, int64_t cursor_id)
{
  mongo_wire_packet_t *p;
  int32_t t_ret, nslen;
  int64_t t_cid;

  if (!ns)
    {
      errno = EINVAL;
      return NULL;
    }

  p = lmc_new0 (mongo_wire_packet_t);
  p->header.id = LMC_INT32_TO_LE (id);
  p->header.opcode = LMC_INT32_TO_LE (OP_GET_MORE);

  t_ret = LMC_INT32_TO_LE (ret);
  t_cid = LMC_INT64_TO_LE (cursor_id);

  nslen = strlen (ns) + 1;
  p->data_size = sizeof (int32_t) + nslen +
    sizeof (int32_t) + sizeof (int64_t);
  p->data = lmc_alloc (uint8_t, p->data_size);

  memcpy (p->data, (void *)&zero, sizeof (int32_t));
  memcpy (p->data + sizeof (int32_t), (void *)ns, nslen);
  memcpy (p->data + sizeof (int32_t) + nslen, (void *)&t_ret, sizeof (int32_t));
  memcpy (p->data + sizeof (int32_t) * 2 + nslen,
	  (void *)&t_cid, sizeof (int64_t));

  p->header.length = LMC_INT32_TO_LE (sizeof (p->header) + p->data_size);

  return p;
}

mongo_wire_packet_t *
mongo_wire_cmd_delete (int32_t id, const char *ns,
		       int32_t flags, const bson_t *sel)
{
  mongo_wire_packet_t *p;
  int32_t t_flags, nslen;

  if (!ns || !sel)
    {
      errno = EINVAL;
      return NULL;
    }

  if (bson_size (sel) < 0)
    {
      errno = EINVAL;
      return NULL;
    }

  p = lmc_new0 (mongo_wire_packet_t);
  p->header.id = LMC_INT32_TO_LE (id);
  p->header.opcode = LMC_INT32_TO_LE (OP_DELETE);

  nslen = strlen (ns) + 1;
  p->data_size = sizeof (int32_t) + nslen + sizeof (int32_t) + bson_size (sel);
  p->data = lmc_alloc (uint8_t, p->data_size);

  t_flags = LMC_INT32_TO_LE (flags);

  memcpy (p->data, (void *)&zero, sizeof (int32_t));
  memcpy (p->data + sizeof (int32_t), (void *)ns, nslen);
  memcpy (p->data + sizeof (int32_t) + nslen,
	  (void *)&t_flags, sizeof (int32_t));
  memcpy (p->data + sizeof (int32_t) * 2 + nslen,
	  bson_data (sel), bson_size (sel));

  p->header.length = LMC_INT32_TO_LE (sizeof (p->header) + p->data_size);

  return p;
}

mongo_wire_packet_t *
mongo_wire_cmd_kill_cursors_va (int32_t id, int32_t n, va_list ap)
{
  mongo_wire_packet_t *p;
  int32_t i, t_n, pos;
  int64_t t_cid;

  p = lmc_new0 (mongo_wire_packet_t);
  p->header.id = LMC_INT32_TO_LE (id);
  p->header.opcode = LMC_INT32_TO_LE (OP_KILL_CURSORS);

  p->data_size = sizeof (int32_t) + sizeof (int32_t) + sizeof (int64_t) * n;
  p->data = lmc_alloc (uint8_t, p->data_size);

  t_n = LMC_INT32_TO_LE (n);
  pos = sizeof (int32_t) * 2;
  memcpy (p->data, (void *)&zero, sizeof (int32_t));
  memcpy (p->data + sizeof (int32_t), (void *)&t_n, sizeof (int32_t));

  for (i = 1; i <= n; i++)
    {
      t_cid = va_arg (ap, int64_t);
      t_cid = LMC_INT64_TO_LE (t_cid);

      memcpy (p->data + pos, (void *)&t_cid, sizeof (int64_t));
      pos += sizeof (int64_t);
    }

  p->header.length = LMC_INT32_TO_LE (sizeof (p->header) + p->data_size);

  return p;
}

mongo_wire_packet_t *
mongo_wire_cmd_kill_cursors (int32_t id, int32_t n, ...)
{
  va_list ap;
  mongo_wire_packet_t *p;

  if (n <= 0)
    {
      errno = EINVAL;
      return NULL;
    }

  va_start (ap, n);
  p = mongo_wire_cmd_kill_cursors_va (id, n, ap);
  va_end (ap);

  return p;
}

mongo_wire_packet_t *
mongo_wire_cmd_custom (int32_t id, const char *db, int32_t flags,
		       const bson_t *command)
{
  mongo_wire_packet_t *p;
  char *ns;
  bson_t *empty;

  if (!db || !command)
    {
      errno = EINVAL;
      return NULL;
    }

  if (bson_size (command) < 0)
    {
      errno = EINVAL;
      return NULL;
    }

  ns = lmc_alloc (char, strlen (db) + 8);
  memcpy (ns, db, strlen (db));
  memcpy (ns + strlen (db), ".$cmd\0", 6);

  empty = bson_finish (bson_new ());

  p = mongo_wire_cmd_query (id, ns, flags, 0, 1, command, empty);
  free (ns);
  bson_free (empty);
  return p;
}

lmc_bool_t
mongo_wire_reply_packet_get_header (const mongo_wire_packet_t *p,
				    mongo_wire_reply_packet_header_t *hdr)
{
  mongo_wire_reply_packet_header_t h;
  const uint8_t *data;

  if (!p || !hdr)
    {
      errno = EINVAL;
      return FALSE;
    }

  if (p->header.opcode != OP_REPLY)
    {
      errno = EPROTO;
      return FALSE;
    }

  if (mongo_wire_packet_get_data (p, &data) == -1)
    return FALSE;

  memcpy (&h, data, sizeof (mongo_wire_reply_packet_header_t));

  hdr->flags = LMC_INT32_FROM_LE (h.flags);
  hdr->cursor_id = LMC_INT64_FROM_LE (h.cursor_id);
  hdr->start = LMC_INT32_FROM_LE (h.start);
  hdr->returned = LMC_INT32_FROM_LE (h.returned);

  return TRUE;
}

lmc_bool_t
mongo_wire_reply_packet_get_data (const mongo_wire_packet_t *p,
				  const uint8_t **data)
{
  const uint8_t *d;

  if (!p || !data)
    {
      errno = EINVAL;
      return FALSE;
    }

  if (p->header.opcode != OP_REPLY)
    {
      errno = EPROTO;
      return FALSE;
    }

  if (mongo_wire_packet_get_data (p, &d) == -1)
    return FALSE;

  *data = d + sizeof (mongo_wire_reply_packet_header_t);
  return TRUE;
}

lmc_bool_t
mongo_wire_reply_packet_get_nth_document (const mongo_wire_packet_t *p,
					  int32_t n,
					  bson_t **doc)
{
  const uint8_t *d;
  mongo_wire_reply_packet_header_t h;
  int32_t i, pos = 0;

  if (!p || !doc || n <= 0)
    {
      errno = EINVAL;
      return FALSE;
    }

  if (p->header.opcode != OP_REPLY)
    {
      errno = EPROTO;
      return FALSE;
    }

  if (!mongo_wire_reply_packet_get_header (p, &h))
    return FALSE;

  if (h.returned < n)
    {
      errno = ERANGE;
      return FALSE;
    }

  if (!mongo_wire_reply_packet_get_data (p, &d))
    return FALSE;

  for (i = 1; i < n; i++)
    pos += bson_stream_doc_size (d, pos);

  *doc = bson_new_from_data (d + pos, bson_stream_doc_size (d, pos) - 1);
  return TRUE;
}
