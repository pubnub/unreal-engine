/* Copyright (c) PubNub Inc. */
/* See LICENSE in the root directory of this source tree. */

#ifndef PUBNUB_FEATURE_PRESENCE_H
#define PUBNUB_FEATURE_PRESENCE_H

#include "pubnub/config.h"

#if PUBNUB_ENABLE_PRESENCE

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
 * @brief Options for @c pubnub_here_now.
 *
 * Initialize with @c PUBNUB_HERE_NOW_OPTS_INIT before overriding
 * individual fields.
 *
 * @see pubnub_here_now
 */
typedef struct pubnub_here_now_opts {
    /**
     * @brief Comma-separated channel names to query (@b borrowed,
     *        NUL-terminated).
     *
     * At least one of @c channels or @c channel_groups must be non-NULL
     * and non-empty.
     */
    const char* channels;

    /**
     * @brief Comma-separated channel-group names (@b optional,
     *        @b borrowed, NUL-terminated).
     *
     * At least one of @c channels or @c channel_groups must be non-NULL
     * and non-empty. Pass @c NULL when targeting channels directly.
     */
    const char* channel_groups;

    /**
     * @brief Include UUIDs in the response.
     *
     * @b Default: @c 1 (include).
     */
    uint8_t include_uuids;

    /**
     * @brief Include per-user state objects in the response.
     *
     * @b Default: @c 0 (exclude).
     */
    uint8_t include_state;

    /**
     * @brief Maximum number of occupants to return per channel. @c 0 means
     * use the server default (100). Clamped server-side to 1000.
     *
     * @note Setting @c limit > 0 enables occupant-level pagination alongside
     * @c offset.
     */
    uint32_t limit;

    /**
     * @brief Zero-based index of the first occupant to return. @c 0 returns
     * from the beginning of the occupant list for the channel.
     */
    uint32_t offset;

    /**
     * @brief Per-request timeout override in milliseconds (@b optional).
     *
     * @note When non-zero, takes priority over the context-level
     *       @c pubnub_config_t::transaction_timeout_ms.
     */
    uint32_t timeout_ms;
} pubnub_here_now_opts_t;

/** @brief Zero-initialize here-now options with include_uuids enabled. */
#define PUBNUB_HERE_NOW_OPTS_INIT {.include_uuids = 1}

PUBNUB_STATIC_ASSERT(
    sizeof(pubnub_here_now_opts_t) <= 2U * sizeof(const char*) + 16U,
    "pubnub_here_now_opts_t exceeds expected size; check for new fields");

/**
 * @brief Options for @c pubnub_where_now.
 *
 * Initialize with @c PUBNUB_WHERE_NOW_OPTS_INIT before overriding
 * individual fields.
 *
 * @see pubnub_where_now
 */
typedef struct pubnub_where_now_opts {
    /**
     * @brief UUID to look up (@b optional, @b borrowed,
     *        NUL-terminated).
     *
     * Pass @c NULL to use the context's own @c user_id from config.
     */
    const char* uuid;

    /**
     * @brief Per-request timeout override in milliseconds (@b optional).
     *
     * @note When non-zero, takes priority over the context-level
     *       @c pubnub_config_t::transaction_timeout_ms.
     */
    uint32_t timeout_ms;
} pubnub_where_now_opts_t;

/** @brief Zero-initialize where-now options (uses own user_id). */
#define PUBNUB_WHERE_NOW_OPTS_INIT {0}

/**
 * @brief Options for @c pubnub_set_state.
 *
 * Initialize with @c PUBNUB_SET_STATE_OPTS_INIT before overriding
 * individual fields.
 *
 * @see pubnub_set_state
 */
typedef struct pubnub_set_state_opts {
    /**
     * @brief Comma-separated channel names (@b borrowed,
     *        NUL-terminated).
     *
     * At least one of @c channels or @c channel_groups is required.
     * Pass @c NULL when targeting only channel groups.
     */
    const char* channels;

    /**
     * @brief Comma-separated channel-group names (@b optional,
     *        @b borrowed, NUL-terminated).
     *
     * Pass @c NULL when setting state on channels directly.
     */
    const char* channel_groups;

    /**
     * @brief JSON state object as a raw string (@b borrowed).
     *
     * Must be a valid JSON object (e.g., @c "{\"mood\":\"happy\"}").
     * NUL-terminated; set @c state_len to 0 for automatic strlen.
     *
     * @attention Setting both @c state and @c state_value to non-NULL
     *            is an error (@c PUBNUB_ERR_INVALID_ARGUMENT).
     *            Setting neither (both NULL/zero) also returns
     *            @c PUBNUB_ERR_INVALID_ARGUMENT — exactly one must be
     *            provided.
     */
    const char* state;

    /**
     * @brief Length of @c state in bytes.
     *
     * @b Default: @c 0 (means "call strlen").
     */
    size_t state_len;

    /**
     * @brief State payload as a JSON value tree (@b borrowed).
     *
     * Build the tree with helper macros from @c json_macros.h:
     * @code
     * pubnub_serialization_provider_t* json = pubnub_serialization(ctx);
     * pubnub_json_value_t* st =
     *     PUBNUB_JSON_OBJ(json,
     *                     PUBNUB_JSON_KV_STR(json, "mood", "happy"),
     *                     PUBNUB_JSON_KV_INT(json, "score", 42));
     * @endcode
     *
     * The SDK serializes the tree during the @c pubnub_set_state call;
     * the caller retains ownership and must destroy via
     * @c pubnub_json_destroy after the call returns.
     *
     * @attention Setting both @c state and @c state_value to non-NULL
     *            is an error (@c PUBNUB_ERR_INVALID_ARGUMENT).
     *            Setting neither (both NULL/zero) also returns
     *            @c PUBNUB_ERR_INVALID_ARGUMENT — exactly one must be
     *            provided.
     */
    pubnub_json_value_t* state_value;

    /**
     * @brief Per-request timeout override in milliseconds (@b optional).
     *
     * @note When non-zero, takes priority over the context-level
     *       @c pubnub_config_t::transaction_timeout_ms.
     */
    uint32_t timeout_ms;
} pubnub_set_state_opts_t;

/** @brief Zero-initialize set-state options. */
#define PUBNUB_SET_STATE_OPTS_INIT {0}

/**
 * @brief Options for @c pubnub_get_state.
 *
 * Initialize with @c PUBNUB_GET_STATE_OPTS_INIT before overriding
 * individual fields.
 *
 * @see pubnub_get_state
 */
typedef struct pubnub_get_state_opts {
    /**
     * @brief Comma-separated channel names (@b borrowed,
     *        NUL-terminated).
     *
     * At least one of @c channels or @c channel_groups is required.
     * Pass @c NULL when targeting only channel groups.
     */
    const char* channels;

    /**
     * @brief Comma-separated channel-group names (@b optional,
     *        @b borrowed, NUL-terminated).
     *
     * Pass @c NULL when querying channels directly.
     */
    const char* channel_groups;

    /**
     * @brief UUID to query state for (@b optional, @b borrowed,
     *        NUL-terminated).
     *
     * Pass @c NULL to use the context's own @c user_id from config.
     */
    const char* uuid;

    /**
     * @brief Per-request timeout override in milliseconds (@b optional).
     *
     * @note When non-zero, takes priority over the context-level
     *       @c pubnub_config_t::transaction_timeout_ms.
     */
    uint32_t timeout_ms;
} pubnub_get_state_opts_t;

/** @brief Zero-initialize get-state options (uses own user_id). */
#define PUBNUB_GET_STATE_OPTS_INIT {0}

/**
 * @brief Aggregate here-now result: totals across all queried channels.
 */
typedef struct pubnub_here_now_result {
    /** Total occupancy across all channels. */
    uint32_t total_occupancy;
    /** Total number of channels reported by the server. */
    uint32_t total_channels;
    /** Number of channels with detailed data (iteration bound). */
    uint32_t channel_count;
} pubnub_here_now_result_t;

/**
 * @brief Per-channel here-now result: name, occupancy, and occupant count.
 */
typedef struct pubnub_here_now_channel_result {
    /** Channel name (valid until @c pubnub_future_release). */
    pubnub_string_view_t name;
    /** Server-reported occupancy for this channel. */
    uint32_t occupancy;
    /** Number of occupant entries available for iteration. */
    uint32_t occupant_count;
} pubnub_here_now_channel_result_t;

/**
 * @brief Per-occupant here-now result: UUID and optional state.
 */
typedef struct pubnub_here_now_occupant_result {
    /** Occupant UUID (valid until @c pubnub_future_release). */
    pubnub_string_view_t uuid;
    /** Raw JSON state, or @c {NULL,0} when absent
     *  (valid until @c pubnub_future_release). */
    pubnub_string_view_t state;
} pubnub_here_now_occupant_result_t;

/**
 * @brief Query channel occupancy and subscriber details.
 *
 * Returns occupancy information for the specified channels and/or
 * channel groups. At least one of @c channels or @c channel_groups
 * must be non-NULL; passing @c NULL for both returns
 * @c PUBNUB_ERR_INVALID_ARGUMENT.
 *
 * Drive the returned future via cooperative polling (@c pubnub_process
 * + @c pubnub_future_is_ready), blocking await (@c pubnub_await),
 * or async callback (@c pubnub_async).
 *
 * Example (cooperative polling)
 * @code
 * pubnub_future_t fut = pubnub_here_now(ctx, &(pubnub_here_now_opts_t){
 *     .channels      = "lobby,game-1",
 *     .include_uuids = 1,
 *     .include_state = 1,
 * });
 *
 * while (!pubnub_future_is_ready(fut)) {
 *     pubnub_process(ctx);
 * }
 *
 * if (PUBNUB_OK == pubnub_future_status(fut)) {
 *     pubnub_here_now_result_t r = pubnub_here_now_result(fut);
 *     printf("Total occupancy: %u\n", r.total_occupancy);
 *     for (size_t i = 0; i < r.channel_count; ++i) {
 *         pubnub_here_now_channel_result_t ch =
 *             pubnub_here_now_result_channel_at(fut, i);
 *         printf("  %.*s: %u occupants\n",
 *                (int)ch.name.len, ch.name.ptr, ch.occupancy);
 *         for (size_t j = 0; j < ch.occupant_count; ++j) {
 *             pubnub_here_now_occupant_result_t occ =
 *                 pubnub_here_now_result_occupant_at(fut, i, j);
 *             printf("    uuid: %.*s\n",
 *                    (int)occ.uuid.len, occ.uuid.ptr);
 *         }
 *     }
 * }
 * pubnub_future_release(fut);
 * @endcode
 *
 * @param ctx   Initialized context (@b borrowed).
 * @param opts  Here-now options struct (@b borrowed).
 * @return Future handle (owned by caller). Release via
 *         @c pubnub_future_release after reading results.
 *
 * @see pubnub_here_now_result
 * @see pubnub_future_release
 * @see pubnub_here_now_opts_t
 * @see pubnub_context_t
 * @see pubnub_future_t
 */
PUBNUB_API pubnub_future_t pubnub_here_now(pubnub_context_t*             ctx,
                                           const pubnub_here_now_opts_t* opts);

/**
 * @brief Query which channels a UUID is subscribed to.
 *
 * Example (cooperative polling)
 * @code
 * pubnub_future_t fut = pubnub_where_now(ctx, &(pubnub_where_now_opts_t){
 *     .uuid = "user-42",
 * });
 *
 * while (!pubnub_future_is_ready(fut)) {
 *     pubnub_process(ctx);
 * }
 *
 * if (PUBNUB_OK == pubnub_future_status(fut)) {
 *     pubnub_where_now_result_t r = pubnub_where_now_result(fut);
 *     for (size_t i = 0; i < r.channel_count; ++i) {
 *         pubnub_string_view_t ch =
 *             pubnub_where_now_result_channel_at(fut, i);
 *         printf("  %.*s\n", (int)ch.len, ch.ptr);
 *     }
 * }
 * pubnub_future_release(fut);
 * @endcode
 *
 * @param ctx   Initialized context (@b borrowed).
 * @param opts  Where-now options struct (@b borrowed).
 * @return Future handle (owned by caller). Release via
 *         @c pubnub_future_release after reading results.
 *
 * @see pubnub_where_now_result
 * @see pubnub_future_release
 * @see pubnub_where_now_opts_t
 * @see pubnub_context_t
 * @see pubnub_future_t
 */
PUBNUB_API pubnub_future_t pubnub_where_now(pubnub_context_t* ctx,
                                            const pubnub_where_now_opts_t* opts);

/**
 * @brief Set presence state for the current user on channels.
 *
 * The state is a JSON object associated with the UUID on the
 * specified channels. Other subscribers receive the state via
 * presence events and here-now queries.
 *
 * Example (cooperative polling)
 * @code
 * pubnub_future_t fut = pubnub_set_state(ctx, &(pubnub_set_state_opts_t){
 *     .channels = "lobby",
 *     .state    = "{\"mood\":\"happy\"}",
 * });
 *
 * while (!pubnub_future_is_ready(fut)) {
 *     pubnub_process(ctx);
 * }
 *
 * if (PUBNUB_OK == pubnub_future_status(fut)) {
 *     pubnub_set_state_result_t r = pubnub_set_state_result(fut);
 *     pubnub_serialization_provider_t* serial = pubnub_serialization(ctx);
 *     if (NULL != r.state) {
 *         char buf[128];
 *         pubnub_json_to_debug_string(serial, r.state, buf, sizeof(buf));
 *         printf("state set: %s\n", buf);
 *     }
 * }
 * pubnub_future_release(fut);
 * @endcode
 *
 * @param ctx   Initialized context (@b borrowed).
 * @param opts  Set-state options struct (@b borrowed).
 * @return Future handle (owned by caller). Release via
 *         @c pubnub_future_release after reading results.
 *
 * @see pubnub_set_state_result
 * @see pubnub_future_release
 * @see pubnub_set_state_opts_t
 * @see pubnub_context_t
 * @see pubnub_future_t
 */
PUBNUB_API pubnub_future_t pubnub_set_state(pubnub_context_t* ctx,
                                            const pubnub_set_state_opts_t* opts);

/**
 * @brief Retrieve presence state for a UUID on channels.
 *
 * Example (cooperative polling)
 * @code
 * pubnub_future_t fut = pubnub_get_state(ctx, &(pubnub_get_state_opts_t){
 *     .channels = "lobby,game-1",
 *     .uuid     = "user-42",
 * });
 *
 * while (!pubnub_future_is_ready(fut)) {
 *     pubnub_process(ctx);
 * }
 *
 * if (PUBNUB_OK == pubnub_future_status(fut)) {
 *     pubnub_get_state_result_t r = pubnub_get_state_result(fut);
 *     pubnub_serialization_provider_t* serial = pubnub_serialization(ctx);
 *     for (size_t i = 0; i < r.channel_count; ++i) {
 *         pubnub_get_state_channel_result_t entry =
 *             pubnub_get_state_result_channel_at(fut, i);
 *         printf("%.*s => ", (int)entry.channel.len, entry.channel.ptr);
 *         if (NULL != entry.state) {
 *             char buf[128];
 *             pubnub_json_to_debug_string(serial, entry.state, buf,
 * sizeof(buf)); printf("%s", buf);
 *         }
 *         printf("\n");
 *     }
 * }
 * pubnub_future_release(fut);
 * @endcode
 *
 * @param ctx   Initialized context (@b borrowed).
 * @param opts  Get-state options struct (@b borrowed).
 * @return Future handle (owned by caller). Release via
 *         @c pubnub_future_release after reading results.
 *
 * @see pubnub_get_state_result
 * @see pubnub_future_release
 * @see pubnub_get_state_opts_t
 * @see pubnub_context_t
 * @see pubnub_future_t
 */
PUBNUB_API pubnub_future_t pubnub_get_state(pubnub_context_t* ctx,
                                            const pubnub_get_state_opts_t* opts);

/**
 * @brief Retrieve aggregate here-now totals from a completed future.
 *
 * All fields are zero-initialized when the future is invalid, not
 * ready, or carries an error.
 *
 * @param future Future returned from @c pubnub_here_now.
 * @return Aggregate result struct (by value).
 */
PUBNUB_API pubnub_here_now_result_t pubnub_here_now_result(pubnub_future_t future);

/**
 * @brief Retrieve per-channel here-now data at @p index.
 *
 * String views in the returned struct are valid until
 * @c pubnub_future_release is called on the future.
 *
 * @param future Future returned from @c pubnub_here_now.
 * @param index  Channel index in @c [0, result.channel_count).
 * @return Channel result struct; zero-initialized when out of range
 *         or the future is not ready.
 */
PUBNUB_API pubnub_here_now_channel_result_t
pubnub_here_now_result_channel_at(pubnub_future_t future, size_t index);

/**
 * @brief Retrieve a single occupant entry within a channel.
 *
 * String views in the returned struct are valid until
 * @c pubnub_future_release is called on the future.
 *
 * @param future    Future returned from @c pubnub_here_now.
 * @param ch_index  Channel index in @c [0, result.channel_count).
 * @param occ_index Occupant index in @c [0, channel.occupant_count).
 * @return Occupant result struct; zero-initialized when indices are
 *         out of range or the future is not ready.
 */
PUBNUB_API pubnub_here_now_occupant_result_t
pubnub_here_now_result_occupant_at(pubnub_future_t future,
                                   size_t          ch_index,
                                   size_t          occ_index);

/**
 * @brief Aggregate where-now result.
 */
typedef struct pubnub_where_now_result {
    /** Number of channels the UUID is present on. */
    uint32_t channel_count;
} pubnub_where_now_result_t;

/**
 * @brief Aggregate set-state result: server-confirmed state.
 *
 * The @c state pointer is valid until @c pubnub_future_release.
 */
typedef struct pubnub_set_state_result {
    /**
     * @brief Server-echoed state object.
     *
     * Walk with @c serial->object_get() to read fields. @c NULL when the
     * server did not echo state. Valid until
     * @c pubnub_future_release.
     */
    const pubnub_json_value_t* state;
} pubnub_set_state_result_t;

/**
 * @brief Aggregate get-state result.
 */
typedef struct pubnub_get_state_result {
    /** Number of channels with state entries. */
    uint32_t channel_count;
} pubnub_get_state_result_t;

/**
 * @brief Per-channel get-state entry: channel name and state object.
 *
 * Views and pointers are valid until @c pubnub_future_release.
 */
typedef struct pubnub_get_state_channel_result {
    /** Channel name. */
    pubnub_string_view_t channel;
    /**
     * @brief User-set presence state object.
     *
     * Walk with @c serial->object_get() to read fields. @c NULL when no
     * state is set for this channel. Valid until
     * @c pubnub_future_release.
     */
    const pubnub_json_value_t* state;
} pubnub_get_state_channel_result_t;

/**
 * @brief Retrieve aggregate where-now totals from a completed future.
 *
 * All fields are zero-initialized when the future is invalid, not
 * ready, or carries an error.
 *
 * @param future Future returned from @c pubnub_where_now.
 * @return Result struct (by value).
 */
PUBNUB_API pubnub_where_now_result_t pubnub_where_now_result(pubnub_future_t future);

/**
 * @brief Channel name at a given index from where-now results.
 *
 * The returned view is valid until @c pubnub_future_release.
 *
 * @param future Future returned from @c pubnub_where_now.
 * @param index  Channel index in @c [0, result.channel_count).
 * @return Channel name view, or zero-initialized view if index is
 *         out of range or the future is not ready.
 */
PUBNUB_API pubnub_string_view_t pubnub_where_now_result_channel_at(pubnub_future_t future,
                                                                   size_t index);

/**
 * @brief Retrieve confirmed state from a completed set-state future.
 *
 * All fields are zero-initialized when the future is invalid, not
 * ready, or carries an error. The @c state view is valid until
 * @c pubnub_future_release.
 *
 * @param future Future returned from @c pubnub_set_state.
 * @return Result struct (by value).
 */
PUBNUB_API pubnub_set_state_result_t pubnub_set_state_result(pubnub_future_t future);

/**
 * @brief Retrieve aggregate get-state totals from a completed future.
 *
 * All fields are zero-initialized when the future is invalid, not
 * ready, or carries an error.
 *
 * @param future Future returned from @c pubnub_get_state.
 * @return Result struct (by value).
 */
PUBNUB_API pubnub_get_state_result_t pubnub_get_state_result(pubnub_future_t future);

/**
 * @brief Per-channel entry at a given index from get-state results.
 *
 * Both views in the returned struct are valid until
 * @c pubnub_future_release.
 *
 * @param future Future returned from @c pubnub_get_state.
 * @param index  Channel index in @c [0, result.channel_count).
 * @return Channel result struct; zero-initialized when out of range
 *         or the future is not ready.
 */
PUBNUB_API pubnub_get_state_channel_result_t
pubnub_get_state_result_channel_at(pubnub_future_t future, size_t index);

#ifdef __cplusplus
// clang-format off
}
// clang-format on
#endif

#endif /* PUBNUB_ENABLE_PRESENCE */

#endif /* PUBNUB_FEATURE_PRESENCE_H */
