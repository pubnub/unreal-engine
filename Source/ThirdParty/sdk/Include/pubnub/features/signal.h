/* Copyright (c) PubNub Inc. */
/* See LICENSE in the root directory of this source tree. */

#ifndef PUBNUB_FEATURE_SIGNAL_H
#define PUBNUB_FEATURE_SIGNAL_H

#include "pubnub/config.h"

#if PUBNUB_ENABLE_SIGNAL

#include "pubnub/error.h"
#include "pubnub/future.h"
#include "pubnub/providers/serialization.h"
#include "pubnub/types.h"

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
// clang-format off
extern "C" {
// clang-format on
#endif

/**
 * @brief Options for @c pubnub_signal.
 *
 * Initialize with @c PUBNUB_SIGNAL_OPTS_INIT before overriding
 * individual fields.
 *
 * @see pubnub_signal
 */
typedef struct pubnub_signal_opts {
    /**
     * @brief Target channel name (@b required, @b borrowed,
     *        NUL-terminated).
     */
    const char* channel;

    /**
     * @brief Valid JSON string for signal payload (@b required,
     *        @b borrowed).
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
     * @brief Signal payload as a JSON value tree (@b required,
     *        @b borrowed).
     *
     * Build the tree with helper macros from @c json_macros.h:
     * @code
     * pubnub_serialization_provider_t* json = pubnub_serialization(ctx);
     * pubnub_json_value_t* msg =
     *     PUBNUB_JSON_OBJ(json,
     *                     PUBNUB_JSON_KV_STR(json, "type", "typing"),
     *                     PUBNUB_JSON_KV_INT(json, "seq", 1));
     * @endcode
     *
     * Destroy the tree after @c pubnub_signal returns via
     * @c pubnub_json_destroy. The SDK serializes the tree during
     * the @c pubnub_signal call; the caller retains ownership.
     *
     * @attention Setting both @c message and @c message_value to non-NULL is an
     *            error (@c PUBNUB_ERR_INVALID_ARGUMENT).
     */
    pubnub_json_value_t* message_value;

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
} pubnub_signal_opts_t;

/** @brief Zero-initializer producing valid defaults for all fields. */
#define PUBNUB_SIGNAL_OPTS_INIT {0}

/**
 * @brief Submit a signal request.
 *
 * Signals are lightweight messages with a server-enforced size limit
 * (currently 64 bytes). They are always sent via HTTP GET. Signals
 * are not persisted in Message Persistence.
 *
 * Drive the returned future via cooperative polling (@c pubnub_process +
 * @c pubnub_future_is_ready), blocking await (@c pubnub_await), or
 * async callback (@c pubnub_async).
 *
 * Cooperative polling example
 * @code
 * pubnub_signal_opts_t opts = PUBNUB_SIGNAL_OPTS_INIT;
 * opts.channel = "typing-indicator";
 * opts.message = "{\"typing\":true}";
 *
 * pubnub_future_t fut = pubnub_signal(ctx, &opts);
 *
 * while (!pubnub_future_is_ready(fut)) {
 *     pubnub_process(ctx);
 * }
 *
 * if (PUBNUB_OK == pubnub_future_status(fut)) {
 *     pubnub_timetoken_t tt = pubnub_signal_result_timetoken(fut);
 *     printf("signal sent at %.*s\n", (int)tt.len, tt.ptr);
 * } else {
 *     pubnub_string_view_t err = pubnub_response_error_message(fut);
 *     printf("error: %.*s\n", (int)err.len, err.ptr);
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
 * @param opts  Signal options struct (@b borrowed).
 * @return Future handle (owned by caller). Release via
 *         @c pubnub_future_release after reading results.
 *
 * @see pubnub_signal_result_timetoken
 * @see pubnub_future_release
 * @see pubnub_signal_opts_t
 * @see pubnub_context_t
 * @see pubnub_future_t
 */
PUBNUB_API pubnub_future_t pubnub_signal(pubnub_context_t*           ctx,
                                         const pubnub_signal_opts_t* opts);

/**
 * @brief Timetoken of the sent signal.
 *
 * The returned view is valid until @c pubnub_future_release is
 * called on the same future.
 *
 * @param future Future returned from @c pubnub_signal.
 * @return Timetoken view on success; a zero-initialised view
 *         (`{.ptr = NULL, .len = 0}`) if the future is not ready,
 *         carries an immediate error, or the server response did
 *         not parse.
 */
PUBNUB_API pubnub_timetoken_t pubnub_signal_result_timetoken(pubnub_future_t future);

#ifdef __cplusplus
// clang-format off
}
// clang-format on
#endif

#endif /* PUBNUB_ENABLE_SIGNAL */

#endif /* PUBNUB_FEATURE_SIGNAL_H */
