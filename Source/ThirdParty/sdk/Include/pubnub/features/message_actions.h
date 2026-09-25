/* Copyright (c) PubNub Inc. */
/* See LICENSE in the root directory of this source tree. */

/**
 * @file message_actions.h
 * @brief PubNub Message Actions REST API — add, retrieve, and remove
 *        reactions/acknowledgments on published messages.
 */

#ifndef PUBNUB_MESSAGE_ACTIONS_H
#define PUBNUB_MESSAGE_ACTIONS_H

#include "pubnub/config.h"

#if PUBNUB_ENABLE_MESSAGE_ACTIONS

#include "pubnub/error.h"
#include "pubnub/future.h"
#include "pubnub/types.h"

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
// clang-format off
extern "C" {
// clang-format on
#endif

/**
 * @brief A single message action as returned by the server.
 *
 * All string views alias internal parsed data and remain valid until
 * pubnub_future_release() is called on the owning future.
 */
typedef struct pubnub_message_action {
    pubnub_string_view_t type;  /**< Action type (e.g. "reaction"). */
    pubnub_string_view_t value; /**< Action value (e.g. "smiley_face"). */
    pubnub_string_view_t uuid;  /**< UUID of the user who added it. */
    pubnub_string_view_t action_timetoken; /**< When the action was added. */
    pubnub_string_view_t message_timetoken; /**< Timetoken of the parent message. */
} pubnub_message_action_t;

/**
 * @brief Options for pubnub_add_message_action().
 *
 * Initialize with @c PUBNUB_ADD_MESSAGE_ACTION_OPTS_INIT before
 * setting individual fields.
 *
 * @code
 * pubnub_add_message_action_opts_t opts = PUBNUB_ADD_MESSAGE_ACTION_OPTS_INIT;
 * opts.channel           = "my-channel";
 * opts.message_timetoken = "15610547826969050";
 * opts.type              = "reaction";
 * opts.value             = "thumbs_up";
 * @endcode
 *
 * @see pubnub_add_message_action
 */
typedef struct pubnub_add_message_action_opts {
    /**
     * @brief Target channel (@b required, @b borrowed,
     *        NUL-terminated).
     */
    const char* channel;

    /**
     * @brief Parent message timetoken (@b required, @b borrowed,
     *        NUL-terminated).
     */
    const char* message_timetoken;

    /**
     * @brief Action type (@b required, @b borrowed, NUL-terminated).
     *
     * @pre Maximum 15 characters.
     */
    const char* type;

    /**
     * @brief Action value (@b required, @b borrowed, NUL-terminated).
     */
    const char* value;

    /**
     * @brief Per-request timeout override in milliseconds (@b optional).
     *
     * @note When non-zero, takes priority over the context-level
     *       @c pubnub_config_t::transaction_timeout_ms.
     */
    uint32_t timeout_ms;
} pubnub_add_message_action_opts_t;

/** @brief Zero-initializer for @c pubnub_add_message_action_opts_t. */
#define PUBNUB_ADD_MESSAGE_ACTION_OPTS_INIT {0}

/**
 * @brief Options for pubnub_get_message_actions().
 *
 * Initialize with @c PUBNUB_GET_MESSAGE_ACTIONS_OPTS_INIT before
 * setting individual fields.
 *
 * @code
 * pubnub_get_message_actions_opts_t opts =
 *     PUBNUB_GET_MESSAGE_ACTIONS_OPTS_INIT;
 * opts.channel = "my-channel";
 * opts.limit   = 25;
 * @endcode
 *
 * @see pubnub_get_message_actions
 */
typedef struct pubnub_get_message_actions_opts {
    /**
     * @brief Target channel (@b required, @b borrowed,
     *        NUL-terminated).
     */
    const char* channel;

    /**
     * @brief Start timetoken boundary (@b optional, @b borrowed,
     *        NUL-terminated).
     *
     * Return actions with timetokens less than this value.
     */
    const char* start;

    /**
     * @brief End timetoken boundary (@b optional, @b borrowed,
     *        NUL-terminated).
     *
     * Return actions with timetokens greater than or equal to this
     * value.
     */
    const char* end;

    /**
     * @brief Maximum results to return.
     *
     * @b Default: @c 0 (server default).
     */
    uint32_t limit;

    /**
     * @brief Per-request timeout override in milliseconds (@b optional).
     *
     * @note When non-zero, takes priority over the context-level
     *       @c pubnub_config_t::transaction_timeout_ms.
     */
    uint32_t timeout_ms;
} pubnub_get_message_actions_opts_t;

/** @brief Zero-initializer for @c pubnub_get_message_actions_opts_t. */
#define PUBNUB_GET_MESSAGE_ACTIONS_OPTS_INIT {0}

/**
 * @brief Options for pubnub_remove_message_action().
 *
 * Initialize with @c PUBNUB_REMOVE_MESSAGE_ACTION_OPTS_INIT before
 * setting individual fields.
 *
 * @code
 * pubnub_remove_message_action_opts_t opts =
 *     PUBNUB_REMOVE_MESSAGE_ACTION_OPTS_INIT;
 * opts.channel           = "my-channel";
 * opts.message_timetoken = "15610547826969050";
 * opts.action_timetoken  = "15610547826970050";
 * @endcode
 *
 * @see pubnub_remove_message_action
 */
typedef struct pubnub_remove_message_action_opts {
    /**
     * @brief Target channel (@b required, @b borrowed,
     *        NUL-terminated).
     */
    const char* channel;

    /**
     * @brief Parent message timetoken (@b required, @b borrowed,
     *        NUL-terminated).
     */
    const char* message_timetoken;

    /**
     * @brief Action timetoken to remove (@b required, @b borrowed,
     *        NUL-terminated).
     */
    const char* action_timetoken;

    /**
     * @brief Per-request timeout override in milliseconds (@b optional).
     *
     * @note When non-zero, takes priority over the context-level
     *       @c pubnub_config_t::transaction_timeout_ms.
     */
    uint32_t timeout_ms;
} pubnub_remove_message_action_opts_t;

/** @brief Zero-initializer for @c pubnub_remove_message_action_opts_t. */
#define PUBNUB_REMOVE_MESSAGE_ACTION_OPTS_INIT {0}

/**
 * @brief Result from pubnub_add_message_action().
 *
 * Contains the added action with server-assigned timetokens. Fields
 * remain valid until pubnub_future_release() is called.
 */
typedef struct pubnub_add_message_action_result {
    /** The added action with server-assigned timetokens. */
    pubnub_message_action_t action;
} pubnub_add_message_action_result_t;

/**
 * @brief Aggregate result from pubnub_get_message_actions().
 *
 * Use @c count as the loop bound for the indexed accessor
 * pubnub_get_message_actions_result_action_at(). Pagination fields
 * indicate whether more results are available.
 */
typedef struct pubnub_get_message_actions_result {
    /** Number of actions (iteration bound for indexed accessor). */
    uint32_t count;
    /** Non-zero when the server indicates more pages exist. */
    uint8_t has_more;
    /** Pagination cursor: pass as @c start for the next page. */
    pubnub_string_view_t more_start;
    /** Pagination cursor: pass as @c end for the next page. */
    pubnub_string_view_t more_end;
    /** Server-recommended limit for the next page. */
    uint32_t more_limit;
} pubnub_get_message_actions_result_t;

/**
 * @brief Add a message action (reaction/receipt) to a published message.
 *
 * Sends a POST request to the message actions endpoint. On success,
 * the response contains the action with server-assigned timetokens.
 *
 * @code
 * pubnub_add_message_action_opts_t opts = PUBNUB_ADD_MESSAGE_ACTION_OPTS_INIT;
 * opts.channel           = "chat-room";
 * opts.message_timetoken = "15610547826969050";
 * opts.type              = "reaction";
 * opts.value             = "thumbs_up";
 *
 * pubnub_future_t fut = pubnub_add_message_action(ctx, &opts);
 * while (!pubnub_future_is_ready(fut)) { pubnub_process(ctx); }
 *
 * if (PUBNUB_OK == pubnub_future_status(fut)) {
 *     pubnub_add_message_action_result_t r =
 *         pubnub_add_message_action_result(fut);
 *     printf("Added at: %.*s\n",
 *            (int)r.action.action_timetoken.len,
 *            r.action.action_timetoken.ptr);
 * }
 * pubnub_future_release(fut);
 * @endcode
 *
 * @param ctx  Initialized context (@b required, @b borrowed).
 * @param opts Operation options (@b required, @b borrowed). All required
 *             fields must be set; type must be <= 15 characters.
 * @return Future handle (owned by caller). Release via @c pubnub_future_release
 *         after reading results.
 *         Returns PUBNUB_FUTURE_INVALID on validation failure.
 *
 * @see pubnub_add_message_action_result
 * @see pubnub_future_release
 * @see pubnub_add_message_action_opts_t
 * @see pubnub_context_t
 * @see pubnub_future_t
 */
PUBNUB_API pubnub_future_t
pubnub_add_message_action(pubnub_context_t*                       ctx,
                          const pubnub_add_message_action_opts_t* opts);

/**
 * @brief Retrieve message actions for a channel, with pagination.
 *
 * Sends a GET request. Results are returned in reverse-chronological
 * order. When the server has more pages, the result struct's
 * @c has_more flag is set and pagination cursors are populated.
 *
 * @code
 * pubnub_get_message_actions_opts_t opts =
 *     PUBNUB_GET_MESSAGE_ACTIONS_OPTS_INIT;
 * opts.channel = "chat-room";
 * opts.limit   = 10;
 *
 * pubnub_future_t fut = pubnub_get_message_actions(ctx, &opts);
 * while (!pubnub_future_is_ready(fut)) { pubnub_process(ctx); }
 *
 * if (PUBNUB_OK == pubnub_future_status(fut)) {
 *     pubnub_get_message_actions_result_t r =
 *         pubnub_get_message_actions_result(fut);
 *     for (uint32_t i = 0; i < r.count; ++i) {
 *         pubnub_message_action_t a =
 *             pubnub_get_message_actions_result_action_at(fut, i);
 *         printf("%.*s: %.*s\n",
 *                (int)a.type.len, a.type.ptr,
 *                (int)a.value.len, a.value.ptr);
 *     }
 * }
 * pubnub_future_release(fut);
 * @endcode
 *
 * @param ctx  Initialized context (@b required, @b borrowed).
 * @param opts Operation options (@b required, @b borrowed). Channel is
 *             required.
 * @return Future handle (owned by caller). Release via @c pubnub_future_release
 *         after reading results.
 *         Returns PUBNUB_FUTURE_INVALID on validation failure.
 *
 * @see pubnub_get_message_actions_result
 * @see pubnub_future_release
 * @see pubnub_get_message_actions_opts_t
 * @see pubnub_context_t
 * @see pubnub_future_t
 */
PUBNUB_API pubnub_future_t
pubnub_get_message_actions(pubnub_context_t*                        ctx,
                           const pubnub_get_message_actions_opts_t* opts);

/**
 * @brief Remove a previously-added message action.
 *
 * Sends a DELETE request. Only the user who originally added the
 * action may remove it (enforced server-side). On success the
 * response body is empty; check status via pubnub_future_status().
 *
 * @code
 * pubnub_remove_message_action_opts_t opts =
 *     PUBNUB_REMOVE_MESSAGE_ACTION_OPTS_INIT;
 * opts.channel           = "chat-room";
 * opts.message_timetoken = "15610547826969050";
 * opts.action_timetoken  = "15610547826970050";
 *
 * pubnub_future_t fut = pubnub_remove_message_action(ctx, &opts);
 * while (!pubnub_future_is_ready(fut)) { pubnub_process(ctx); }
 *
 * if (PUBNUB_OK == pubnub_future_status(fut)) {
 *     printf("Action removed successfully.\n");
 * }
 * pubnub_future_release(fut);
 * @endcode
 *
 * @param ctx  Initialized context (@b required, @b borrowed).
 * @param opts Operation options (@b required, @b borrowed). All three
 *             identifier fields are required.
 * @return Future handle (owned by caller). Release via @c pubnub_future_release
 *         after reading results.
 *         Returns PUBNUB_FUTURE_INVALID on validation failure.
 *
 * @see pubnub_future_release
 * @see pubnub_remove_message_action_opts_t
 * @see pubnub_context_t
 * @see pubnub_future_t
 */
PUBNUB_API pubnub_future_t
pubnub_remove_message_action(pubnub_context_t*                          ctx,
                             const pubnub_remove_message_action_opts_t* opts);

/**
 * @brief Extract the result of pubnub_add_message_action().
 *
 * Lazily parses the response on first call and caches the result.
 * Returned data aliases internal storage valid until
 * pubnub_future_release() is called.
 *
 * @param future Completed add-message-action future (@b required).
 * @return Result struct; zero-initialized if the future is invalid,
 *         not ready, or parsing fails.
 */
PUBNUB_API pubnub_add_message_action_result_t
pubnub_add_message_action_result(pubnub_future_t future);

/**
 * @brief Extract the aggregate result of pubnub_get_message_actions().
 *
 * Lazily parses the response on first call and caches the result.
 * Use @c count as the loop bound for the indexed accessor.
 *
 * @param future Completed get-message-actions future (@b required).
 * @return Result struct; zero-initialized if the future is invalid,
 *         not ready, or parsing fails.
 */
PUBNUB_API pubnub_get_message_actions_result_t
pubnub_get_message_actions_result(pubnub_future_t future);

/**
 * @brief Retrieve a single action from a get-message-actions result.
 *
 * @param future Completed get-message-actions future (@b required).
 * @param index  Zero-based index (must be less than result.count).
 * @return Action struct; zero-initialized if out of range or invalid.
 */
PUBNUB_API pubnub_message_action_t
pubnub_get_message_actions_result_action_at(pubnub_future_t future, size_t index);

#ifdef __cplusplus
// clang-format off
}
// clang-format on
#endif

#endif /* PUBNUB_ENABLE_MESSAGE_ACTIONS */

#endif /* PUBNUB_MESSAGE_ACTIONS_H */
