/* Copyright (c) PubNub Inc. */
/* See LICENSE in the root directory of this source tree. */

#ifndef PUBNUB_FEATURE_PUBLISH_H
#define PUBNUB_FEATURE_PUBLISH_H

#include "pubnub/config.h"

#if PUBNUB_ENABLE_PUBLISH

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

/** Message persistence options. */
typedef enum pubnub_publish_store {
    /** Server honors keyset (@b default). */
    PUBNUB_PUBLISH_STORE_ACCOUNT_DEFAULT = 0,
    /** Persist the message in Message Persistence. */
    PUBNUB_PUBLISH_STORE_YES,
    /** Do not persist. */
    PUBNUB_PUBLISH_STORE_NO
} pubnub_publish_store_t;

/** HTTP method to use when publishing. */
typedef enum pubnub_publish_method {
    /** Message as part of URL path (@b default). */
    PUBNUB_PUBLISH_METHOD_GET = 0,
    /** Message as part of request body. */
    PUBNUB_PUBLISH_METHOD_POST
} pubnub_publish_method_t;

/** Request-body compression preference. */
typedef enum pubnub_publish_compress {
    /**
     * Follow the compile-time @c PUBNUB_ENABLE_REQUEST_COMPRESSION
     * toggle (@b default).
     */
    PUBNUB_PUBLISH_COMPRESS_DEFAULT = 0,
    /** Compress the request body. */
    PUBNUB_PUBLISH_COMPRESS_YES,
    /** Send the request body uncompressed. */
    PUBNUB_PUBLISH_COMPRESS_NO
} pubnub_publish_compress_t;

/**
 * @brief Options for @c pubnub_publish.
 *
 * Initialize with @c PUBNUB_PUBLISH_OPTS_INIT before overriding
 * individual fields.
 *
 * @see pubnub_publish
 */
typedef struct pubnub_publish_opts {
    /**
     * @brief Target channel name (@b required, @b borrowed,
     *        NUL-terminated).
     */
    const char* channel;

    /**
     * @brief Valid JSON string for publish (@b required, @b borrowed).
     *
     * Either NUL-terminated (leave @c message_len at 0) or
     * length-counted (set @c message_len explicitly).
     *
     * @attention Setting both @c message and @c message_value to non-NULL is an
     *            error (@c PUBNUB_ERR_INVALID_ARGUMENT).
     */
    const char* message;

    /**
     * @brief Length of @c message in bytes.
     *
     * @b Default: @c 0 means "call strlen".
     */
    size_t message_len;

    /**
     * @brief Message payload as a JSON value tree (@b required, @b borrowed).
     *
     * Build the tree with helper macros from @c json_macros.h:
     * @code
     * pubnub_serialization_provider_t* json = pubnub_serialization(ctx);
     * pubnub_json_value_t* msg =
     *     PUBNUB_JSON_OBJ(json,
     *                     PUBNUB_JSON_KV_STR(json, "device", "sensor-1"),
     *                     PUBNUB_JSON_KV_INT(json, "temp_c", 35),
     *                     PUBNUB_JSON_KV_BOOL(json, "alarm", 0));
     * @endcode
     *
     * Destroy the tree after @c pubnub_publish returns via
     * @c pubnub_json_destroy. The SDK serializes the tree during
     * the @c pubnub_publish call; the caller retains ownership.
     *
     * For raw vtable usage (building nested arrays/objects without
     * macros), see @c examples/publish/value.c.
     *
     * @note Ownership is @b borrowed — the SDK serializes the tree
     *       during the call but does not free it. Contrast with App
     *       Context @c custom_value fields, which transfer ownership
     *       to the SDK. Call @c pubnub_json_destroy yourself after
     *       @c pubnub_publish returns.
     *
     * @attention Setting both @c message and @c message_value to non-NULL is an
     *            error (@c PUBNUB_ERR_INVALID_ARGUMENT).
     */
    pubnub_json_value_t* message_value;

    /**
     * @brief HTTP method to send the publish request with.
     *
     * With the @c PUBNUB_PUBLISH_METHOD_GET, the message will be lengthier
     * because the original or encrypted message will be URL-encoded, which may
     * lead to:
     * - error because of URL-length ceiling;
     * - error because of PubNub message size limit (percent-encoding adds to
     *   the message size).
     *
     * With the @c PUBNUB_PUBLISH_METHOD_POST, the messages could reach the
     * PubNub message size limit only because of encryption, which makes it
     * lengthier.
     * If the @c PUBNUB_ENABLE_REQUEST_COMPRESSION flag is set to @c ON,
     * message transfer might be faster, but it will cost more CPU and
     * memory.
     *
     * @b Default: @c PUBNUB_PUBLISH_METHOD_GET.
     *
     * @see compress
     */
    pubnub_publish_method_t method;

    /**
     * @brief Request-body compression preference.
     *
     * @b Default: @c PUBNUB_PUBLISH_COMPRESS_DEFAULT, which follows the
     * compile-time @c PUBNUB_ENABLE_REQUEST_COMPRESSION toggle. Set
     * @c PUBNUB_PUBLISH_COMPRESS_NO to opt a single request out of
     * compression, or @c PUBNUB_PUBLISH_COMPRESS_YES to state the intent
     * explicitly.
     *
     * Only requests that carry a body can be compressed, so this field
     * takes effect only when @c method is @c PUBNUB_PUBLISH_METHOD_POST.
     * It is ignored for @c PUBNUB_PUBLISH_METHOD_GET, which encodes the
     * message into the URL path and therefore has no body to compress —
     * set @c method to @c PUBNUB_PUBLISH_METHOD_POST to compress.
     *
     * When compression applies, the body is gzip-encoded and the request
     * carries a @c Content-Encoding:gzip header.
     *
     * @note Requesting @c PUBNUB_PUBLISH_COMPRESS_YES has no effect when
     *       the SDK is built with @c PUBNUB_ENABLE_REQUEST_COMPRESSION
     *       disabled; the body is sent uncompressed. Publishing still
     *       succeeds — the request is never rejected for this reason.
     * @note Compression trades CPU and memory for a smaller request body.
     *       It pays off for large JSON payloads and costs more than it
     *       saves for small ones. On memory-constrained targets, prefer
     *       @c PUBNUB_PUBLISH_COMPRESS_NO (or build with the toggle off)
     *       unless payloads are large enough to earn back the compressor's
     *       working buffers.
     * @note Compression is best-effort. When every compression slot is
     *       already in use by concurrent requests, the body is sent
     *       uncompressed rather than failing or blocking.
     * @note @c PUBNUB_PUBLISH_COMPRESS_YES does not switch @c method to
     *       @c PUBNUB_PUBLISH_METHOD_POST for you. A GET publish stays a
     *       GET publish and is sent uncompressed.
     */
    /* 4-byte width for API symmetry with the adjacent store/method
     * tri-state enums; it lands in existing padding (see the size
     * assertion below), so a narrower type would not shrink the struct. */
    pubnub_publish_compress_t compress;

    /**
     * @brief Message persistence.
     *
     * @b Default: @c PUBNUB_PUBLISH_STORE_ACCOUNT_DEFAULT.
     */
    pubnub_publish_store_t store;

    /**
     * @brief Per-message time-to-live in hours.
     *
     * @b Default: @c 0 (account default TTL).
     *
     * @note Ignored when @c store is set to @c PUBNUB_PUBLISH_STORE_NO
     *       (the message will not be persisted).
     */
    unsigned int ttl;

    /**
     * @brief Stream-filter metadata JSON (@b optional, @b borrowed).
     *
     * Either NUL-terminated (leave @c meta_len at 0) or
     * length-counted (set @c meta_len explicitly).
     *
     * @attention Setting both @c meta and @c meta_value to non-NULL is
     *            an error (@c PUBNUB_ERR_INVALID_ARGUMENT).
     */
    const char* meta;

    /**
     * @brief Length of @c meta in bytes.
     *
     * @b Default: @c 0 means "call strlen". */
    size_t meta_len;

    /**
     * @brief Stream-filter metadata as a JSON value tree
     *        (@b optional, @b borrowed).
     *
     * Destroy the tree after @c pubnub_publish returns via
     * @c json->value_destroy() or @c pubnub_json_destroy.
     *
     * @attention Setting both @c meta and @c meta_value to non-NULL is
     *            an error (@c PUBNUB_ERR_INVALID_ARGUMENT).
     *
     * @see @c message_value documentation for examples of how a JSON value tree
     *      can be created.
     */
    pubnub_json_value_t* meta_value;

    /**
     * @brief User-supplied message-type label (@b optional, @b borrowed,
     *        NUL-terminated).
     *
     * @pre 3-50 characters long.
     */
    const char* custom_message_type;

    /**
     * @brief Per-request timeout override in milliseconds (@b optional).
     *
     * @note When non-zero, takes priority over the context-level
     *       @c pubnub_config_t::transaction_timeout_ms.
     */
    uint32_t timeout_ms;
} pubnub_publish_opts_t;

/* Callers put this struct on the stack, so its growth is a per-call-site
 * stack cost. 52 bytes on ILP32, 88 on LP64 — the bound is the larger of
 * the two so a future field addition trips the build instead of silently
 * growing caller stack. */
PUBNUB_STATIC_ASSERT(sizeof(pubnub_publish_opts_t) <= 88U,
                     "pubnub_publish_opts_t grew beyond its stack budget");

/**
 * @brief Zero-initialize publish options with protocol-correct defaults.
 *
 * Every default is the zero value, so a designated initializer such as
 * @c &(pubnub_publish_opts_t){ .channel = "ch", .message = "1" } is
 * equivalent. In particular @c compress defaults to
 * @c PUBNUB_PUBLISH_COMPRESS_DEFAULT, which follows the compile-time
 * @c PUBNUB_ENABLE_REQUEST_COMPRESSION toggle.
 */
#define PUBNUB_PUBLISH_OPTS_INIT {0}

/**
 * @brief Submit a publish request.
 *
 * Drive the returned future via cooperative polling (@c pubnub_process +
 * @c pubnub_future_is_ready), blocking await (@c pubnub_await), or
 * async callback (@c pubnub_async).
 *
 * String-form publish (cooperative polling)
 * @code
 * pubnub_future_t fut = pubnub_publish(ctx, &(pubnub_publish_opts_t){
 *     .channel = "my-channel",
 *     .message = "\"hello world\"",
 *     .store   = PUBNUB_PUBLISH_STORE_YES,
 *     .ttl     = 12,
 * });
 *
 * while (!pubnub_future_is_ready(fut)) {
 *     pubnub_process(ctx);
 * }
 *
 * if (PUBNUB_OK == pubnub_future_status(fut)) {
 *     pubnub_timetoken_t tt = pubnub_publish_result_timetoken(fut);
 *     printf("published at %.*s\n", (int)tt.len, tt.ptr);
 * } else {
 *     pubnub_string_view_t err = pubnub_response_error_message(fut);
 *     printf("error: %.*s\n", (int)err.len, err.ptr);
 * }
 * pubnub_future_release(fut);
 * @endcode
 *
 * Value-tree publish using macros (cooperative polling)
 * @code
 * pubnub_serialization_provider_t* json = pubnub_serialization(ctx);
 * pubnub_json_value_t* msg = PUBNUB_JSON_OBJ(json,
 *     PUBNUB_JSON_KV_STR(json, "device", "sensor-1"),
 *     PUBNUB_JSON_KV_INT(json, "temp_c", 42),
 *     PUBNUB_JSON_KV_BOOL(json, "alarm", 0));
 *
 * pubnub_future_t fut = pubnub_publish(ctx, &(pubnub_publish_opts_t){
 *     .channel       = "sensors",
 *     .message_value = msg,
 * });
 * pubnub_json_destroy(json, msg); // Tree is borrowed; destroy after call.
 *
 * while (!pubnub_future_is_ready(fut)) {
 *     pubnub_process(ctx);
 * }
 *
 * if (PUBNUB_OK == pubnub_future_status(fut)) {
 *     pubnub_timetoken_t tt = pubnub_publish_result_timetoken(fut);
 *     printf("published at %.*s\n", (int)tt.len, tt.ptr);
 * }
 * pubnub_future_release(fut);
 * @endcode
 *
 * On validation failure (@c NULL arguments, missing required context keys,
 * queue full) the returned future carries an immediate error code readable
 * via @c pubnub_future_status.
 *
 * @note Required context configuration: @c publish_key, @c subscribe_key,
 *       and @c user_id must all be set in @c pubnub_config_t.
 *
 * @param ctx   Initialized context (@b borrowed).
 * @param opts  Publish options struct (@b borrowed).
 * @return Future handle (owned by caller). Release via
 *         @c pubnub_future_release after reading results.
 *
 * @see pubnub_publish_result_timetoken
 * @see pubnub_future_release
 * @see pubnub_publish_opts_t
 * @see pubnub_context_t
 * @see pubnub_future_t
 */
PUBNUB_API pubnub_future_t pubnub_publish(pubnub_context_t*            ctx,
                                          const pubnub_publish_opts_t* opts);

/**
 * @brief Timetoken of the published message.
 *
 * The returned view is valid until @c pubnub_future_release is
 * called on the same future.
 *
 * @param future Future returned from @c pubnub_publish.
 * @return Timetoken view on success; a zero-initialised view
 *         (`{.ptr = NULL, .len = 0}`) if the future is not ready,
 *         carries an immediate error, or the server response did
 *         not parse.
 */
PUBNUB_API pubnub_timetoken_t pubnub_publish_result_timetoken(pubnub_future_t future);

#ifdef __cplusplus
// clang-format off
}
// clang-format on
#endif

#endif /* PUBNUB_ENABLE_PUBLISH */

#endif /* PUBNUB_FEATURE_PUBLISH_H */
