/* Copyright (c) PubNub Inc. */
/* See LICENSE in the root directory of this source tree. */

/**
 * @file features/subscribe_types.h
 * @brief Public types for the subscribe feature: events, status
 *        categories, and listener callbacks.
 *
 * These types are shared between the subscribe public API and the
 * listener registration interface. Separating them into their own
 * header avoids a circular dependency between subscribe.h and
 * client-level listener helpers.
 */

#ifndef PUBNUB_FEATURE_SUBSCRIBE_TYPES_H
#define PUBNUB_FEATURE_SUBSCRIBE_TYPES_H

#include "pubnub/config.h"

#if PUBNUB_ENABLE_SUBSCRIBE

#include "pubnub/error.h"
#include "pubnub/pubnub_compat.h"
#include "pubnub/types.h"

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
// clang-format off
extern "C" {
// clang-format on
#endif

/** @brief Subscribe connection state for cooperative-polling queries. */
typedef enum pubnub_subscribe_connection_state {
    /** No active subscription — event engine is idle. */
    PUBNUB_SUBSCRIBE_IDLE = 0,
    /** Handshake in progress (first connect or subscription change). */
    PUBNUB_SUBSCRIBE_CONNECTING = 1,
    /** Long-poll active — receiving real-time events. */
    PUBNUB_SUBSCRIBE_CONNECTED = 2,
    /** Reconnecting after a failure (handshake or receive failed). */
    PUBNUB_SUBSCRIBE_RECONNECTING = 3,
    /** User-initiated disconnect (stopped). */
    PUBNUB_SUBSCRIBE_DISCONNECTED = 4
} pubnub_subscribe_connection_state_t;

/** @brief Subscribe status categories delivered to status listeners. */
typedef enum pubnub_subscribe_status {
    /** Handshake succeeded on first attempt — real-time stream active. */
    PUBNUB_SUBSCRIBE_STATUS_CONNECTED = 0,
    /** User-initiated graceful disconnect. */
    PUBNUB_SUBSCRIBE_STATUS_DISCONNECTED = 1,
    /** Connection lost without user intent (network drop, timeout). */
    PUBNUB_SUBSCRIBE_STATUS_DISCONNECTED_UNEXPECTEDLY = 2,
    /** Handshake attempt failed (network/server/timeout). */
    PUBNUB_SUBSCRIBE_STATUS_CONNECTION_ERROR = 3,
    /** Channel set changed while actively receiving. */
    PUBNUB_SUBSCRIBE_STATUS_SUBSCRIPTION_CHANGED = 4
} pubnub_subscribe_status_t;

/** @brief Message event types received over the subscribe stream. */
typedef enum pubnub_subscribe_message_type {
    /** Presence event (join/leave/timeout/state-change).
     *  SDK-assigned via `-pnpres` suffix detection, not a wire value. */
    PUBNUB_SUBSCRIBE_PRESENCE = -1,
    /** Regular published message (default when "e" absent). */
    PUBNUB_SUBSCRIBE_MESSAGE = 0,
    /** Signal (lightweight, no persistence). */
    PUBNUB_SUBSCRIBE_SIGNAL = 1,
    /** App Context event. */
    PUBNUB_SUBSCRIBE_APP_CONTEXT = 2,
    /** Message action event (reaction add/remove). */
    PUBNUB_SUBSCRIBE_MESSAGE_ACTION = 3,
    /** File sharing event. */
    PUBNUB_SUBSCRIBE_FILE = 4
} pubnub_subscribe_message_type_t;

/** Forward declaration for internal JSON value node. */
struct pubnub_json_value;

/**
 * @brief A single real-time event received over the subscribe stream.
 *
 * String views alias memory owned by the SDK (either the parsed JSON
 * tree or the transport response buffer). Views are valid only during
 * the listener callback invocation.
 *
 * @attention Do not store the view pointers beyond the callback
 *            boundary — copy the bytes you need into caller-owned storage.
 */
typedef struct pubnub_subscribe_event {
    /** Decoded event type. */
    pubnub_subscribe_message_type_t type;
    /** Channel the message arrived on (minus `-pnpres` suffix). */
    pubnub_string_view_t channel;
    /** Subscription match pattern (wildcard or channel group name). */
    pubnub_string_view_t subscription;
    /** Publisher UUID (may be empty for system events). */
    pubnub_string_view_t publisher;
    /** Parsed "d" (payload) node. For messages/signals this is the
     *  user-published content; for presence/objects/files it is the
     *  server-structured event body. Access via the serialization
     *  vtable: value_as_string() for strings, object_get()/array_get()
     *  for objects/arrays. @c NULL when "d" is absent. Valid only within
     *  the listener callback. */
    const struct pubnub_json_value* payload;
    /** Custom message type ("cmt" field, may be empty). */
    pubnub_string_view_t custom_message_type;
    /** Parsed "u" (user metadata) node. @c NULL when absent. Access via
     *  the serialization vtable. Valid only within the listener callback. */
    const struct pubnub_json_value* user_metadata;
    /** Publish timetoken (17-digit decimal string). */
    pubnub_string_view_t timetoken;
    /** Wire flags ("f" field). */
    uint32_t flags;
} pubnub_subscribe_event_t;

/**
 * @brief Enriched status event delivered to status listeners.
 *
 * Stack-allocated at the emit site; the pointer is valid only for the
 * duration of the callback invocation. Zero-initialization produces a
 * safe default (CONNECTED, PUBNUB_OK, no HTTP status).
 */
typedef struct pubnub_subscribe_status_event {
    /** Status category (connected, reconnected, error, etc.). */
    pubnub_subscribe_status_t status;
    /** Underlying error code. PUBNUB_OK for non-error statuses. */
    pubnub_res_t reason;
    /** HTTP status code when reason is PUBNUB_ERR_SERVER (e.g. 403,
     *  429); 0 otherwise. */
    uint16_t http_status_code;
    /** Comma-separated channel names now active (including -pnpres
     *  variants). Populated for SUBSCRIPTION_CHANGED and CONNECTED;
     *  empty view for other categories. Valid only within the callback
     *  invocation. */
    pubnub_string_view_t channels;
    /** Comma-separated channel group names now active. Populated for
     *  SUBSCRIPTION_CHANGED and CONNECTED; empty view for other
     *  categories. Valid only within the callback invocation. */
    pubnub_string_view_t groups;
} pubnub_subscribe_status_event_t;

/**
 * @brief Callback invoked on subscribe status changes.
 *
 * @param event     Status event (borrowed, valid for callback duration).
 * @param user_data Opaque pointer from listener registration.
 */
typedef void (*pubnub_subscribe_status_cb_t)(const pubnub_subscribe_status_event_t* event,
                                             void* user_data);

/**
 * @brief Callback invoked for every received message (type=MESSAGE).
 *
 * @param event     Event data (borrowed, valid for callback duration).
 * @param user_data Opaque pointer from listener registration.
 */
typedef void (*pubnub_subscribe_message_cb_t)(const pubnub_subscribe_event_t* event,
                                              void* user_data);

/**
 * @brief Callback invoked for signal events (type=SIGNAL).
 *
 * @param event     Event data (borrowed, valid for callback duration).
 * @param user_data Opaque pointer from listener registration.
 */
typedef void (*pubnub_subscribe_signal_cb_t)(const pubnub_subscribe_event_t* event,
                                             void* user_data);

/**
 * @brief Callback invoked for presence events (type=PRESENCE).
 *
 * @param event     Event data (borrowed, valid for callback duration).
 * @param user_data Opaque pointer from listener registration.
 */
typedef void (*pubnub_subscribe_presence_cb_t)(const pubnub_subscribe_event_t* event,
                                               void* user_data);

/**
 * @brief Callback invoked for message action events.
 *
 * @param event     Event data (borrowed, valid for callback duration).
 * @param user_data Opaque pointer from listener registration.
 */
typedef void (*pubnub_subscribe_message_action_cb_t)(const pubnub_subscribe_event_t* event,
                                                     void* user_data);

/**
 * @brief Callback invoked for App Context events.
 *
 * @param event     Event data (borrowed, valid for callback duration).
 * @param user_data Opaque pointer from listener registration.
 */
typedef void (*pubnub_subscribe_app_context_cb_t)(const pubnub_subscribe_event_t* event,
                                                  void* user_data);

/**
 * @brief Callback invoked for file sharing events.
 *
 * @param event     Event data (borrowed, valid for callback duration).
 * @param user_data Opaque pointer from listener registration.
 */
typedef void (*pubnub_subscribe_file_cb_t)(const pubnub_subscribe_event_t* event,
                                           void* user_data);

/**
 * @brief Listener registration structure with typed callbacks.
 *
 * Pass to @c pubnub_add_listener (for global/context-wide dispatch),
 * @c pubnub_subscription_add_listener (for per-subscription
 * filtering), or @c pubnub_subscription_set_add_listener (for
 * per-set filtering).
 *
 * All callback fields are optional (NULL = not interested in that
 * event type). The SDK dispatches received events to the matching
 * typed callback.
 *
 * Designated-initializer friendly: zero-initialize and set only the
 * callbacks you need:
 *
 * @code
 * pubnub_subscribe_listener_t listener = {
 *     .on_message = my_msg_handler,
 *     .on_status  = my_status_handler,
 *     .user_data  = my_ctx,
 * };
 * @endcode
 */
typedef struct pubnub_subscribe_listener {
    /** Status change callback (may be @c NULL) invoked only for
     * context-global listeners (registered via @c pubnub_add_listener). */
    pubnub_subscribe_status_cb_t on_status;
    /** Regular message callback (may be @c NULL). */
    pubnub_subscribe_message_cb_t on_message;
    /** Signal callback (may be @c NULL). */
    pubnub_subscribe_signal_cb_t on_signal;
    /** Presence event callback (may be @c NULL). */
    pubnub_subscribe_presence_cb_t on_presence;
    /** Message action callback (may be @c NULL). */
    pubnub_subscribe_message_action_cb_t on_message_action;
    /** App Context callback (may be @c NULL). */
    pubnub_subscribe_app_context_cb_t on_app_context;
    /** File event callback (may be @c NULL). */
    pubnub_subscribe_file_cb_t on_file;
    /** Opaque user data forwarded to all callbacks. */
    void* user_data;
} pubnub_subscribe_listener_t;

/**
 * @brief Opaque handle for a listener registration.
 *
 * Returned by @c pubnub_add_listener (or the per-subscription /
 * per-set variants) and used to remove the listener later via the
 * corresponding remove function.
 */
typedef uint16_t pubnub_listener_handle_t;

/** Sentinel for an invalid listener handle. */
#define PUBNUB_LISTENER_HANDLE_INVALID ((uint16_t)UINT16_MAX)

/** @brief Entity type for subscribe channel specification. */
typedef enum pubnub_subscribe_entity_type {
    /** Regular channel. */
    PUBNUB_SUBSCRIBE_CHANNEL = 0,
    /** Channel group (resolved server-side). */
    PUBNUB_SUBSCRIBE_CHANNEL_GROUP = 1,
    /** Channel metadata entity (App Context). */
    PUBNUB_SUBSCRIBE_CHANNEL_METADATA = 2,
    /** User metadata entity (App Context). */
    PUBNUB_SUBSCRIBE_USER_METADATA = 3
} pubnub_subscribe_entity_type_t;

/**
 * @brief Opaque entity handle.
 *
 * Represents a subscribable entity (channel, channel group, or
 * metadata object). Created via factory functions such as
 * @c pubnub_channel, @c pubnub_channel_group,
 * @c pubnub_channel_metadata, or @c pubnub_user_metadata.
 * Must be destroyed with @c pubnub_entity_destroy when no longer
 * needed.
 */
typedef struct pubnub_entity* pubnub_entity_t;

/**
 * @brief Options for creating a subscription.
 *
 * Designated-initializer friendly: zero-initialize and set only the
 * fields you need:
 *
 * @code
 * pubnub_subscription_opts_t opts = {
 *     .with_presence = 1,
 * };
 * @endcode
 *
 * @see pubnub_subscription_create
 */
typedef struct pubnub_subscription_opts {
    /** 1 = also subscribe to presence events (<channel>-pnpres).
     *  Silently ignored for metadata entities. */
    uint8_t with_presence;
} pubnub_subscription_opts_t;

/** @brief Zero-initializes subscription options. Call before overriding fields. */
#define PUBNUB_SUBSCRIPTION_OPTS_INIT {0}

/**
 * @brief Opaque subscription handle.
 *
 * Represents a single channel/channel-group subscription. Created
 * via @c pubnub_subscription_create, must be destroyed with
 * @c pubnub_subscription_destroy when no longer needed.
 */
typedef struct pubnub_subscription* pubnub_subscription_t;

/**
 * @brief Opaque subscription set handle.
 *
 * Represents a logical group of subscriptions that can be activated
 * or deactivated atomically. Created via
 * @c pubnub_subscription_set_create, destroyed with
 * @c pubnub_subscription_set_destroy. Listeners registered on a
 * set receive events only from channels/groups in that set.
 *
 * @c NULL indicates an invalid/exhausted handle.
 */
typedef struct pubnub_subscription_set* pubnub_subscription_set_t;

/** Sentinel for an invalid subscription set handle. */
#define PUBNUB_SUBSCRIPTION_SET_INVALID ((pubnub_subscription_set_t)NULL)

/**
 * @brief Message event (type == @c PUBNUB_SUBSCRIBE_MESSAGE).
 *
 * Populated by @c pubnub_subscribe_event_message. All pointers are
 * valid only within the listener callback.
 */
typedef struct pubnub_subscribe_message_event {
    /** Channel the message arrived on. */
    pubnub_string_view_t channel;
    /** Subscription match pattern. */
    pubnub_string_view_t subscription;
    /** Publisher UUID. */
    pubnub_string_view_t publisher;
    /** Publish timetoken. */
    pubnub_string_view_t timetoken;
    /** Custom message type (may be empty). */
    pubnub_string_view_t custom_message_type;
    /** Message payload ("d" field). Access via the serialization vtable:
     *  value_as_string() for string messages, object_get()/array_get()
     *  for JSON object/array messages. @c NULL when absent. */
    const struct pubnub_json_value* message;
    /** User metadata ("u" field). @c NULL when absent. */
    const struct pubnub_json_value* user_metadata;
} pubnub_subscribe_message_event_t;

/**
 * @brief Signal event (type == @c PUBNUB_SUBSCRIBE_SIGNAL).
 *
 * Populated by @c pubnub_subscribe_event_signal. All pointers are
 * valid only within the listener callback.
 */
typedef struct pubnub_subscribe_signal_event {
    /** Channel the signal arrived on. */
    pubnub_string_view_t channel;
    /** Subscription match pattern. */
    pubnub_string_view_t subscription;
    /** Publisher UUID. */
    pubnub_string_view_t publisher;
    /** Publish timetoken. */
    pubnub_string_view_t timetoken;
    /** Custom message type (may be empty). */
    pubnub_string_view_t custom_message_type;
    /** Signal payload ("d" field). Access via the serialization vtable. */
    const struct pubnub_json_value* message;
    /** User metadata ("u" field). @c NULL when absent. */
    const struct pubnub_json_value* user_metadata;
} pubnub_subscribe_signal_event_t;

/**
 * @brief Presence event actions decoded from the subscribe stream.
 */
typedef enum pubnub_presence_action {
    /** A user joined the channel. */
    PUBNUB_PRESENCE_JOIN = 0,
    /** A user left the channel. */
    PUBNUB_PRESENCE_LEAVE = 1,
    /** A user timed out (heartbeat expired). */
    PUBNUB_PRESENCE_TIMEOUT = 2,
    /** A user's state changed on the channel. */
    PUBNUB_PRESENCE_STATE_CHANGE = 3,
    /** Periodic interval event with occupancy update. */
    PUBNUB_PRESENCE_INTERVAL = 4
} pubnub_presence_action_t;

/**
 * @brief Decoded presence event data
 *        (type == @c PUBNUB_SUBSCRIBE_PRESENCE).
 *
 * Populated by @c pubnub_subscribe_event_presence from a source
 * @c pubnub_subscribe_event_t. All string views in this struct
 * alias memory from the source event and become invalid after the
 * listener callback returns.
 */
typedef struct pubnub_subscribe_presence_event {
    /** Presence action (join, leave, timeout, state-change, interval). */
    pubnub_presence_action_t action;
    /** UUID of the user who triggered the event (may be empty for interval). */
    pubnub_string_view_t uuid;
    /** Channel on which the event occurred. */
    pubnub_string_view_t channel;
    /** Subscription match pattern (wildcard or channel group). */
    pubnub_string_view_t subscription;
    /** Current occupancy count. */
    uint32_t occupancy;
    /** Publish timetoken of the event. */
    pubnub_string_view_t timetoken;
    /** User-set presence state object. @c NULL when absent.
     *  Valid only within the listener callback. Walk with
     *  serial->object_get() to read individual fields. */
    const struct pubnub_json_value* state;
    /** Array of UUIDs that joined since last interval. @c NULL when
     *  action != INTERVAL or presence_deltas disabled. Valid only
     *  within the listener callback. Iterate with
     *  serial->array_size() / array_get() / value_as_string(). */
    const struct pubnub_json_value* joined;
    /** Array of UUIDs that left since last interval. @c NULL when
     *  action != INTERVAL or presence_deltas disabled. Valid only
     *  within the listener callback. Iterate same as @c joined. */
    const struct pubnub_json_value* left;
    /** Array of UUIDs that timed out since last interval. @c NULL when
     *  action != INTERVAL or presence_deltas disabled. Valid only
     *  within the listener callback. Iterate same as @c joined. */
    const struct pubnub_json_value* timed_out;
    /** 1 = UUID arrays omitted (payload > 30KB); call here_now for
     *  the full occupancy list. 0 = arrays present (or non-interval). */
    uint8_t here_now_refresh;
} pubnub_subscribe_presence_event_t;

PUBNUB_STATIC_ASSERT(sizeof(pubnub_subscribe_presence_event_t) <= 128,
                     "presence_event exceeds embedded stack budget");

/**
 * @brief Message action event types.
 */
typedef enum pubnub_message_action_type {
    /** A reaction or action was added to a message. */
    PUBNUB_MESSAGE_ACTION_ADDED = 0,
    /** A reaction or action was removed from a message. */
    PUBNUB_MESSAGE_ACTION_REMOVED = 1
} pubnub_message_action_type_t;

/**
 * @brief Decoded message action event.
 *
 * Extracted from a subscribe event whose type is
 * @c PUBNUB_SUBSCRIBE_MESSAGE_ACTION.
 */
typedef struct pubnub_subscribe_message_action_event {
    /** Whether the action was added or removed. */
    pubnub_message_action_type_t event;
    /** Channel on which the action occurred. */
    pubnub_string_view_t channel;
    /** Subscription match pattern. */
    pubnub_string_view_t subscription;
    /** Publisher of the original message. */
    pubnub_string_view_t publisher;
    /** Timetoken of the message being acted on. */
    pubnub_string_view_t message_timetoken;
    /** Timetoken when the action itself was created. */
    pubnub_string_view_t action_timetoken;
    /** Action type (e.g. "reaction"). */
    pubnub_string_view_t type;
    /** Action value (e.g. emoji string). */
    pubnub_string_view_t value;
} pubnub_subscribe_message_action_event_t;

/**
 * @brief App Context event types.
 */
typedef enum pubnub_app_context_event_type {
    /** An object was set (created or updated). */
    PUBNUB_APP_CONTEXT_SET = 0,
    /** An object was removed. */
    PUBNUB_APP_CONTEXT_REMOVED = 1
} pubnub_app_context_event_type_t;

/**
 * @brief App Context object type discriminator.
 */
typedef enum pubnub_app_context_object_type {
    /** Unrecognized or absent object type (zero-init safe). */
    PUBNUB_APP_CONTEXT_OBJECT_UNKNOWN = 0,
    /** UUID (user) metadata object. */
    PUBNUB_APP_CONTEXT_OBJECT_UUID = 1,
    /** Channel metadata object. */
    PUBNUB_APP_CONTEXT_OBJECT_CHANNEL = 2,
    /** Membership relation object. */
    PUBNUB_APP_CONTEXT_OBJECT_MEMBERSHIP = 3
} pubnub_app_context_object_type_t;

/**
 * @brief Decoded App Context event.
 *
 * Extracted from a subscribe event whose type is
 * @c PUBNUB_SUBSCRIBE_APP_CONTEXT.
 *
 * Use @c object_type to select the appropriate typed sub-accessor:
 * - @c PUBNUB_APP_CONTEXT_OBJECT_UUID → pubnub_subscribe_app_context_uuid_metadata()
 * - @c PUBNUB_APP_CONTEXT_OBJECT_CHANNEL → pubnub_subscribe_app_context_channel_metadata()
 * - @c PUBNUB_APP_CONTEXT_OBJECT_MEMBERSHIP → pubnub_subscribe_app_context_membership()
 */
typedef struct pubnub_subscribe_app_context_event {
    /** Event type (set or removed). */
    pubnub_app_context_event_type_t event;
    /** Decoded object type. */
    pubnub_app_context_object_type_t object_type;
    /** Channel on which the event was delivered. */
    pubnub_string_view_t channel;
    /** Subscription match pattern. */
    pubnub_string_view_t subscription;
    /** Parsed "data" payload node. Valid only within the listener
     *  callback. Prefer the typed sub-accessors listed above. */
    const struct pubnub_json_value* data;
} pubnub_subscribe_app_context_event_t;

/**
 * @brief Decoded file sharing event.
 *
 * Extracted from a subscribe event whose type is
 * @c PUBNUB_SUBSCRIBE_FILE.
 */
typedef struct pubnub_subscribe_file_event {
    /** Channel on which the file was published. */
    pubnub_string_view_t channel;
    /** Subscription match pattern. */
    pubnub_string_view_t subscription;
    /** Publisher UUID. */
    pubnub_string_view_t publisher;
    /** Server-generated file identifier. */
    pubnub_string_view_t file_id;
    /** Human-readable file name. */
    pubnub_string_view_t file_name;
    /** User-supplied message attached to the file. @c NULL when absent.
     *  Valid only within the listener callback. Walk with
     *  serial->object_get() / value_as_string() to read fields. */
    const struct pubnub_json_value* message;
    /** Publish timetoken. */
    pubnub_string_view_t timetoken;
} pubnub_subscribe_file_event_t;

#ifdef __cplusplus
// clang-format off
}
// clang-format on
#endif

#endif /* PUBNUB_ENABLE_SUBSCRIBE */

#endif /* PUBNUB_FEATURE_SUBSCRIBE_TYPES_H */
