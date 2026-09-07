/* Copyright (c) PubNub Inc. */
/* See LICENSE in the root directory of this source tree. */

#ifndef PUBNUB_FEATURE_HISTORY_H
#define PUBNUB_FEATURE_HISTORY_H

#include "pubnub/config.h"

#if PUBNUB_ENABLE_HISTORY

#include "pubnub/error.h"
#include "pubnub/future.h"
#include "pubnub/providers/serialization.h"
#include "pubnub/pubnub_compat.h"
#include "pubnub/types.h"

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
// clang-format off
extern "C" {
// clang-format on
#endif

/**
 * @brief Options for @c pubnub_fetch_messages.
 *
 * Initialize with @c PUBNUB_FETCH_MESSAGES_OPTS_INIT to get
 * protocol-correct defaults (@c include_uuid and
 * @c include_message_type default to ON).
 */
typedef struct pubnub_fetch_messages_opts {
    /**
     * @brief Comma-separated channel names (@b required, @b borrowed,
     *        NUL-terminated).
     *
     * Pass a single channel name or multiple channels separated by
     * commas (e.g., @c "ch1,ch2,ch3"). When
     * @c include_message_actions is set, only a single channel is
     * allowed.
     */
    const char* channels;

    /**
     * @brief Exclusive start timetoken (@b optional, @b borrowed,
     *        NUL-terminated).
     *
     * Messages older than this timetoken are returned. Pass @c NULL
     * to omit (fetch from newest available).
     */
    const char* start;

    /**
     * @brief Inclusive end timetoken (@b optional, @b borrowed,
     *        NUL-terminated).
     *
     * Messages newer than or equal to this timetoken are returned.
     * Pass @c NULL to omit.
     */
    const char* end;

    /**
     * @brief Maximum number of messages per channel.
     *
     * @b Default: @c 0 (server default: 25 for multi-channel /
     * with-actions, 100 for single channel without actions).
     *
     * Clamped server-side to 25 for multi-channel or with-actions
     * requests, and 100 for single-channel requests.
     */
    uint16_t count;

    /**
     * @brief Reverse chronological order.
     *
     * @c 0 = newest first (default), non-zero = oldest first.
     */
    uint8_t reverse;

    /**
     * @brief Include message metadata.
     *
     * @c 0 = omit, non-zero = include the @c meta field in results.
     */
    uint8_t include_meta;

    /**
     * @brief Include publisher UUID.
     *
     * @b Default: @c 1 (ON via @c PUBNUB_FETCH_MESSAGES_OPTS_INIT).
     */
    uint8_t include_uuid;

    /**
     * @brief Include message type discriminator.
     *
     * @b Default: @c 1 (ON via @c PUBNUB_FETCH_MESSAGES_OPTS_INIT).
     */
    uint8_t include_message_type;

    /**
     * @brief Include user-supplied custom message type label.
     *
     * @c 0 = omit (default), non-zero = include.
     */
    uint8_t include_custom_message_type;

    /**
     * @brief Include message actions in the response.
     *
     * When enabled, the request is routed to the history-with-actions
     * endpoint. Only a single channel is allowed; maximum count is
     * clamped to 25.
     *
     * @c 0 = omit (default), non-zero = include.
     */
    uint8_t include_message_actions;

    /**
     * @brief Per-request timeout override in milliseconds (@b optional).
     *
     * @note When non-zero, takes priority over the context-level
     *       @c pubnub_config_t::transaction_timeout_ms.
     */
    uint32_t timeout_ms;
} pubnub_fetch_messages_opts_t;

/**
 * @brief Designated-initializer macro for @c
 *        pubnub_fetch_messages_opts_t.
 *
 * Sets @c include_uuid and @c include_message_type to 1 (protocol
 * defaults). All other fields zero-initialize.
 */
#define PUBNUB_FETCH_MESSAGES_OPTS_INIT \
    {.include_uuid = 1, .include_message_type = 1}

/**
 * @brief Options for @c pubnub_delete_messages.
 *
 * Initialize with @c PUBNUB_DELETE_MESSAGES_OPTS_INIT (zero-init
 * produces valid defaults).
 */
typedef struct pubnub_delete_messages_opts {
    /**
     * @brief Single channel name (@b required, @b borrowed,
     *        NUL-terminated).
     *
     * Only one channel per request; commas are not allowed.
     */
    const char* channel;

    /**
     * @brief Exclusive start timetoken (@b optional, @b borrowed,
     *        NUL-terminated).
     *
     * Messages published after this timetoken are deleted. Pass
     * @c NULL to omit (delete from the beginning of stored history).
     */
    const char* start;

    /**
     * @brief Exclusive end timetoken (@b optional, @b borrowed,
     *        NUL-terminated).
     *
     * Messages published before this timetoken are deleted. Pass
     * @c NULL to omit (delete up to the most recent message).
     *
     * @warning The @c end bound is @b exclusive — the message at
     *          @c end is NOT deleted. This differs from
     *          @c pubnub_fetch_messages, whose @c end is inclusive.
     */
    const char* end;

    /**
     * @brief Per-request timeout override in milliseconds (@b optional).
     *
     * @note When non-zero, takes priority over the context-level
     *       @c pubnub_config_t::transaction_timeout_ms.
     */
    uint32_t timeout_ms;
} pubnub_delete_messages_opts_t;

/**
 * @brief Designated-initializer macro for @c
 *        pubnub_delete_messages_opts_t.
 */
#define PUBNUB_DELETE_MESSAGES_OPTS_INIT {0}

/**
 * @brief Options for @c pubnub_message_counts.
 *
 * Initialize with @c PUBNUB_MESSAGE_COUNTS_OPTS_INIT (zero-init
 * produces valid defaults).
 *
 * Exactly one of @c timetoken or @c channels_timetokens must be
 * non-NULL.
 */
typedef struct pubnub_message_counts_opts {
    /**
     * @brief Comma-separated channel names (@b required, @b borrowed,
     *        NUL-terminated).
     */
    const char* channels;

    /**
     * @brief Single timetoken applied to all channels (@b borrowed,
     *        NUL-terminated).
     *
     * Counts messages published after this timetoken. Mutually
     * exclusive with @c channels_timetokens.
     */
    const char* timetoken;

    /**
     * @brief Per-channel timetokens, comma-separated (@b borrowed,
     *        NUL-terminated).
     *
     * One timetoken per channel in the same order as @c channels.
     * Mutually exclusive with @c timetoken.
     */
    const char* channels_timetokens;

    /**
     * @brief Per-request timeout override in milliseconds (@b optional).
     *
     * @note When non-zero, takes priority over the context-level
     *       @c pubnub_config_t::transaction_timeout_ms.
     */
    uint32_t timeout_ms;
} pubnub_message_counts_opts_t;

/**
 * @brief Designated-initializer macro for @c
 *        pubnub_message_counts_opts_t.
 */
#define PUBNUB_MESSAGE_COUNTS_OPTS_INIT {0}

/**
 * @brief Per-message result from a fetch-messages response.
 *
 * Pointer fields alias internal parsed data and are valid until
 * @c pubnub_future_release is called on the owning future.
 */
typedef struct pubnub_history_message_result {
    /**
     * @brief Message payload node.
     *
     * Access via the serialization vtable: value_as_string() for
     * string messages, object_get()/array_get() for JSON
     * object/array messages. @c NULL when absent. Valid until
     * @c pubnub_future_release.
     *
     * When a crypto module is configured and decryption succeeds,
     * this field points to the decrypted and re-parsed tree. When
     * decryption fails, this field retains the raw bytes received
     * from the server and @c crypto_result is set to
     * @c PUBNUB_ERR_CRYPTO. Calling the accessor with different
     * channel/message indices invalidates the previously returned
     * decrypted node. Copy data before advancing.
     */
    const pubnub_json_value_t* message;

    /** Server-assigned publish timetoken. */
    pubnub_timetoken_t timetoken;

    /**
     * @brief Publisher UUID.
     *
     * Empty (@c .len == 0) when @c include_uuid was not set.
     */
    pubnub_string_view_t uuid;

    /**
     * @brief Message metadata node.
     *
     * Access via the serialization vtable. @c NULL when
     * @c include_meta was not set or the message carried no
     * metadata. Valid until @c pubnub_future_release.
     */
    const pubnub_json_value_t* meta;

    /**
     * @brief User-supplied custom message type label.
     *
     * Empty when @c include_custom_message_type was not set or
     * the publisher did not supply one.
     */
    pubnub_string_view_t custom_message_type;

    /**
     * @brief SDK-assigned event type discriminator.
     *
     * @c PUBNUB_EVENT_TYPE_UNKNOWN when @c include_message_type
     * was not set in the request options.
     */
    pubnub_event_type_t event_type;

    /**
     * @brief Per-message crypto status.
     *
     * @c PUBNUB_OK when decryption succeeded or no crypto module is
     * configured. @c PUBNUB_ERR_CRYPTO when a crypto module IS
     * configured but decryption failed (e.g. the stored message is
     * plaintext, or was encrypted with a different key). In this
     * case @c message still points to the raw bytes received from
     * the server.
     *
     * @note Valid only until @c pubnub_future_release.
     */
    pubnub_res_t crypto_result;
} pubnub_history_message_result_t;

PUBNUB_STATIC_ASSERT(sizeof(pubnub_history_message_result_t) <= 96U,
                     "pubnub_history_message_result_t exceeds 96-byte budget");

/**
 * @brief File metadata extracted from a file-type message.
 *
 * Call @c pubnub_fetch_messages_result_file_at when
 * @c event_type == @c PUBNUB_EVENT_TYPE_FILE to extract file
 * details from the message payload.
 *
 * All string view fields alias internal parsed data and are valid
 * until @c pubnub_future_release is called on the owning future.
 * For encrypted file messages, views are valid until the next call
 * to @c pubnub_fetch_messages_result_file_at or until
 * @c pubnub_future_release, whichever comes first.
 */
typedef struct pubnub_history_file_result {
    /** File identifier (UUID-style). */
    pubnub_string_view_t id;

    /** Original filename. */
    pubnub_string_view_t name;

    /**
     * @brief Optional user-attached JSON from the file upload.
     *
     * Empty when the publisher did not attach a message to the
     * file upload.
     */
    pubnub_string_view_t message;
} pubnub_history_file_result_t;

/**
 * @brief Top-level result for @c pubnub_fetch_messages.
 *
 * Use @c channel_count as the loop bound for indexed channel access
 * via @c pubnub_fetch_messages_result_channel_at.
 *
 * When @c next.len is non-zero the server signalled that additional
 * messages are available. Pass @c next as the @c start (or @c end)
 * field of the next @c pubnub_fetch_messages_opts_t to page forward.
 * The view is valid until @c pubnub_future_release is called on the
 * owning future.
 */
typedef struct pubnub_fetch_messages_result {
    /** Number of channels in the response. */
    uint32_t channel_count;

    /**
     * @brief Next-page cursor timetoken.
     *
     * Non-empty when the server returned a @c "more" object indicating
     * additional messages are available. Pass its string value as the
     * @c start (exclusive upper bound) on the next
     * @c pubnub_fetch_messages call to retrieve older messages.
     *
     * Zero-length view (@c .len == 0) means this is the last page.
     * Valid until @c pubnub_future_release.
     */
    pubnub_timetoken_t next;
} pubnub_fetch_messages_result_t;

/**
 * @brief Per-channel result within a fetch-messages response.
 *
 * Use @c message_count as the loop bound for indexed message access
 * via @c pubnub_fetch_messages_result_message_at.
 */
typedef struct pubnub_fetch_messages_channel_result {
    /** Channel name (aliases internal data). */
    pubnub_string_view_t name;

    /** Number of messages returned for this channel. */
    uint32_t message_count;
} pubnub_fetch_messages_channel_result_t;

/**
 * @brief Top-level result for @c pubnub_message_counts.
 *
 * Use @c channel_count as the loop bound for indexed channel access
 * via @c pubnub_message_counts_result_channel_at.
 */
typedef struct pubnub_message_counts_result {
    /** Number of channels in the response. */
    uint32_t channel_count;
} pubnub_message_counts_result_t;

/**
 * @brief Per-channel unread message count.
 *
 * Returned by @c pubnub_message_counts_result_channel_at.
 */
typedef struct pubnub_message_counts_channel_result {
    /** Channel name (aliases internal data). */
    pubnub_string_view_t name;

    /** Number of messages published after the given timetoken. */
    uint32_t count;
} pubnub_message_counts_channel_result_t;

/**
 * @brief Fetch messages from one or more channels (v3 batch history).
 *
 * Drive the returned future via cooperative polling
 * (@c pubnub_process + @c pubnub_future_is_ready), blocking
 * await (@c pubnub_await), or async callback (@c pubnub_async).
 *
 * Cooperative polling example
 * @code
 * pubnub_fetch_messages_opts_t opts =
 *     PUBNUB_FETCH_MESSAGES_OPTS_INIT;
 * opts.channels = "ch1,ch2";
 * opts.count = 50;
 * opts.include_meta = 1;
 *
 * pubnub_future_t fut = pubnub_fetch_messages(ctx, &opts);
 * while (!pubnub_future_is_ready(fut)) {
 *     pubnub_process(ctx);
 * }
 *
 * if (PUBNUB_OK == pubnub_future_status(fut)) {
 *     pubnub_fetch_messages_result_t r =
 *         pubnub_fetch_messages_result(fut);
 *     pubnub_serialization_provider_t* serial = pubnub_serialization(ctx);
 *     for (size_t i = 0; i < r.channel_count; ++i) {
 *         pubnub_fetch_messages_channel_result_t ch =
 *             pubnub_fetch_messages_result_channel_at(fut, i);
 *         for (size_t j = 0; j < ch.message_count; ++j) {
 *             pubnub_history_message_result_t msg =
 *                 pubnub_fetch_messages_result_message_at(
 *                     fut, i, j);
 *             size_t mlen = 0;
 *             const char* mptr =
 *                 serial->value_as_string(msg.message, &mlen);
 *             printf("[%.*s] %.*s\n",
 *                    (int)msg.timetoken.len,
 *                    msg.timetoken.ptr,
 *                    (int)mlen,
 *                    mptr ? mptr : "");
 *         }
 *     }
 * }
 * pubnub_future_release(fut);
 * @endcode
 *
 * On validation failure (NULL channels, multi-channel with actions,
 * missing subscribe_key, or queue full) the returned future carries
 * an immediate error code readable via @c pubnub_future_status.
 *
 * @note Required context configuration: @c subscribe_key must be
 *       set in @c pubnub_config_t.
 *
 * @param ctx   Initialized context (@b borrowed).
 * @param opts  Fetch-messages options struct (@b borrowed).
 * @return Future handle (owned by caller). Release via
 *         @c pubnub_future_release after reading results.
 *
 * @see pubnub_fetch_messages_result
 * @see pubnub_future_release
 * @see pubnub_fetch_messages_opts_t
 * @see pubnub_context_t
 * @see pubnub_future_t
 */
PUBNUB_API pubnub_future_t
pubnub_fetch_messages(pubnub_context_t*                   ctx,
                      const pubnub_fetch_messages_opts_t* opts);

/**
 * @brief Delete messages from a single channel's history.
 *
 * On success the future status is @c PUBNUB_OK. There is no result
 * data to extract — deletion is confirmed by the status code alone.
 *
 * Example
 * @code
 * pubnub_delete_messages_opts_t opts =
 *     PUBNUB_DELETE_MESSAGES_OPTS_INIT;
 * opts.channel = "my-channel";
 * opts.end = "17001234567890123";
 *
 * pubnub_future_t fut = pubnub_delete_messages(ctx, &opts);
 * pubnub_res_t st = pubnub_await(fut);
 * if (PUBNUB_OK != st) {
 *     pubnub_string_view_t err =
 *         pubnub_response_error_message(fut);
 *     printf("delete failed: %.*s\n",
 *            (int)err.len, err.ptr);
 * }
 * pubnub_future_release(fut);
 * @endcode
 *
 * @note Required context configuration: @c subscribe_key must be
 *       set in @c pubnub_config_t.
 *
 * @param ctx   Initialized context (@b borrowed).
 * @param opts  Delete-messages options struct (@b borrowed).
 * @return Future handle (owned by caller). Release via
 *         @c pubnub_future_release after reading results.
 *
 * @see pubnub_future_release
 * @see pubnub_delete_messages_opts_t
 * @see pubnub_context_t
 * @see pubnub_future_t
 */
PUBNUB_API pubnub_future_t
pubnub_delete_messages(pubnub_context_t*                    ctx,
                       const pubnub_delete_messages_opts_t* opts);

/**
 * @brief Count unread messages per channel since a given timetoken.
 *
 * Example
 * @code
 * pubnub_message_counts_opts_t opts =
 *     PUBNUB_MESSAGE_COUNTS_OPTS_INIT;
 * opts.channels = "ch1,ch2";
 * opts.timetoken = "17001234567890123";
 *
 * pubnub_future_t fut = pubnub_message_counts(ctx, &opts);
 * pubnub_res_t st = pubnub_await(fut);
 * if (PUBNUB_OK == st) {
 *     pubnub_message_counts_result_t r =
 *         pubnub_message_counts_result(fut);
 *     for (size_t i = 0; i < r.channel_count; ++i) {
 *         pubnub_message_counts_channel_result_t ch =
 *             pubnub_message_counts_result_channel_at(fut, i);
 *         printf("%.*s: %u\n",
 *                (int)ch.name.len, ch.name.ptr, ch.count);
 *     }
 * }
 * pubnub_future_release(fut);
 * @endcode
 *
 * Exactly one of @c opts->timetoken or @c opts->channels_timetokens
 * must be non-NULL. If both or neither are set, the returned future
 * carries @c PUBNUB_ERR_INVALID_ARGUMENT.
 *
 * @note Required context configuration: @c subscribe_key must be
 *       set in @c pubnub_config_t.
 *
 * @param ctx   Initialized context (@b borrowed).
 * @param opts  Message-counts options struct (@b borrowed).
 * @return Future handle (owned by caller). Release via
 *         @c pubnub_future_release after reading results.
 *
 * @see pubnub_message_counts_result
 * @see pubnub_future_release
 * @see pubnub_message_counts_opts_t
 * @see pubnub_context_t
 * @see pubnub_future_t
 */
PUBNUB_API pubnub_future_t
pubnub_message_counts(pubnub_context_t*                   ctx,
                      const pubnub_message_counts_opts_t* opts);

/**
 * @brief Read the top-level fetch-messages result.
 *
 * Triggers lazy parsing of the response on first call.
 *
 * @param future Future returned from @c pubnub_fetch_messages.
 * @return Result struct with @c channel_count, or zero-initialized
 *         if the future is not ready or carries an error.
 */
PUBNUB_API pubnub_fetch_messages_result_t
pubnub_fetch_messages_result(pubnub_future_t future);

/**
 * @brief Read per-channel metadata at @p index.
 *
 * @param future Future returned from @c pubnub_fetch_messages.
 * @param index  Channel index in [0, @c channel_count).
 * @return Channel result with name and message count, or
 *         zero-initialized if @p index is out of range.
 */
PUBNUB_API pubnub_fetch_messages_channel_result_t
pubnub_fetch_messages_result_channel_at(pubnub_future_t future, size_t index);

/**
 * @brief Read a single message from the fetch-messages response.
 *
 * Two-level indexing: first by channel, then by message position
 * within that channel's array.
 *
 * When a crypto module is configured on the context, message payloads
 * are automatically decrypted on access. Decryption applies only to
 * regular messages (message_type == 0) with non-trivial payloads.
 * The last-accessed message's decrypted payload is cached until the
 * future is released or a different message is accessed.
 *
 * @param future        Future returned from @c pubnub_fetch_messages.
 * @param channel_index Channel index in [0, @c channel_count).
 * @param message_index Message index in [0, @c message_count).
 * @return Message result struct, or zero-initialized if either
 *         index is out of range or the future is not ready.
 */
PUBNUB_API pubnub_history_message_result_t
pubnub_fetch_messages_result_message_at(pubnub_future_t future,
                                        size_t          channel_index,
                                        size_t          message_index);

/**
 * @brief Extract file metadata from a file-type message.
 *
 * Call this when @c event_type == @c PUBNUB_EVENT_TYPE_FILE.
 * For non-file messages, returns a zero-initialized struct.
 *
 * @param future        Future returned from @c pubnub_fetch_messages.
 * @param channel_index Channel index in [0, @c channel_count).
 * @param message_index Message index in [0, @c message_count).
 * @return File result struct, or zero-initialized if the message
 *         is not a file or indices are out of range.
 *
 * @note   When a crypto module is configured and the message payload
 *         is encrypted, file metadata is extracted from the decrypted
 *         content. The returned views are valid until the next call
 *         to this function or until @c pubnub_future_release.
 */
PUBNUB_API pubnub_history_file_result_t
pubnub_fetch_messages_result_file_at(pubnub_future_t future,
                                     size_t          channel_index,
                                     size_t          message_index);

/**
 * @brief Access message actions JSON for a specific message.
 *
 * Returns the raw JSON value tree representing message actions
 * attached to the message. The tree is owned by the SDK and must
 * NOT be destroyed or modified.
 *
 * @param future        Future returned from @c pubnub_fetch_messages.
 * @param channel_index Channel index in [0, @c channel_count).
 * @param message_index Message index in [0, @c message_count).
 * @return Borrowed JSON value tree, or @c NULL if the message has
 *         no actions, @c include_message_actions was not set, or
 *         indices are out of range.
 */
PUBNUB_API const pubnub_json_value_t*
pubnub_fetch_messages_result_actions_at(pubnub_future_t future,
                                        size_t          channel_index,
                                        size_t          message_index);

/**
 * @brief Read the top-level message-counts result.
 *
 * Triggers lazy parsing of the response on first call.
 *
 * @param future Future returned from @c pubnub_message_counts.
 * @return Result struct with @c channel_count, or zero-initialized
 *         if the future is not ready or carries an error.
 */
PUBNUB_API pubnub_message_counts_result_t
pubnub_message_counts_result(pubnub_future_t future);

/**
 * @brief Read per-channel unread count at @p index.
 *
 * @param future Future returned from @c pubnub_message_counts.
 * @param index  Channel index in [0, @c channel_count).
 * @return Channel result with name and count, or zero-initialized
 *         if @p index is out of range.
 */
PUBNUB_API pubnub_message_counts_channel_result_t
pubnub_message_counts_result_channel_at(pubnub_future_t future, size_t index);

#ifdef __cplusplus
// clang-format off
}
// clang-format on
#endif

#endif /* PUBNUB_ENABLE_HISTORY */

#endif /* PUBNUB_FEATURE_HISTORY_H */
