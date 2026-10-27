/* Copyright (c) PubNub Inc. */
/* See LICENSE in the root directory of this source tree. */

#ifndef PUBNUB_FEATURE_PUSH_H
#define PUBNUB_FEATURE_PUSH_H

#include "pubnub/config.h"

#if PUBNUB_ENABLE_PUSH_NOTIFICATIONS

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

/** @brief Push notification gateway type. */
typedef enum pubnub_push_gateway {
    /** Apple Push Notification Service (HTTP/2). */
    PUBNUB_PUSH_APNS2 = 0,
    /** Firebase Cloud Messaging. */
    PUBNUB_PUSH_FCM = 1,
} pubnub_push_gateway_t;

/** @brief Push notification environment (APNS2 only). */
typedef enum pubnub_push_environment {
    /** Development/sandbox APNS certificates (@b default). */
    PUBNUB_PUSH_ENV_DEVELOPMENT = 0,
    /** Production APNS certificates. */
    PUBNUB_PUSH_ENV_PRODUCTION = 1,
} pubnub_push_environment_t;

/**
 * @brief Options for @c pubnub_push_add_channels.
 *
 * Initialize with @c PUBNUB_PUSH_ADD_CHANNELS_OPTS_INIT before overriding
 * individual fields.
 */
typedef struct pubnub_push_add_channels_opts {
    /**
     * @brief Device token (@b required, @b borrowed, NUL-terminated).
     *
     * Hexadecimal APNS device token or FCM registration token string.
     */
    const char* device;

    /** @brief Push gateway type (@b required). */
    pubnub_push_gateway_t gateway;

    /**
     * @brief Comma-separated channel list (@b required, @b borrowed,
     *        NUL-terminated).
     */
    const char* channels;

    /**
     * @brief APNS2 topic / bundle ID (@b required for APNS2,
     *        @b borrowed, NUL-terminated).
     *
     * Ignored for FCM.
     */
    const char* topic;

    /**
     * @brief APNS2 environment.
     *
     * @b Default: @c PUBNUB_PUSH_ENV_DEVELOPMENT. Ignored for FCM.
     */
    pubnub_push_environment_t environment;

    /**
     * @brief Per-request timeout override in milliseconds (@b optional).
     *
     * @note When non-zero, takes priority over the context-level
     *       @c pubnub_config_t::transaction_timeout_ms.
     */
    uint32_t timeout_ms;
} pubnub_push_add_channels_opts_t;

/**
 * @brief Default initializer for @c pubnub_push_add_channels_opts_t.
 *
 * Set device, gateway, and channels before calling
 * @c pubnub_push_add_channels. For APNS2, also set topic.
 */
#define PUBNUB_PUSH_ADD_CHANNELS_OPTS_INIT {0}

/**
 * @brief Options for @c pubnub_push_remove_channels.
 *
 * Initialize with @c PUBNUB_PUSH_REMOVE_CHANNELS_OPTS_INIT before overriding
 * individual fields.
 */
typedef struct pubnub_push_remove_channels_opts {
    /** @brief Device token (@b required, @b borrowed, NUL-terminated). */
    const char* device;

    /** @brief Push gateway type (@b required). */
    pubnub_push_gateway_t gateway;

    /**
     * @brief Comma-separated channel list (@b required, @b borrowed,
     *        NUL-terminated).
     */
    const char* channels;

    /**
     * @brief APNS2 topic / bundle ID (@b required for APNS2,
     *        @b borrowed, NUL-terminated).
     *
     * Ignored for FCM.
     */
    const char* topic;

    /**
     * @brief APNS2 environment.
     *
     * @b Default: @c PUBNUB_PUSH_ENV_DEVELOPMENT. Ignored for FCM.
     */
    pubnub_push_environment_t environment;

    /**
     * @brief Per-request timeout override in milliseconds (@b optional).
     *
     * @note When non-zero, takes priority over the context-level
     *       @c pubnub_config_t::transaction_timeout_ms.
     */
    uint32_t timeout_ms;
} pubnub_push_remove_channels_opts_t;

/**
 * @brief Default initializer for @c pubnub_push_remove_channels_opts_t.
 *
 * Set device, gateway, and channels before calling
 * @c pubnub_push_remove_channels. For APNS2, also set topic.
 */
#define PUBNUB_PUSH_REMOVE_CHANNELS_OPTS_INIT {0}

/**
 * @brief Options for @c pubnub_push_list_channels.
 *
 * Initialize with @c PUBNUB_PUSH_LIST_CHANNELS_OPTS_INIT before overriding
 * individual fields.
 */
typedef struct pubnub_push_list_channels_opts {
    /** @brief Device token (@b required, @b borrowed, NUL-terminated). */
    const char* device;

    /** @brief Push gateway type (@b required). */
    pubnub_push_gateway_t gateway;

    /**
     * @brief APNS2 topic / bundle ID (@b required for APNS2,
     *        @b borrowed, NUL-terminated).
     *
     * Ignored for FCM.
     */
    const char* topic;

    /**
     * @brief APNS2 environment.
     *
     * @b Default: @c PUBNUB_PUSH_ENV_DEVELOPMENT. Ignored for FCM.
     */
    pubnub_push_environment_t environment;

    /**
     * @brief Pagination cursor from a previous response (@b optional,
     *        @b borrowed, NUL-terminated).
     *
     * Pass @c NULL to start from the beginning.
     */
    const char* start;

    /**
     * @brief Maximum channels per page.
     *
     * @b Default: @c 0 (server default, typically 500). Maximum 1000.
     */
    uint16_t count;

    /**
     * @brief Per-request timeout override in milliseconds (@b optional).
     *
     * @note When non-zero, takes priority over the context-level
     *       @c pubnub_config_t::transaction_timeout_ms.
     */
    uint32_t timeout_ms;
} pubnub_push_list_channels_opts_t;

/**
 * @brief Default initializer for @c pubnub_push_list_channels_opts_t.
 *
 * Set device and gateway before calling @c pubnub_push_list_channels.
 * For APNS2, also set topic.
 */
#define PUBNUB_PUSH_LIST_CHANNELS_OPTS_INIT {0}

/**
 * @brief Options for @c pubnub_push_remove_device.
 *
 * Initialize with @c PUBNUB_PUSH_REMOVE_DEVICE_OPTS_INIT before overriding
 * individual fields.
 */
typedef struct pubnub_push_remove_device_opts {
    /** @brief Device token (@b required, @b borrowed, NUL-terminated). */
    const char* device;

    /** @brief Push gateway type (@b required). */
    pubnub_push_gateway_t gateway;

    /**
     * @brief APNS2 topic / bundle ID (@b required for APNS2,
     *        @b borrowed, NUL-terminated).
     *
     * Ignored for FCM.
     */
    const char* topic;

    /**
     * @brief APNS2 environment.
     *
     * @b Default: @c PUBNUB_PUSH_ENV_DEVELOPMENT. Ignored for FCM.
     */
    pubnub_push_environment_t environment;

    /**
     * @brief Per-request timeout override in milliseconds (@b optional).
     *
     * @note When non-zero, takes priority over the context-level
     *       @c pubnub_config_t::transaction_timeout_ms.
     */
    uint32_t timeout_ms;
} pubnub_push_remove_device_opts_t;

/**
 * @brief Default initializer for @c pubnub_push_remove_device_opts_t.
 *
 * Set device and gateway before calling @c pubnub_push_remove_device.
 * For APNS2, also set topic.
 */
#define PUBNUB_PUSH_REMOVE_DEVICE_OPTS_INIT {0}

/**
 * @brief Aggregate result for @c pubnub_push_list_channels.
 *
 * Contains the channel count; iterate with
 * @c pubnub_push_list_channels_result_channel_at.
 */
typedef struct pubnub_push_list_channels_result {
    /** Number of channels in the response. */
    uint32_t channel_count;
} pubnub_push_list_channels_result_t;

/**
 * @brief Register channels for push notifications on a device.
 *
 * Cooperative polling
 * @code
 * pubnub_future_t fut = pubnub_push_add_channels(ctx,
 *     &(pubnub_push_add_channels_opts_t){
 *         .device   = "dXh7YzE:APA91bGExample",
 *         .gateway  = PUBNUB_PUSH_FCM,
 *         .channels = "alerts,updates",
 *     });
 *
 * while (!pubnub_future_is_ready(fut)) {
 *     pubnub_process(ctx);
 * }
 * pubnub_future_release(fut);
 * @endcode
 *
 * @param ctx  Initialized context (@b borrowed).
 * @param opts Add-channels options (@b borrowed).
 * @return Future handle (owned by caller). Release via @c pubnub_future_release
 *         after reading results.
 *
 * @see pubnub_future_release
 * @see pubnub_push_add_channels_opts_t
 * @see pubnub_context_t
 * @see pubnub_future_t
 */
PUBNUB_API pubnub_future_t
pubnub_push_add_channels(pubnub_context_t*                      ctx,
                         const pubnub_push_add_channels_opts_t* opts);

/**
 * @brief Unregister channels from push notifications on a device.
 *
 * @param ctx  Initialized context (@b borrowed).
 * @param opts Remove-channels options (@b borrowed).
 * @return Future handle (owned by caller). Release via @c pubnub_future_release
 *         after reading results.
 *
 * @see pubnub_future_release
 * @see pubnub_push_remove_channels_opts_t
 * @see pubnub_context_t
 * @see pubnub_future_t
 */
PUBNUB_API pubnub_future_t
pubnub_push_remove_channels(pubnub_context_t*                         ctx,
                            const pubnub_push_remove_channels_opts_t* opts);

/**
 * @brief List channels registered for push on a device.
 *
 * On success, retrieve channel names with
 * @c pubnub_push_list_channels_result and
 * @c pubnub_push_list_channels_result_channel_at.
 *
 * @param ctx  Initialized context (@b borrowed).
 * @param opts List-channels options (@b borrowed).
 * @return Future handle (owned by caller). Release via @c pubnub_future_release
 *         after reading results.
 *
 * @see pubnub_push_list_channels_result
 * @see pubnub_future_release
 * @see pubnub_push_list_channels_opts_t
 * @see pubnub_context_t
 * @see pubnub_future_t
 */
PUBNUB_API pubnub_future_t
pubnub_push_list_channels(pubnub_context_t*                       ctx,
                          const pubnub_push_list_channels_opts_t* opts);

/**
 * @brief Remove a device from all push notification registrations.
 *
 * @param ctx  Initialized context (@b borrowed).
 * @param opts Remove-device options (@b borrowed).
 * @return Future handle (owned by caller). Release via @c pubnub_future_release
 *         after reading results.
 *
 * @see pubnub_future_release
 * @see pubnub_push_remove_device_opts_t
 * @see pubnub_context_t
 * @see pubnub_future_t
 */
PUBNUB_API pubnub_future_t
pubnub_push_remove_device(pubnub_context_t*                       ctx,
                          const pubnub_push_remove_device_opts_t* opts);

/**
 * @brief Retrieve the list-channels result summary.
 *
 * @param future Completed future from @c pubnub_push_list_channels.
 * @return Result struct; zero-initialized if future is invalid or not ready.
 */
PUBNUB_API pubnub_push_list_channels_result_t
pubnub_push_list_channels_result(pubnub_future_t future);

/**
 * @brief Retrieve a channel name by index from a list-channels result.
 *
 * The returned view is valid until @c pubnub_future_release is called.
 *
 * @param future Completed future from @c pubnub_push_list_channels.
 * @param index  Zero-based channel index.
 * @return Channel name view, or zero-initialized view on error/out-of-range.
 */
PUBNUB_API pubnub_string_view_t
pubnub_push_list_channels_result_channel_at(pubnub_future_t future, size_t index);

#ifdef __cplusplus
// clang-format off
}
// clang-format on
#endif

#endif /* PUBNUB_ENABLE_PUSH_NOTIFICATIONS */

#endif /* PUBNUB_FEATURE_PUSH_H */
