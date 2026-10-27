/* Copyright (c) PubNub Inc. */
/* See LICENSE in the root directory of this source tree. */

#ifndef PUBNUB_FEATURE_CHANNEL_GROUPS_H
#define PUBNUB_FEATURE_CHANNEL_GROUPS_H

#include "pubnub/config.h"

#if PUBNUB_ENABLE_CHANNEL_GROUPS

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
 * @brief Options for @c pubnub_channel_group_add_channels.
 *
 * Initialize with @c PUBNUB_CHANNEL_GROUP_ADD_OPTS_INIT before overriding
 * individual fields.
 *
 * @see pubnub_channel_group_add_channels
 */
typedef struct pubnub_channel_group_add_opts {
    /**
     * @brief Target channel group name (@b required, @b borrowed,
     *        NUL-terminated).
     */
    const char* channel_group;

    /**
     * @brief Comma-separated channel names to add (@b required,
     *        @b borrowed, NUL-terminated).
     *
     * @code
     * opts.channels = "ch1,ch2,ch3";
     * @endcode
     */
    const char* channels;

    /**
     * @brief Per-request timeout override in milliseconds (@b optional).
     *
     * @note When non-zero, takes priority over the context-level
     *       @c pubnub_config_t::transaction_timeout_ms.
     */
    uint32_t timeout_ms;
} pubnub_channel_group_add_opts_t;

/** @brief Zero-initialize add-channels options with protocol defaults. */
#define PUBNUB_CHANNEL_GROUP_ADD_OPTS_INIT {0}

/**
 * @brief Options for @c pubnub_channel_group_remove_channels.
 *
 * Initialize with @c PUBNUB_CHANNEL_GROUP_REMOVE_OPTS_INIT before overriding
 * individual fields.
 *
 * @see pubnub_channel_group_remove_channels
 */
typedef struct pubnub_channel_group_remove_opts {
    /**
     * @brief Target channel group name (@b required, @b borrowed,
     *        NUL-terminated).
     */
    const char* channel_group;

    /**
     * @brief Comma-separated channel names to remove (@b required,
     *        @b borrowed, NUL-terminated).
     *
     * @code
     * opts.channels = "ch1,ch2";
     * @endcode
     */
    const char* channels;

    /**
     * @brief Per-request timeout override in milliseconds (@b optional).
     *
     * @note When non-zero, takes priority over the context-level
     *       @c pubnub_config_t::transaction_timeout_ms.
     */
    uint32_t timeout_ms;
} pubnub_channel_group_remove_opts_t;

/** @brief Zero-initialize remove-channels options with protocol defaults. */
#define PUBNUB_CHANNEL_GROUP_REMOVE_OPTS_INIT {0}

/**
 * @brief Options for @c pubnub_channel_group_list_channels.
 *
 * Initialize with @c PUBNUB_CHANNEL_GROUP_LIST_OPTS_INIT before overriding
 * individual fields.
 *
 * @see pubnub_channel_group_list_channels
 */
typedef struct pubnub_channel_group_list_opts {
    /**
     * @brief Target channel group name (@b required, @b borrowed,
     *        NUL-terminated).
     */
    const char* channel_group;

    /**
     * @brief Per-request timeout override in milliseconds (@b optional).
     *
     * @note When non-zero, takes priority over the context-level
     *       @c pubnub_config_t::transaction_timeout_ms.
     */
    uint32_t timeout_ms;
} pubnub_channel_group_list_opts_t;

/** @brief Zero-initialize list-channels options with protocol defaults. */
#define PUBNUB_CHANNEL_GROUP_LIST_OPTS_INIT {0}

/**
 * @brief Options for @c pubnub_channel_group_remove.
 *
 * Initialize with @c PUBNUB_CHANNEL_GROUP_REMOVE_GROUP_OPTS_INIT before
 * overriding individual fields.
 *
 * @see pubnub_channel_group_remove
 */
typedef struct pubnub_channel_group_remove_group_opts {
    /**
     * @brief Target channel group name to delete (@b required,
     *        @b borrowed, NUL-terminated).
     */
    const char* channel_group;

    /**
     * @brief Per-request timeout override in milliseconds (@b optional).
     *
     * @note When non-zero, takes priority over the context-level
     *       @c pubnub_config_t::transaction_timeout_ms.
     */
    uint32_t timeout_ms;
} pubnub_channel_group_remove_group_opts_t;

/** @brief Zero-initialize remove-group options with protocol defaults. */
#define PUBNUB_CHANNEL_GROUP_REMOVE_GROUP_OPTS_INIT {0}

/**
 * @brief Result of a list-channels operation.
 *
 * Returned by value from @c pubnub_channel_group_list_result.
 * String views alias internal data valid until @c pubnub_future_release.
 */
typedef struct pubnub_channel_group_list_result {
    /** Number of channels in the group. */
    uint32_t count;
} pubnub_channel_group_list_result_t;

/**
 * @brief Add channels to a channel group.
 *
 * @param ctx  Initialized context (@b borrowed).
 * @param opts Add options struct (@b borrowed).
 * @return Future handle (owned by caller). Release via
 *         @c pubnub_future_release after reading results.
 *
 * @see pubnub_future_release
 * @see pubnub_channel_group_add_opts_t
 * @see pubnub_context_t
 * @see pubnub_future_t
 */
PUBNUB_API pubnub_future_t
pubnub_channel_group_add_channels(pubnub_context_t*                      ctx,
                                  const pubnub_channel_group_add_opts_t* opts);

/**
 * @brief Remove channels from a channel group.
 *
 * @param ctx  Initialized context (@b borrowed).
 * @param opts Remove options struct (@b borrowed).
 * @return Future handle (owned by caller). Release via
 *         @c pubnub_future_release after reading results.
 *
 * @see pubnub_future_release
 * @see pubnub_channel_group_remove_opts_t
 * @see pubnub_context_t
 * @see pubnub_future_t
 */
PUBNUB_API pubnub_future_t pubnub_channel_group_remove_channels(
    pubnub_context_t*                         ctx,
    const pubnub_channel_group_remove_opts_t* opts);

/**
 * @brief List channels in a channel group.
 *
 * @param ctx  Initialized context (@b borrowed).
 * @param opts List options struct (@b borrowed).
 * @return Future handle (owned by caller). Release via
 *         @c pubnub_future_release after reading results.
 *
 * @see pubnub_channel_group_list_result
 * @see pubnub_channel_group_list_result_channel_at
 * @see pubnub_future_release
 * @see pubnub_channel_group_list_opts_t
 * @see pubnub_context_t
 * @see pubnub_future_t
 */
PUBNUB_API pubnub_future_t
pubnub_channel_group_list_channels(pubnub_context_t* ctx,
                                   const pubnub_channel_group_list_opts_t* opts);

/**
 * @brief Delete a channel group entirely.
 *
 * @param ctx  Initialized context (@b borrowed).
 * @param opts Remove-group options struct (@b borrowed).
 * @return Future handle (owned by caller). Release via
 *         @c pubnub_future_release after reading results.
 *
 * @see pubnub_future_release
 * @see pubnub_channel_group_remove_group_opts_t
 * @see pubnub_context_t
 * @see pubnub_future_t
 */
PUBNUB_API pubnub_future_t pubnub_channel_group_remove(
    pubnub_context_t*                               ctx,
    const pubnub_channel_group_remove_group_opts_t* opts);

/**
 * @brief Aggregate result of a list-channels operation.
 *
 * @param future Future returned from @c pubnub_channel_group_list_channels.
 * @return Result struct; zero-initialized if future is not ready or invalid.
 */
PUBNUB_API pubnub_channel_group_list_result_t
pubnub_channel_group_list_result(pubnub_future_t future);

/**
 * @brief Indexed channel name from a list-channels result.
 *
 * The returned view aliases internal data valid until
 * @c pubnub_future_release is called on the same future.
 *
 * @param future Future returned from @c pubnub_channel_group_list_channels.
 * @param index  Zero-based channel index (must be < result.count).
 * @return Channel name view; zero-initialized if out of range or invalid.
 */
PUBNUB_API pubnub_string_view_t
pubnub_channel_group_list_result_channel_at(pubnub_future_t future, size_t index);

#ifdef __cplusplus
// clang-format off
}
// clang-format on
#endif

#endif /* PUBNUB_ENABLE_CHANNEL_GROUPS */

#endif /* PUBNUB_FEATURE_CHANNEL_GROUPS_H */
