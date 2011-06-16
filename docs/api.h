/** @page api API Documentation
 *
 * On these pages, one will find the complete API documentation.
 *
 * @section Structure
 *
 * The library can be split into four major parts:
 *   - bson: The low-level BSON implementation. @see bson_mod
 *   - mongo-wire: Functions to construct packets that can be sent
 *     later. @see mongo_wire
 *   - mongo-client: The high-level API that deals with the
 *     network. @see mongo_client
 *   - mongo-utils: Various miscellaneous utilities related to either
 *     BSON or MongoDB. @see mongo_util
 *   - mongo-sync: Wrappers around the wire commands, that talk to the
 *     network aswell, in a synchronous, blocking manner. @see
 *     mongo_sync.
 *   - mongo-sync-cursor: Cursor API on top of the synchronous
 *     API. @see mongo_sync_cursor.
 *   - mongo-sync-pool: Simple connection pooling on top of
 *     mongo-sync, @see mongo_sync_pool_api.
 *
 * The intended way to use the library to work with MongoDB is to
 * first construct the BSON objects, then construct the packets, and
 * finally send the packets out to the network.
 *
 * The reason behind the split between packet construction and sending
 * is because we want to be able to construct packets in one thread
 * and send it in another, if so need be.
 *
 * This split also allows scenarios where the packet must be sent over
 * a medium the library was not prepared for (such as an SSL tunnel),
 * or when the packet is supposed to be sent to multiple destinations
 * (for whatever reason - perhaps for being logged to a file for
 * debugging purposes).
 *
 * All in all, this separation between modules provides a great deal
 * of flexibility.

 */
