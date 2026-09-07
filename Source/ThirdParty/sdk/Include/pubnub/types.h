/* Copyright (c) PubNub Inc. */
/* See LICENSE in the root directory of this source tree. */

/**
 * @file types.h
 * @brief Common scalar and view types used across the PubNub C SDK.
 */

#ifndef PUBNUB_TYPES_H
#define PUBNUB_TYPES_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
// clang-format off
extern "C" {
// clang-format on
#endif

/**
 * @brief Millisecond timestamp or duration.
 *
 * Used throughout the SDK for monotonic clock readings, timeout
 * durations, and elapsed-time calculations. Always represents
 * milliseconds.
 */
typedef uint64_t pubnub_milliseconds_t;

/**
 * @brief Sentinel value indicating no active timers in a timer list.
 *
 * Returned by pn_timer_list_ms_until_next() when the list is empty.
 */
#define PUBNUB_TIMER_LIST_NO_ACTIVE_TIMERS UINT64_MAX

/**
 * @brief Non-owning view over a contiguous character range.
 *
 * The pointed-to data is NOT required to be null-terminated --
 * always pair @c ptr with @c len (`printf("%.*s", (int)len, ptr)`
 * or `memcpy`) rather than assuming C-string semantics.
 *
 * Lifetime: the caller must ensure `ptr` remains valid for the
 * duration of any operation that references this view. Views
 * produced by the SDK typically alias into a feature's response
 * buffer and are valid until the caller releases the associated
 * @c pubnub_future_t.
 */
typedef struct pubnub_string_view {
    /** Start of the byte range (not NUL-terminated). */
    const char* ptr;
    /** Length in bytes. */
    size_t len;
} pubnub_string_view_t;

/**
 * @brief PubNub server timetoken — a 17-digit decimal string.
 *
 * Aliases @c pubnub_string_view_t. The pointed-to data is NOT
 * NUL-terminated; use `printf("%.*s", (int)tt.len, tt.ptr)` or
 * copy into a caller-owned buffer when a C-string is needed.
 *
 * Views produced by result accessors alias into a feature's response
 * buffer and are valid until @c pubnub_future_release is called on
 * the owning future. A zero-initialized value (@c {.ptr = NULL,
 * .len = 0}) means no timetoken was available.
 *
 * Inbound cursors (start/end in history/subscribe opts) are accepted
 * as NUL-terminated @c const @c char* strings.
 */
typedef pubnub_string_view_t pubnub_timetoken_t;

/**
 * @brief Event type discriminator shared across subscribe and history.
 *
 * Wire values 0–4 match the PubNub server's "e" / "message_type" field.
 * Negative values are SDK-internal concepts that never appear on the wire.
 *
 * Subscribe produces all values except UNKNOWN. History's fetch_messages
 * produces only MESSAGE, FILE, and UNKNOWN (when @c include_message_type
 * was not set in the request options).
 */
typedef enum pubnub_event_type {
    /** Field not included in the response (history only). */
    PUBNUB_EVENT_TYPE_UNKNOWN = -2,
    /** Presence event — SDK-assigned via @c -pnpres suffix detection,
     *  not a wire value (subscribe only). */
    PUBNUB_EVENT_TYPE_PRESENCE = -1,
    /** Regular published message. */
    PUBNUB_EVENT_TYPE_MESSAGE = 0,
    /** Signal (lightweight, no persistence). */
    PUBNUB_EVENT_TYPE_SIGNAL = 1,
    /** App Context (Objects) event. */
    PUBNUB_EVENT_TYPE_OBJECTS = 2,
    /** Message action event (reaction add/remove). */
    PUBNUB_EVENT_TYPE_MESSAGE_ACTION = 3,
    /** File sharing event. */
    PUBNUB_EVENT_TYPE_FILE = 4
} pubnub_event_type_t;

/**
 * @brief Opaque lock type.
 *
 * Size determined at runtime by the platform provider's lock_size()
 * method. The SDK allocates the required bytes via the allocator and
 * casts the result to `pubnub_lock_t*`. The struct is intentionally
 * never defined (incomplete type) -- this prevents accidental
 * dereference and gives the compiler type-safety over raw `void*`.
 */
typedef struct pubnub_lock pubnub_lock_t;

#ifdef __cplusplus
// clang-format off
}
// clang-format on
#endif

#endif /* PUBNUB_TYPES_H */
