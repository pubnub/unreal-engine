/* Copyright (c) PubNub Inc. */
/* See LICENSE in the root directory of this source tree. */

/**
 * @file pubnub.h
 * @brief Umbrella header -- includes the full public API surface.
 *
 * Feature-specific headers are conditionally included based on
 * compile-time feature toggles defined in pubnub/config.h.
 */

#ifndef PUBNUB_H
#define PUBNUB_H

#include "pubnub/config.h"
#include "pubnub/version.h"
#include "pubnub/error.h"
#include "pubnub/types.h"
#include "pubnub/capabilities.h"
#include "pubnub/client.h"
#include "pubnub/log.h"
#include "pubnub/future.h"
#include "pubnub/response.h"
#include "pubnub/service_error.h"
#include "pubnub/json.h"

#if PUBNUB_ENABLE_PUBLISH
#include "pubnub/features/publish.h"
#endif

#if PUBNUB_ENABLE_SUBSCRIBE
#include "pubnub/features/subscribe.h"
#endif

#if PUBNUB_ENABLE_PRESENCE
#include "pubnub/features/presence.h"
#endif

#if PUBNUB_ENABLE_HISTORY
#include "pubnub/features/history.h"
#endif

#if PUBNUB_ENABLE_MESSAGE_ACTIONS
#include "pubnub/features/message_actions.h"
#endif

#if PUBNUB_ENABLE_SIGNAL
#include "pubnub/features/signal.h"
#endif

#if PUBNUB_ENABLE_PAM
#include "pubnub/features/access.h"
#endif

#if PUBNUB_ENABLE_APP_CONTEXT
#include "pubnub/features/app_context.h"
#endif

#if PUBNUB_ENABLE_FILES
#include "pubnub/features/files.h"
#endif

#if PUBNUB_ENABLE_CHANNEL_GROUPS
#include "pubnub/features/channel_groups.h"
#endif

#if PUBNUB_ENABLE_CRYPTO
#include "pubnub/features/crypto.h"
#endif

#if PUBNUB_ENABLE_PUSH_NOTIFICATIONS
#include "pubnub/features/push.h"
#endif

#if PUBNUB_ENABLE_TIME
#include "pubnub/features/time.h"
#endif

#endif /* PUBNUB_H */
