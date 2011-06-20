/* sync-gridfs.h - libmong-client GridFS API
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

/** @file src/sync-gridfs.h
 * MongoDB GridFS API.
 *
 * @addtogroup mongo_sync
 * @{
 */

#ifndef LIBMONGO_SYNC_GRIDFS_H
#define LIBMONGO_SYNC_GRIDFS_H 1

#include <mongo-sync.h>
#include <mongo-sync-cursor.h>
#include <glib.h>

G_BEGIN_DECLS

/** @defgroup mongo_sync_gridfs_api Mongo GridFS API
 *
 * @addtogroup mongo_sync_gridfs_api
 * @{
 */

/** Opaque GridFS object. */
typedef struct _mongo_sync_gridfs mongo_sync_gridfs;

/** Opaque GridFS file object. */
typedef struct _mongo_sync_gridfs_file mongo_sync_gridfs_file;

/** Create a new GridFS object.
 *
 * @param conn is the MongoDB connection to base the filesystem object
 * on.
 * @param ns_prefix is the prefix the GridFS collections should be
 * under.
 *
 * @returns A newly allocated GridFS object, or NULL on error.
 */
mongo_sync_gridfs *mongo_sync_gridfs_new (mongo_sync_connection *conn,
					  const gchar *ns_prefix);

/** Close and free a GridFS object.
 *
 * @param gfs is the GridFS object to free up.
 * @param disconnect signals whether to free the underlying connection
 * aswell.
 */
void mongo_sync_gridfs_free (mongo_sync_gridfs *gfs, gboolean disconnect);

/** Get the default chunk size of a GridFS object.
 *
 * @param gfs is the GridFS object to get the default chunk size of.
 *
 * @returns The chunk size in bytes, or -1 on error.
 */
gint32 mongo_sync_gridfs_get_chunk_size (mongo_sync_gridfs *gfs);

/** Set the default chunk size of a GridFS object.
 *
 * @param gfs is the GridFS object to set the default chunk size of.
 * @param chunk_size is the desired default chunk size.
 *
 * @returns TRUE on success, FALSE otherwise.
 */
gboolean mongo_sync_gridfs_set_chunk_size (mongo_sync_gridfs *gfs,
					   gint32 chunk_size);


/** @defgroup mongo_sync_gridfs_fops Mongo GridFS File Operations
 *
 * @addtogroup mongo_sync_gridfs_fops
 * @{
 */

/** Find a file on GridFS.
 *
 * Finds a file on GridFS, based on a custom query.
 *
 * @param gfs is the GridFS to find the file in.
 * @param query is the custom query based on which the file shall be
 * sought.
 *
 * @returns A newly allocated file object, or NULL on error. It is the
 * responsibility of the caller to free the returned object once it is
 * no longer needed.
 */
mongo_sync_gridfs_file *mongo_sync_gridfs_find (mongo_sync_gridfs *gfs,
						const bson *query);

/** List GridFS files matching a query.
 *
 * Finds all files on a GridFS, based on a custom query.
 *
 * @param gfs is the GridFS to list files from.
 * @param query is the custom query based on which files shall be
 * sought. Passing a NULL query will find all files, without
 * restriction.
 *
 * @returns A newly allocated cursor object, or NULL on error. It is
 * the responsibility of the caller to free the returned cursor once
 * it is no longer needed.
 */
mongo_sync_cursor *mongo_sync_gridfs_list (mongo_sync_gridfs *gfs,
					   const bson *query);

/** Upload a file to GridFS from a buffer.
 *
 * Create a new file on GridFS from a buffer, using custom meta-data.
 *
 * @param gfs is the GridFS to create the file on.
 * @param metadata is the (optional) file metadata.
 * @param data is the data to store on GridFS.
 * @param size is the size of the data.
 *
 * @returns A newly allocated file object, or NULL on error. It is the
 * responsibility of the caller to free the returned object once it is
 * no longer needed.
 *
 * @note The metadata MUST NOT contain any of the required GridFS
 * metadata fields (_id, length, chunkSize, uploadDate, md5),
 * otherwise a conflict will occurr, against which the function does
 * not guard by design.
 */
mongo_sync_gridfs_file *mongo_sync_gridfs_file_new_from_buffer (mongo_sync_gridfs *gfs,
								const bson *metadata,
								const guint8 *data,
								gint64 size);

/** Free a GridFS File object.
 *
 * @param gfile is the file object to free.
 */
void mongo_sync_gridfs_file_free (mongo_sync_gridfs_file *gfile);

/** @} */

/** @defgroup mongo_sync_gridfs_file Mongo GridFS file objects
 *
 * @addtogroup mongo_sync_gridfs_file
 * @{
*/

/* Metadata */

/** Get the file ID of a GridFS file.
 *
 * @param gfile is the GridFS file to work with.
 *
 * @returns The ObjectID of the file, or NULL on error. The returned
 * pointer points to an internal area, and should not be modified or
 * freed, and is only valid as long as the file object is valid.
 */
const guint8 *mongo_sync_gridfs_file_get_id (mongo_sync_gridfs_file *gfile);

/** Get the length of a GridFS file.
 *
 * @param gfile is the GridFS file to work with.
 *
 * @returns The length of the file, or -1 on error.
 */
gint64 mongo_sync_gridfs_file_get_length (mongo_sync_gridfs_file *gfile);

/** Get the chunk size of a GridFS file.
 *
 * @param gfile is the GridFS file to work with.
 *
 * @returns The maximum size of the chunks of the file, or -1 on error.
 */
gint32 mongo_sync_gridfs_file_get_chunk_size (mongo_sync_gridfs_file *gfile);

/** Get the MD5 digest of a GridFS file.
 *
 * @param gfile is the GridFS file to work with.
 *
 * @returns The MD5 digest of the file, or NULL on error. The returned
 * pointer points to an internal area, and should not be modified or
 * freed, and is only valid as long as the file object is valid.
 */
const gchar *mongo_sync_gridfs_file_get_md5 (mongo_sync_gridfs_file *gfile);

/** Get the upload date of a GridFS file.
 *
 * @param gfile is the GridFS file to work with.
 *
 * @returns The upload date of the file, or -1 on error.
 */
gint64 mongo_sync_gridfs_file_get_date (mongo_sync_gridfs_file *gfile);

/** Get the full metadata of a GridFS file
 *
 * @param gfile is the GridFS file to work with.
 *
 * @returns A BSON object containing the full metadata, or NULL on
 * error. The returned pointer points to an internal area, and should
 * not be modified or freed, and is only valid as long as the file
 * object is valid.
 */
const bson *mongo_sync_gridfs_file_get_metadata (mongo_sync_gridfs_file *gfile);

/** Get the number of chunks in a GridFS file.
 *
 * @param gfile is the GridFS file to work with.
 *
 * @returns The number of chunks in the GridFS file, or -1 on error.
 */
gint64 mongo_sync_gridfs_file_get_chunks (mongo_sync_gridfs_file *gfile);

/* Data access */

/** Create a cursor for a GridFS file.
 *
 * The cursor can be used (via
 * mongo_sync_gridfs_file_cursor_get_chunk()) to retrieve a GridFS
 * file chunk by chunk.
 *
 * @param gfile is the GridFS file to work with.
 * @param start is the starting chunk.
 * @param num is the total number of chunks to make a cursor for.
 *
 * @returns A newly allocated cursor object, or NULL on error. It is
 * the responsibility of the caller to free the cursor once it is no
 * longer needed.
 */
mongo_sync_cursor *mongo_sync_gridfs_file_cursor_new (mongo_sync_gridfs_file *gfile,
						      gint start, gint num);

/** Get the data of a GridFS file chunk, via a cursor.
 *
 * Once we have a cursor, it can be iterated over with
 * mongo_sync_cursor_next(), and its data can be conveniently accessed
 * with this function.
 *
 * @param cursor is the cursor object to work with.
 * @param size is a pointer to a variable where the chunk's actual
 * size can be stored.
 *
 * @returns A pointer to newly allocated memory that holds the current
 * chunk's data, or NULL on error. It is the responsibility of the
 * caller to free this once it is no longer needed.
 */
guint8 *mongo_sync_gridfs_file_cursor_get_chunk (mongo_sync_cursor *cursor,
						 gint32 *size);

/** @} */

G_END_DECLS

/** @} */

/** @} */

#endif
