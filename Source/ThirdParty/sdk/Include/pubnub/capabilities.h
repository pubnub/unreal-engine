/* Copyright (c) PubNub Inc. */
/* See LICENSE in the root directory of this source tree. */

#ifndef PUBNUB_CAPABILITIES_H
#define PUBNUB_CAPABILITIES_H

#include "pubnub/config.h"
#include "pubnub/pubnub_compat.h"
#include "pubnub/types_fwd.h"

#include <stdint.h>

#ifdef __cplusplus
// clang-format off
extern "C" {
// clang-format on
#endif

/**
 * @brief Identifies an SDK feature for capability queries and per-context
 *        registration.
 */
typedef enum pubnub_feature {
    /** Publish messages to a channel. */
    PUBNUB_FEATURE_PUBLISH = 0,
    /** Subscribe to channels / groups. */
    PUBNUB_FEATURE_SUBSCRIBE = 1,
    /** Presence (here-now, leave/join). */
    PUBNUB_FEATURE_PRESENCE = 2,
    /** Message history retrieval. */
    PUBNUB_FEATURE_HISTORY = 3,
    /** Message reactions / actions. */
    PUBNUB_FEATURE_MESSAGE_ACTIONS = 4,
    /** Lightweight signal messages. */
    PUBNUB_FEATURE_SIGNAL = 5,
    /** PubNub Access Manager. */
    PUBNUB_FEATURE_PAM = 6,
    /** App Context API. */
    PUBNUB_FEATURE_APP_CONTEXT = 7,
    /** File sharing API. */
    PUBNUB_FEATURE_FILES = 8,
    /** Channel-group management. */
    PUBNUB_FEATURE_CHANNEL_GROUPS = 9,
    /** Payload encryption. */
    PUBNUB_FEATURE_CRYPTO = 10,
    /** Push notification device management. */
    PUBNUB_FEATURE_PUSH_NOTIFICATIONS = 11,
    /** Get current PubNub server timetoken. */
    PUBNUB_FEATURE_TIME = 12,

    /** Sentinel — must not exceed 31 (bitmap width). */
    PUBNUB_FEATURE_COUNT = 13
} pubnub_feature_t;

/**
 * @brief Query whether a feature is active on a context.
 *
 * @param ctx     Initialized context, or @c NULL.
 * @param feature Feature identifier to query.
 * @return Non-zero if active, @c 0 if disabled or @c ctx is @c NULL.
 */
PUBNUB_API int pubnub_has_feature(const pubnub_context_t* ctx,
                                  pubnub_feature_t        feature);

#ifdef __cplusplus
// clang-format off
}
// clang-format on
#endif

#endif /* PUBNUB_CAPABILITIES_H */
