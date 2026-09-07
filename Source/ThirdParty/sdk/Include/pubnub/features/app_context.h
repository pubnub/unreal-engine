/* Copyright (c) PubNub Inc. */
/* See LICENSE in the root directory of this source tree. */

#ifndef PUBNUB_FEATURE_APP_CONTEXT_H
#define PUBNUB_FEATURE_APP_CONTEXT_H

#include "pubnub/config.h"

#if PUBNUB_ENABLE_APP_CONTEXT

#include "pubnub/error.h"
#include "pubnub/future.h"
#include "pubnub/types.h"

#include <stddef.h>
#include <stdint.h>

/** Forward declaration for JSON value tree nodes. */
struct pubnub_json_value;

#ifdef __cplusplus
// clang-format off
extern "C" {
// clang-format on
#endif

/**
 * @brief Include bitmask flags for App Context operations.
 *
 * Combine with bitwise OR to request additional data in responses.
 * Pass zero for server defaults (no additional includes).
 */
/** @{ */
/** Include custom data object for the entity. */
#define PUBNUB_APP_CONTEXT_INCLUDE_CUSTOM (1u << 0)
/** Include type field. */
#define PUBNUB_APP_CONTEXT_INCLUDE_TYPE (1u << 1)
/** Include status field. */
#define PUBNUB_APP_CONTEXT_INCLUDE_STATUS (1u << 2)
/** Request total count in paginated responses. */
#define PUBNUB_APP_CONTEXT_INCLUDE_TOTAL_COUNT (1u << 3)
/** Include UUID metadata on membership/member results. */
#define PUBNUB_APP_CONTEXT_INCLUDE_UUID (1u << 4)
/** Include UUID custom data on membership/member results. */
#define PUBNUB_APP_CONTEXT_INCLUDE_UUID_CUSTOM (1u << 5)
/** Include UUID type on membership/member results. */
#define PUBNUB_APP_CONTEXT_INCLUDE_UUID_TYPE (1u << 6)
/** Include UUID status on membership/member results. */
#define PUBNUB_APP_CONTEXT_INCLUDE_UUID_STATUS (1u << 7)
/** Include channel metadata on membership/member results. */
#define PUBNUB_APP_CONTEXT_INCLUDE_CHANNEL (1u << 8)
/** Include channel custom data on membership/member results. */
#define PUBNUB_APP_CONTEXT_INCLUDE_CHANNEL_CUSTOM (1u << 9)
/** Include channel type on membership/member results. */
#define PUBNUB_APP_CONTEXT_INCLUDE_CHANNEL_TYPE (1u << 10)
/** Include channel status on membership/member results. */
#define PUBNUB_APP_CONTEXT_INCLUDE_CHANNEL_STATUS (1u << 11)

/** @} */

/**
 * @brief UUID metadata returned from App Context operations.
 *
 * All string views alias the parsed response body and are valid until
 * @c pubnub_future_release is called on the owning future.
 * Fields not included in the response are zero-initialized.
 */
typedef struct pubnub_uuid_metadata {
    /** Unique identifier for the UUID object. */
    pubnub_string_view_t id;
    /** Display name. */
    pubnub_string_view_t name;
    /** External system identifier. */
    pubnub_string_view_t external_id;
    /** Profile URL. */
    pubnub_string_view_t profile_url;
    /** Email address. */
    pubnub_string_view_t email;
    /** Classification type label. */
    pubnub_string_view_t type;
    /** Lifecycle status label. */
    pubnub_string_view_t status;
    /** User-defined custom data object. Walk with
     *  serial->object_get(). NULL when absent.
     *  Valid until pubnub_future_release(). */
    const struct pubnub_json_value* custom;
    /** ISO 8601 last-updated timestamp. */
    pubnub_string_view_t updated;
    /** Entity tag for conditional updates. */
    pubnub_string_view_t etag;
} pubnub_uuid_metadata_t;

/**
 * @brief Channel metadata returned from App Context operations.
 *
 * All string views alias the parsed response body and are valid until
 * @c pubnub_future_release is called on the owning future.
 * Fields not included in the response are zero-initialized.
 */
typedef struct pubnub_channel_metadata {
    /** Unique identifier for the channel object. */
    pubnub_string_view_t id;
    /** Display name. */
    pubnub_string_view_t name;
    /** Human-readable description. */
    pubnub_string_view_t description;
    /** Classification type label. */
    pubnub_string_view_t type;
    /** Lifecycle status label. */
    pubnub_string_view_t status;
    /** User-defined custom data object. Walk with
     *  serial->object_get(). NULL when absent.
     *  Valid until pubnub_future_release(). */
    const struct pubnub_json_value* custom;
    /** ISO 8601 last-updated timestamp. */
    pubnub_string_view_t updated;
    /** Entity tag for conditional updates. */
    pubnub_string_view_t etag;
} pubnub_channel_metadata_t;

/**
 * @brief Membership result (UUID-to-channel relationship).
 *
 * Returned when querying which channels a UUID belongs to.
 * The embedded @c channel struct may be partially populated
 * depending on include flags.
 */
typedef struct pubnub_membership {
    /** Channel metadata (partial unless include flags request it). */
    pubnub_channel_metadata_t channel;
    /** Membership status label. */
    pubnub_string_view_t status;
    /** Membership type label. */
    pubnub_string_view_t type;
    /** User-defined custom data object for this membership.
     *  Walk with serial->object_get(). NULL when absent.
     *  Valid until pubnub_future_release(). */
    const struct pubnub_json_value* custom;
    /** ISO 8601 last-updated timestamp. */
    pubnub_string_view_t updated;
    /** Entity tag for conditional updates. */
    pubnub_string_view_t etag;
} pubnub_membership_t;

/**
 * @brief Member result (channel-to-UUID relationship).
 *
 * Returned when querying which UUIDs belong to a channel.
 * The embedded @c uuid struct may be partially populated
 * depending on include flags.
 */
typedef struct pubnub_member {
    /** UUID metadata (partial unless include flags request it). */
    pubnub_uuid_metadata_t uuid;
    /** Member status label. */
    pubnub_string_view_t status;
    /** Member type label. */
    pubnub_string_view_t type;
    /** User-defined custom data object for this member.
     *  Walk with serial->object_get(). NULL when absent.
     *  Valid until pubnub_future_release(). */
    const struct pubnub_json_value* custom;
    /** ISO 8601 last-updated timestamp. */
    pubnub_string_view_t updated;
    /** Entity tag for conditional updates. */
    pubnub_string_view_t etag;
} pubnub_member_t;

/**
 * @brief Pagination and count metadata for list operations.
 *
 * Returned by paginated App Context operations. Use @c next and
 * @c prev cursors for forward/backward navigation.
 */
typedef struct pubnub_app_context_page {
    /**
     * @brief Total object count (server-side).
     *
     * Only meaningful when @c PUBNUB_APP_CONTEXT_INCLUDE_TOTAL_COUNT was set in
     * the request include bitmask. Zero otherwise.
     */
    uint32_t total_count;
    /** Cursor for the next page (pass as @c start on next call). */
    pubnub_string_view_t next;
    /** Cursor for the previous page (pass as @c end on next call). */
    pubnub_string_view_t prev;
    /** Number of items in the current page (iteration bound). */
    uint32_t count;
} pubnub_app_context_page_t;

/**
 * @brief Input descriptor for set/remove membership operations.
 *
 * Used in the @c set and @c remove arrays of
 * @c pubnub_set_memberships_opts_t.
 */
typedef struct pubnub_membership_input {
    /** Channel identifier (@b required, NUL-terminated). */
    const char* channel_id;
    /** Optional status label (NULL to omit). */
    const char* status;
    /** Optional type label (NULL to omit). */
    const char* type;
    /** Optional raw JSON custom data (NULL to omit). */
    const char* custom;
    /**
     * @brief Length of @c custom in bytes.
     *
     * @b Default: @c 0 means "call strlen" when @c custom is non-NULL.
     */
    size_t custom_len;

    /**
     * @brief Custom data as a JSON value tree (ownership transfers).
     *
     * When non-NULL, takes precedence over @c custom. The SDK
     * consumes and frees this tree during the API call — do not
     * access or free it afterward. Build with helpers from
     * @c json_macros.h.
     *
     * @warning Unlike @c pubnub_publish, @c pubnub_signal, and
     *          @c pubnub_set_state which @b borrow the JSON tree,
     *          App Context @b transfers ownership here. Do not
     *          access or free the tree after the call returns.
     *
     * @attention Setting both @c custom and @c custom_value is an
     *            error (@c PUBNUB_ERR_INVALID_ARGUMENT).
     */
    struct pubnub_json_value* custom_value;
} pubnub_membership_input_t;

/**
 * @brief Input descriptor for set/remove member operations.
 *
 * Used in the @c set and @c remove arrays of
 * @c pubnub_set_channel_members_opts_t.
 */
typedef struct pubnub_member_input {
    /** UUID identifier (@b required, NUL-terminated). */
    const char* uuid_id;
    /** Optional status label (NULL to omit). */
    const char* status;
    /** Optional type label (NULL to omit). */
    const char* type;
    /** Optional raw JSON custom data (NULL to omit). */
    const char* custom;
    /**
     * @brief Length of @c custom in bytes.
     *
     * @b Default: @c 0 means "call strlen" when @c custom is non-NULL.
     */
    size_t custom_len;

    /**
     * @brief Custom data as a JSON value tree (ownership transfers).
     *
     * When non-NULL, takes precedence over @c custom. The SDK
     * consumes and frees this tree during the API call — do not
     * access or free it afterward. Build with helpers from
     * @c json_macros.h.
     *
     * @warning Unlike @c pubnub_publish, @c pubnub_signal, and
     *          @c pubnub_set_state which @b borrow the JSON tree,
     *          App Context @b transfers ownership here. Do not
     *          access or free the tree after the call returns.
     *
     * @attention Setting both @c custom and @c custom_value is an
     *            error (@c PUBNUB_ERR_INVALID_ARGUMENT).
     */
    struct pubnub_json_value* custom_value;
} pubnub_member_input_t;

/**
 * @brief Options for @c pubnub_get_all_uuid_metadata.
 *
 * Designated-initializer friendly: every zero value resolves to a
 * documented default. Initialize with
 * @c PUBNUB_GET_ALL_UUID_METADATA_OPTS_INIT.
 *
 * @see pubnub_get_all_uuid_metadata
 */
typedef struct pubnub_get_all_uuid_metadata_opts {
    /**
     * @brief Include bitmask (bitwise OR of @c PUBNUB_APP_CONTEXT_INCLUDE_*).
     *
     * @b Default: @c 0 (no additional includes).
     */
    uint32_t include;

    /**
     * @brief Maximum items per page.
     *
     * @b Default: @c 0 (server default, typically 100).
     */
    uint32_t limit;

    /**
     * @brief Next-page cursor (NUL-terminated, borrowed).
     *
     * @b Default: @c NULL (start from beginning).
     */
    const char* start;

    /**
     * @brief Previous-page cursor (NUL-terminated, borrowed).
     *
     * @b Default: @c NULL.
     */
    const char* end;

    /**
     * @brief Filter expression (NUL-terminated, borrowed).
     *
     * Passed verbatim as the @c filter query parameter.
     *
     * @b Default: @c NULL (no filter).
     */
    const char* filter;

    /**
     * @brief Sort expression (NUL-terminated, borrowed).
     *
     * Comma-separated field:direction pairs (e.g.,
     * "name:asc,updated:desc"). Passed verbatim as the @c sort query
     * parameter.
     *
     * @b Default: @c NULL (server default order).
     */
    const char* sort;

    /**
     * @brief Per-request timeout override in milliseconds.
     *
     * @b Default: @c 0 (use context-level timeout).
     */
    uint32_t timeout_ms;
} pubnub_get_all_uuid_metadata_opts_t;

/** @brief Zero-initializer for @c pubnub_get_all_uuid_metadata_opts_t. */
#define PUBNUB_GET_ALL_UUID_METADATA_OPTS_INIT {0}

/**
 * @brief Options for @c pubnub_get_uuid_metadata.
 *
 * Initialize with @c PUBNUB_GET_UUID_METADATA_OPTS_INIT.
 *
 * @see pubnub_get_uuid_metadata
 */
typedef struct pubnub_get_uuid_metadata_opts {
    /**
     * @brief UUID to query (@b required, NUL-terminated, borrowed).
     *
     * Pass NULL to use the context's configured @c user_id.
     */
    const char* uuid;

    /**
     * @brief Include bitmask (bitwise OR of @c PUBNUB_APP_CONTEXT_INCLUDE_*).
     *
     * @b Default: @c PUBNUB_APP_CONTEXT_INCLUDE_CUSTOM (custom fields included,
     * matching cross-SDK default for single-entity operations).
     */
    uint32_t include;

    /**
     * @brief Per-request timeout override in milliseconds.
     *
     * @b Default: @c 0 (use context-level timeout).
     */
    uint32_t timeout_ms;
} pubnub_get_uuid_metadata_opts_t;

/** @brief Zero-initializer for @c pubnub_get_uuid_metadata_opts_t. */
#define PUBNUB_GET_UUID_METADATA_OPTS_INIT \
    {.include = PUBNUB_APP_CONTEXT_INCLUDE_CUSTOM}

/**
 * @brief Options for @c pubnub_set_uuid_metadata.
 *
 * Initialize with @c PUBNUB_SET_UUID_METADATA_OPTS_INIT.
 *
 * @see pubnub_set_uuid_metadata
 */
typedef struct pubnub_set_uuid_metadata_opts {
    /**
     * @brief UUID to set (@b required, NUL-terminated, borrowed).
     *
     * Pass NULL to use the context's configured @c user_id.
     */
    const char* uuid;

    /** Display name (NULL to omit / leave unchanged). */
    const char* name;

    /** External identifier (NULL to omit). */
    const char* external_id;

    /** Profile URL (NULL to omit). */
    const char* profile_url;

    /** Email address (NULL to omit). */
    const char* email;

    /** Type label (NULL to omit). */
    const char* type;

    /** Status label (NULL to omit). */
    const char* status;

    /**
     * @brief Raw JSON custom data (NULL to omit).
     *
     * Must be a valid JSON object when non-NULL.
     */
    const char* custom;

    /**
     * @brief Length of @c custom in bytes.
     *
     * @b Default: @c 0 means "call strlen" when @c custom is non-NULL.
     */
    size_t custom_len;

    /**
     * @brief Custom data as a JSON value tree (ownership transfers).
     *
     * When non-NULL, takes precedence over @c custom. The SDK
     * consumes and frees this tree during the API call — do not
     * access or free it afterward. Build with helpers from
     * @c json_macros.h.
     *
     * @warning Unlike @c pubnub_publish, @c pubnub_signal, and
     *          @c pubnub_set_state which @b borrow the JSON tree,
     *          App Context @b transfers ownership here. Do not
     *          access or free the tree after the call returns.
     *
     * @attention Setting both @c custom and @c custom_value is an
     *            error (@c PUBNUB_ERR_INVALID_ARGUMENT).
     */
    struct pubnub_json_value* custom_value;

    /**
     * @brief Include bitmask (bitwise OR of @c PUBNUB_APP_CONTEXT_INCLUDE_*).
     *
     * @b Default: @c PUBNUB_APP_CONTEXT_INCLUDE_CUSTOM (custom fields included,
     * matching cross-SDK default for single-entity operations).
     */
    uint32_t include;

    /**
     * @brief ETag for conditional update (NUL-terminated, borrowed).
     *
     * When non-NULL, sent as an If-Match header for optimistic
     * concurrency control. The server rejects the update if the
     * current ETag does not match. Must remain valid until the
     * function returns.
     *
     * @b Default: @c NULL (unconditional update).
     */
    const char* if_match;

    /**
     * @brief Per-request timeout override in milliseconds.
     *
     * @b Default: @c 0 (use context-level timeout).
     */
    uint32_t timeout_ms;
} pubnub_set_uuid_metadata_opts_t;

/** @brief Zero-initializer for @c pubnub_set_uuid_metadata_opts_t. */
#define PUBNUB_SET_UUID_METADATA_OPTS_INIT \
    {.include = PUBNUB_APP_CONTEXT_INCLUDE_CUSTOM}

/**
 * @brief Options for @c pubnub_remove_uuid_metadata.
 *
 * Initialize with @c PUBNUB_REMOVE_UUID_METADATA_OPTS_INIT.
 *
 * @see pubnub_remove_uuid_metadata
 */
typedef struct pubnub_remove_uuid_metadata_opts {
    /**
     * @brief UUID to remove (@b required, NUL-terminated, borrowed).
     *
     * Pass NULL to use the context's configured @c user_id.
     */
    const char* uuid;

    /**
     * @brief Per-request timeout override in milliseconds.
     *
     * @b Default: @c 0 (use context-level timeout).
     */
    uint32_t timeout_ms;
} pubnub_remove_uuid_metadata_opts_t;

/** @brief Zero-initializer for @c pubnub_remove_uuid_metadata_opts_t. */
#define PUBNUB_REMOVE_UUID_METADATA_OPTS_INIT {0}

/**
 * @brief Options for @c pubnub_get_all_channel_metadata.
 *
 * Initialize with @c PUBNUB_GET_ALL_CHANNEL_METADATA_OPTS_INIT.
 *
 * @see pubnub_get_all_channel_metadata
 */
typedef struct pubnub_get_all_channel_metadata_opts {
    /**
     * @brief Include bitmask (bitwise OR of @c PUBNUB_APP_CONTEXT_INCLUDE_*).
     *
     * @b Default: @c 0 (no additional includes).
     */
    uint32_t include;

    /**
     * @brief Maximum items per page.
     *
     * @b Default: @c 0 (server default, typically 100).
     */
    uint32_t limit;

    /**
     * @brief Next-page cursor (NUL-terminated, borrowed).
     *
     * @b Default: @c NULL (start from beginning).
     */
    const char* start;

    /**
     * @brief Previous-page cursor (NUL-terminated, borrowed).
     *
     * @b Default: @c NULL.
     */
    const char* end;

    /**
     * @brief Filter expression (NUL-terminated, borrowed).
     *
     * @b Default: @c NULL (no filter).
     */
    const char* filter;

    /**
     * @brief Sort expression (NUL-terminated, borrowed).
     *
     * @b Default: @c NULL (server default order).
     */
    const char* sort;

    /**
     * @brief Per-request timeout override in milliseconds.
     *
     * @b Default: @c 0 (use context-level timeout).
     */
    uint32_t timeout_ms;
} pubnub_get_all_channel_metadata_opts_t;

/** @brief Zero-initializer for @c pubnub_get_all_channel_metadata_opts_t. */
#define PUBNUB_GET_ALL_CHANNEL_METADATA_OPTS_INIT {0}

/**
 * @brief Options for @c pubnub_get_channel_metadata.
 *
 * Initialize with @c PUBNUB_GET_CHANNEL_METADATA_OPTS_INIT.
 *
 * @see pubnub_get_channel_metadata
 */
typedef struct pubnub_get_channel_metadata_opts {
    /**
     * @brief Channel to query (@b required, NUL-terminated, borrowed).
     */
    const char* channel;

    /**
     * @brief Include bitmask (bitwise OR of @c PUBNUB_APP_CONTEXT_INCLUDE_*).
     *
     * @b Default: @c PUBNUB_APP_CONTEXT_INCLUDE_CUSTOM (custom fields included,
     * matching cross-SDK default for single-entity operations).
     */
    uint32_t include;

    /**
     * @brief Per-request timeout override in milliseconds.
     *
     * @b Default: @c 0 (use context-level timeout).
     */
    uint32_t timeout_ms;
} pubnub_get_channel_metadata_opts_t;

/** @brief Zero-initializer for @c pubnub_get_channel_metadata_opts_t. */
#define PUBNUB_GET_CHANNEL_METADATA_OPTS_INIT \
    {.include = PUBNUB_APP_CONTEXT_INCLUDE_CUSTOM}

/**
 * @brief Options for @c pubnub_set_channel_metadata.
 *
 * Initialize with @c PUBNUB_SET_CHANNEL_METADATA_OPTS_INIT.
 *
 * @see pubnub_set_channel_metadata
 */
typedef struct pubnub_set_channel_metadata_opts {
    /**
     * @brief Channel to update (@b required, NUL-terminated, borrowed).
     */
    const char* channel;

    /** Display name (NULL to omit / leave unchanged). */
    const char* name;

    /** Description text (NULL to omit). */
    const char* description;

    /** Type label (NULL to omit). */
    const char* type;

    /** Status label (NULL to omit). */
    const char* status;

    /**
     * @brief Raw JSON custom data (NULL to omit).
     *
     * Must be a valid JSON object when non-NULL.
     */
    const char* custom;

    /**
     * @brief Length of @c custom in bytes.
     *
     * @b Default: @c 0 means "call strlen" when @c custom is non-NULL.
     */
    size_t custom_len;

    /**
     * @brief Custom data as a JSON value tree (ownership transfers).
     *
     * When non-NULL, takes precedence over @c custom. The SDK
     * consumes and frees this tree during the API call — do not
     * access or free it afterward. Build with helpers from
     * @c json_macros.h.
     *
     * @warning Unlike @c pubnub_publish, @c pubnub_signal, and
     *          @c pubnub_set_state which @b borrow the JSON tree,
     *          App Context @b transfers ownership here. Do not
     *          access or free the tree after the call returns.
     *
     * @attention Setting both @c custom and @c custom_value is an
     *            error (@c PUBNUB_ERR_INVALID_ARGUMENT).
     */
    struct pubnub_json_value* custom_value;

    /**
     * @brief Include bitmask (bitwise OR of @c PUBNUB_APP_CONTEXT_INCLUDE_*).
     *
     * @b Default: @c PUBNUB_APP_CONTEXT_INCLUDE_CUSTOM (custom fields included,
     * matching cross-SDK default for single-entity operations).
     */
    uint32_t include;

    /**
     * @brief ETag for conditional update (NUL-terminated, borrowed).
     *
     * Must remain valid until the function returns.
     *
     * @b Default: @c NULL (unconditional update).
     */
    const char* if_match;

    /**
     * @brief Per-request timeout override in milliseconds.
     *
     * @b Default: @c 0 (use context-level timeout).
     */
    uint32_t timeout_ms;
} pubnub_set_channel_metadata_opts_t;

/** @brief Zero-initializer for @c pubnub_set_channel_metadata_opts_t. */
#define PUBNUB_SET_CHANNEL_METADATA_OPTS_INIT \
    {.include = PUBNUB_APP_CONTEXT_INCLUDE_CUSTOM}

/**
 * @brief Options for @c pubnub_remove_channel_metadata.
 *
 * Initialize with @c PUBNUB_REMOVE_CHANNEL_METADATA_OPTS_INIT.
 *
 * @see pubnub_remove_channel_metadata
 */
typedef struct pubnub_remove_channel_metadata_opts {
    /**
     * @brief Channel to remove (@b required, NUL-terminated, borrowed).
     */
    const char* channel;

    /**
     * @brief Per-request timeout override in milliseconds.
     *
     * @b Default: @c 0 (use context-level timeout).
     */
    uint32_t timeout_ms;
} pubnub_remove_channel_metadata_opts_t;

/** @brief Zero-initializer for @c pubnub_remove_channel_metadata_opts_t. */
#define PUBNUB_REMOVE_CHANNEL_METADATA_OPTS_INIT {0}

/**
 * @brief Options for @c pubnub_get_memberships.
 *
 * Initialize with @c PUBNUB_GET_MEMBERSHIPS_OPTS_INIT.
 *
 * @see pubnub_get_memberships
 */
typedef struct pubnub_get_memberships_opts {
    /**
     * @brief UUID whose memberships to query (NUL-terminated, borrowed).
     *
     * Pass NULL to use the context's configured @c user_id.
     */
    const char* uuid;

    /**
     * @brief Include bitmask (bitwise OR of @c PUBNUB_APP_CONTEXT_INCLUDE_*).
     *
     * @b Default: @c 0.
     */
    uint32_t include;

    /**
     * @brief Maximum items per page.
     *
     * @b Default: @c 0 (server default, typically 100).
     */
    uint32_t limit;

    /**
     * @brief Next-page cursor (NUL-terminated, borrowed).
     *
     * @b Default: @c NULL.
     */
    const char* start;

    /**
     * @brief Previous-page cursor (NUL-terminated, borrowed).
     *
     * @b Default: @c NULL.
     */
    const char* end;

    /**
     * @brief Filter expression (NUL-terminated, borrowed).
     *
     * @b Default: @c NULL (no filter).
     */
    const char* filter;

    /**
     * @brief Sort expression (NUL-terminated, borrowed).
     *
     * @b Default: @c NULL (server default order).
     */
    const char* sort;

    /**
     * @brief Per-request timeout override in milliseconds.
     *
     * @b Default: @c 0 (use context-level timeout).
     */
    uint32_t timeout_ms;
} pubnub_get_memberships_opts_t;

/** @brief Zero-initializer for @c pubnub_get_memberships_opts_t. */
#define PUBNUB_GET_MEMBERSHIPS_OPTS_INIT {0}

/**
 * @brief Options for @c pubnub_set_memberships.
 *
 * Initialize with @c PUBNUB_SET_MEMBERSHIPS_OPTS_INIT.
 *
 * @see pubnub_set_memberships
 */
typedef struct pubnub_set_memberships_opts {
    /**
     * @brief UUID whose memberships to modify (NUL-terminated, borrowed).
     *
     * Pass NULL to use the context's configured @c user_id.
     */
    const char* uuid;

    /**
     * @brief Array of memberships to add or update (borrowed).
     *
     * @b Default: @c NULL (no additions).
     */
    const pubnub_membership_input_t* set;

    /**
     * @brief Number of elements in the @c set array.
     *
     * @b Default: @c 0.
     */
    size_t set_count;

    /**
     * @brief Array of memberships to remove (borrowed).
     *
     * Only the @c channel_id field is used for removal.
     *
     * @b Default: @c NULL (no removals).
     */
    const pubnub_membership_input_t* remove;

    /**
     * @brief Number of elements in the @c remove array.
     *
     * @b Default: @c 0.
     */
    size_t remove_count;

    /**
     * @brief Include bitmask (bitwise OR of @c PUBNUB_APP_CONTEXT_INCLUDE_*).
     *
     * @b Default: @c 0.
     */
    uint32_t include;

    /**
     * @brief Maximum items per page in the response.
     *
     * @b Default: @c 0 (server default, typically 100).
     */
    uint32_t limit;

    /**
     * @brief Next-page cursor (NUL-terminated, borrowed).
     *
     * @b Default: @c NULL.
     */
    const char* start;

    /**
     * @brief Previous-page cursor (NUL-terminated, borrowed).
     *
     * @b Default: @c NULL.
     */
    const char* end;

    /**
     * @brief Filter expression (NUL-terminated, borrowed).
     *
     * @b Default: @c NULL.
     */
    const char* filter;

    /**
     * @brief Sort expression (NUL-terminated, borrowed).
     *
     * @b Default: @c NULL.
     */
    const char* sort;

    /**
     * @brief Per-request timeout override in milliseconds.
     *
     * @b Default: @c 0 (use context-level timeout).
     */
    uint32_t timeout_ms;
} pubnub_set_memberships_opts_t;

/** @brief Zero-initializer for @c pubnub_set_memberships_opts_t. */
#define PUBNUB_SET_MEMBERSHIPS_OPTS_INIT {0}

/**
 * @brief Options for @c pubnub_get_channel_members.
 *
 * Initialize with @c PUBNUB_GET_CHANNEL_MEMBERS_OPTS_INIT.
 *
 * @see pubnub_get_channel_members
 */
typedef struct pubnub_get_channel_members_opts {
    /**
     * @brief Channel whose members to query (@b required,
     *        NUL-terminated, borrowed).
     */
    const char* channel;

    /**
     * @brief Include bitmask (bitwise OR of @c PUBNUB_APP_CONTEXT_INCLUDE_*).
     *
     * @b Default: @c 0.
     */
    uint32_t include;

    /**
     * @brief Maximum items per page.
     *
     * @b Default: @c 0 (server default, typically 100).
     */
    uint32_t limit;

    /**
     * @brief Next-page cursor (NUL-terminated, borrowed).
     *
     * @b Default: @c NULL.
     */
    const char* start;

    /**
     * @brief Previous-page cursor (NUL-terminated, borrowed).
     *
     * @b Default: @c NULL.
     */
    const char* end;

    /**
     * @brief Filter expression (NUL-terminated, borrowed).
     *
     * @b Default: @c NULL.
     */
    const char* filter;

    /**
     * @brief Sort expression (NUL-terminated, borrowed).
     *
     * @b Default: @c NULL.
     */
    const char* sort;

    /**
     * @brief Per-request timeout override in milliseconds.
     *
     * @b Default: @c 0 (use context-level timeout).
     */
    uint32_t timeout_ms;
} pubnub_get_channel_members_opts_t;

/** @brief Zero-initializer for @c pubnub_get_channel_members_opts_t. */
#define PUBNUB_GET_CHANNEL_MEMBERS_OPTS_INIT {0}

/**
 * @brief Options for @c pubnub_set_channel_members.
 *
 * Initialize with @c PUBNUB_SET_CHANNEL_MEMBERS_OPTS_INIT.
 *
 * @see pubnub_set_channel_members
 */
typedef struct pubnub_set_channel_members_opts {
    /**
     * @brief Channel whose members to modify (@b required,
     *        NUL-terminated, borrowed).
     */
    const char* channel;

    /**
     * @brief Array of members to add or update (borrowed).
     *
     * @b Default: @c NULL (no additions).
     */
    const pubnub_member_input_t* set;

    /**
     * @brief Number of elements in the @c set array.
     *
     * @b Default: @c 0.
     */
    size_t set_count;

    /**
     * @brief Array of members to remove (borrowed).
     *
     * Only the @c uuid_id field is used for removal.
     *
     * @b Default: @c NULL (no removals).
     */
    const pubnub_member_input_t* remove;

    /**
     * @brief Number of elements in the @c remove array.
     *
     * @b Default: @c 0.
     */
    size_t remove_count;

    /**
     * @brief Include bitmask (bitwise OR of @c PUBNUB_APP_CONTEXT_INCLUDE_*).
     *
     * @b Default: @c 0.
     */
    uint32_t include;

    /**
     * @brief Maximum items per page in the response.
     *
     * @b Default: @c 0 (server default, typically 100).
     */
    uint32_t limit;

    /**
     * @brief Next-page cursor (NUL-terminated, borrowed).
     *
     * @b Default: @c NULL.
     */
    const char* start;

    /**
     * @brief Previous-page cursor (NUL-terminated, borrowed).
     *
     * @b Default: @c NULL.
     */
    const char* end;

    /**
     * @brief Filter expression (NUL-terminated, borrowed).
     *
     * @b Default: @c NULL.
     */
    const char* filter;

    /**
     * @brief Sort expression (NUL-terminated, borrowed).
     *
     * @b Default: @c NULL.
     */
    const char* sort;

    /**
     * @brief Per-request timeout override in milliseconds.
     *
     * @b Default: @c 0 (use context-level timeout).
     */
    uint32_t timeout_ms;
} pubnub_set_channel_members_opts_t;

/** @brief Zero-initializer for @c pubnub_set_channel_members_opts_t. */
#define PUBNUB_SET_CHANNEL_MEMBERS_OPTS_INIT {0}

/**
 * @brief Retrieve metadata for all UUID objects.
 *
 * @par Example (cooperative polling)
 * @code
 *   pubnub_get_all_uuid_metadata_opts_t opts =
 *       PUBNUB_GET_ALL_UUID_METADATA_OPTS_INIT;
 *   opts.include = PUBNUB_APP_CONTEXT_INCLUDE_CUSTOM |
 * PUBNUB_APP_CONTEXT_INCLUDE_TOTAL_COUNT; opts.limit   = 10;
 *
 *   pubnub_future_t fut = pubnub_get_all_uuid_metadata(ctx, &opts);
 *   while (!pubnub_future_is_ready(fut)) {
 *       pubnub_process(ctx);
 *   }
 *
 *   if (PUBNUB_OK == pubnub_future_status(fut)) {
 *       pubnub_app_context_page_t page =
 *           pubnub_get_all_uuid_metadata_result(fut);
 *       for (uint32_t i = 0; i < page.count; ++i) {
 *           pubnub_uuid_metadata_t m =
 *               pubnub_get_all_uuid_metadata_result_uuid_at(fut, i);
 *           printf("uuid: %.*s\n", (int)m.id.len, m.id.ptr);
 *       }
 *   }
 *   pubnub_future_release(fut);
 * @endcode
 *
 * @param ctx  Initialized context (borrowed).
 * @param opts Options struct (borrowed). Pass NULL for all defaults.
 * @return Future handle. Release via @c pubnub_future_release.
 *
 * @see pubnub_get_all_uuid_metadata_result
 * @see pubnub_future_release
 * @see pubnub_get_all_uuid_metadata_opts_t
 * @see pubnub_context_t
 * @see pubnub_future_t
 */
PUBNUB_API pubnub_future_t
pubnub_get_all_uuid_metadata(pubnub_context_t*                          ctx,
                             const pubnub_get_all_uuid_metadata_opts_t* opts);

/**
 * @brief Page result for @c pubnub_get_all_uuid_metadata.
 *
 * @param future Completed future from @c pubnub_get_all_uuid_metadata.
 * @return Page metadata. Zero-initialized if the future is not ready
 *         or carries an error.
 * @note Returns zero-initialized page (count=0) when the future is
 *       not ready or invalid. Check @c pubnub_future_status to
 *       distinguish empty results from errors.
 */
PUBNUB_API pubnub_app_context_page_t
pubnub_get_all_uuid_metadata_result(pubnub_future_t future);

/**
 * @brief Retrieve a single UUID metadata item by index.
 *
 * @param future Completed future from @c pubnub_get_all_uuid_metadata.
 * @param index  Zero-based index (must be < page.count).
 * @return UUID metadata. Zero-initialized if index is out of range or
 *         the future is not ready.
 */
PUBNUB_API pubnub_uuid_metadata_t
pubnub_get_all_uuid_metadata_result_uuid_at(pubnub_future_t future, size_t index);

/**
 * @brief Retrieve metadata for a single UUID.
 *
 * @param ctx  Initialized context (borrowed).
 * @param opts Options struct (borrowed). Pass NULL to query the
 *             context's own @c user_id with no includes.
 * @return Future handle. Release via @c pubnub_future_release.
 *
 * @see pubnub_get_uuid_metadata_result
 * @see pubnub_future_release
 * @see pubnub_get_uuid_metadata_opts_t
 * @see pubnub_context_t
 * @see pubnub_future_t
 */
PUBNUB_API pubnub_future_t
pubnub_get_uuid_metadata(pubnub_context_t*                      ctx,
                         const pubnub_get_uuid_metadata_opts_t* opts);

/**
 * @brief Result for @c pubnub_get_uuid_metadata.
 *
 * @param future Completed future from @c pubnub_get_uuid_metadata.
 * @return UUID metadata. Zero-initialized on error.
 */
PUBNUB_API pubnub_uuid_metadata_t pubnub_get_uuid_metadata_result(pubnub_future_t future);

/**
 * @brief Create or update metadata for a UUID.
 *
 * This is a partial update (PATCH): only non-NULL fields in opts
 * are sent to the server. Existing fields not included in the
 * request are preserved.
 *
 * @param ctx  Initialized context (borrowed).
 * @param opts Options struct (borrowed, required).
 * @return Future handle. Release via @c pubnub_future_release.
 *
 * @see pubnub_set_uuid_metadata_result
 * @see pubnub_future_release
 * @see pubnub_set_uuid_metadata_opts_t
 * @see pubnub_context_t
 * @see pubnub_future_t
 */
PUBNUB_API pubnub_future_t
pubnub_set_uuid_metadata(pubnub_context_t*                      ctx,
                         const pubnub_set_uuid_metadata_opts_t* opts);

/**
 * @brief Result for @c pubnub_set_uuid_metadata.
 *
 * @param future Completed future from @c pubnub_set_uuid_metadata.
 * @return Updated UUID metadata. Zero-initialized on error.
 */
PUBNUB_API pubnub_uuid_metadata_t pubnub_set_uuid_metadata_result(pubnub_future_t future);

/**
 * @brief Remove metadata for a UUID.
 *
 * @param ctx  Initialized context (borrowed).
 * @param opts Options struct (borrowed). Pass NULL to remove the
 *             context's own @c user_id.
 * @return Future handle. Release via @c pubnub_future_release.
 *
 * @see pubnub_future_release
 * @see pubnub_remove_uuid_metadata_opts_t
 * @see pubnub_context_t
 * @see pubnub_future_t
 */
PUBNUB_API pubnub_future_t
pubnub_remove_uuid_metadata(pubnub_context_t*                         ctx,
                            const pubnub_remove_uuid_metadata_opts_t* opts);

/**
 * @brief Retrieve metadata for all channel objects.
 *
 * @param ctx  Initialized context (borrowed).
 * @param opts Options struct (borrowed). Pass NULL for all defaults.
 * @return Future handle. Release via @c pubnub_future_release.
 *
 * @see pubnub_get_all_channel_metadata_result
 * @see pubnub_future_release
 * @see pubnub_get_all_channel_metadata_opts_t
 * @see pubnub_context_t
 * @see pubnub_future_t
 */
PUBNUB_API pubnub_future_t pubnub_get_all_channel_metadata(
    pubnub_context_t*                             ctx,
    const pubnub_get_all_channel_metadata_opts_t* opts);

/**
 * @brief Page result for @c pubnub_get_all_channel_metadata.
 *
 * @param future Completed future from
 *               @c pubnub_get_all_channel_metadata.
 * @return Page metadata. Zero-initialized on error.
 * @note Returns zero-initialized page (count=0) when the future is
 *       not ready or invalid. Check @c pubnub_future_status to
 *       distinguish empty results from errors.
 */
PUBNUB_API pubnub_app_context_page_t
pubnub_get_all_channel_metadata_result(pubnub_future_t future);

/**
 * @brief Retrieve a single channel metadata item by index.
 *
 * @param future Completed future from
 *               @c pubnub_get_all_channel_metadata.
 * @param index  Zero-based index (must be < page.count).
 * @return Channel metadata. Zero-initialized if out of range.
 */
PUBNUB_API pubnub_channel_metadata_t
pubnub_get_all_channel_metadata_result_channel_at(pubnub_future_t future,
                                                  size_t          index);

/**
 * @brief Retrieve metadata for a single channel.
 *
 * @param ctx  Initialized context (borrowed).
 * @param opts Options struct (borrowed, required).
 * @return Future handle. Release via @c pubnub_future_release.
 *
 * @see pubnub_get_channel_metadata_result
 * @see pubnub_future_release
 * @see pubnub_get_channel_metadata_opts_t
 * @see pubnub_context_t
 * @see pubnub_future_t
 */
PUBNUB_API pubnub_future_t
pubnub_get_channel_metadata(pubnub_context_t*                         ctx,
                            const pubnub_get_channel_metadata_opts_t* opts);

/**
 * @brief Result for @c pubnub_get_channel_metadata.
 *
 * @param future Completed future from
 *               @c pubnub_get_channel_metadata.
 * @return Channel metadata. Zero-initialized on error.
 */
PUBNUB_API pubnub_channel_metadata_t
pubnub_get_channel_metadata_result(pubnub_future_t future);

/**
 * @brief Create or update metadata for a channel.
 *
 * Partial update (PATCH): only non-NULL fields are sent.
 *
 * @param ctx  Initialized context (borrowed).
 * @param opts Options struct (borrowed, required).
 * @return Future handle. Release via @c pubnub_future_release.
 *
 * @see pubnub_set_channel_metadata_result
 * @see pubnub_future_release
 * @see pubnub_set_channel_metadata_opts_t
 * @see pubnub_context_t
 * @see pubnub_future_t
 */
PUBNUB_API pubnub_future_t
pubnub_set_channel_metadata(pubnub_context_t*                         ctx,
                            const pubnub_set_channel_metadata_opts_t* opts);

/**
 * @brief Result for @c pubnub_set_channel_metadata.
 *
 * @param future Completed future from
 *               @c pubnub_set_channel_metadata.
 * @return Updated channel metadata. Zero-initialized on error.
 */
PUBNUB_API pubnub_channel_metadata_t
pubnub_set_channel_metadata_result(pubnub_future_t future);

/**
 * @brief Remove metadata for a channel.
 *
 * @param ctx  Initialized context (borrowed).
 * @param opts Options struct (borrowed, required).
 * @return Future handle. Release via @c pubnub_future_release.
 *
 * @see pubnub_future_release
 * @see pubnub_remove_channel_metadata_opts_t
 * @see pubnub_context_t
 * @see pubnub_future_t
 */
PUBNUB_API pubnub_future_t pubnub_remove_channel_metadata(
    pubnub_context_t*                            ctx,
    const pubnub_remove_channel_metadata_opts_t* opts);

/**
 * @brief Retrieve memberships for a UUID.
 *
 * Returns which channels the specified UUID is a member of.
 *
 * @param ctx  Initialized context (borrowed).
 * @param opts Options struct (borrowed). Pass NULL to query the
 *             context's own @c user_id with defaults.
 * @return Future handle. Release via @c pubnub_future_release.
 *
 * @see pubnub_get_memberships_result
 * @see pubnub_future_release
 * @see pubnub_get_memberships_opts_t
 * @see pubnub_context_t
 * @see pubnub_future_t
 */
PUBNUB_API pubnub_future_t
pubnub_get_memberships(pubnub_context_t*                    ctx,
                       const pubnub_get_memberships_opts_t* opts);

/**
 * @brief Page result for @c pubnub_get_memberships.
 *
 * @param future Completed future from @c pubnub_get_memberships.
 * @return Page metadata. Zero-initialized on error.
 * @note Returns zero-initialized page (count=0) when the future is
 *       not ready or invalid. Check @c pubnub_future_status to
 *       distinguish empty results from errors.
 */
PUBNUB_API pubnub_app_context_page_t pubnub_get_memberships_result(pubnub_future_t future);

/**
 * @brief Retrieve a single membership item by index.
 *
 * @param future Completed future from @c pubnub_get_memberships.
 * @param index  Zero-based index (must be < page.count).
 * @return Membership data. Zero-initialized if out of range.
 */
PUBNUB_API pubnub_membership_t
pubnub_get_memberships_result_membership_at(pubnub_future_t future, size_t index);

/**
 * @brief Add, update, or remove memberships for a UUID.
 *
 * The request body contains @c set (add/update) and @c remove arrays.
 * Both may be used in a single call.
 *
 * @param ctx  Initialized context (borrowed).
 * @param opts Options struct (borrowed, required).
 * @return Future handle. Release via @c pubnub_future_release.
 * @retval PUBNUB_ERR_INVALID_ARGUMENT if @c set_count and
 *         @c remove_count are both zero (no-op call).
 *
 * @see pubnub_set_memberships_result
 * @see pubnub_future_release
 * @see pubnub_set_memberships_opts_t
 * @see pubnub_context_t
 * @see pubnub_future_t
 */
PUBNUB_API pubnub_future_t
pubnub_set_memberships(pubnub_context_t*                    ctx,
                       const pubnub_set_memberships_opts_t* opts);

/**
 * @brief Page result for @c pubnub_set_memberships.
 *
 * @param future Completed future from @c pubnub_set_memberships.
 * @return Page metadata. Zero-initialized on error.
 * @note Returns zero-initialized page (count=0) when the future is
 *       not ready or invalid. Check @c pubnub_future_status to
 *       distinguish empty results from errors.
 */
PUBNUB_API pubnub_app_context_page_t pubnub_set_memberships_result(pubnub_future_t future);

/**
 * @brief Retrieve a single membership item by index from a set result.
 *
 * @param future Completed future from @c pubnub_set_memberships.
 * @param index  Zero-based index (must be < page.count).
 * @return Membership data. Zero-initialized if out of range.
 */
PUBNUB_API pubnub_membership_t
pubnub_set_memberships_result_membership_at(pubnub_future_t future, size_t index);

/**
 * @brief Retrieve members of a channel.
 *
 * Returns which UUIDs are members of the specified channel.
 *
 * @param ctx  Initialized context (borrowed).
 * @param opts Options struct (borrowed, required).
 * @return Future handle. Release via @c pubnub_future_release.
 *
 * @see pubnub_get_channel_members_result
 * @see pubnub_future_release
 * @see pubnub_get_channel_members_opts_t
 * @see pubnub_context_t
 * @see pubnub_future_t
 */
PUBNUB_API pubnub_future_t
pubnub_get_channel_members(pubnub_context_t*                        ctx,
                           const pubnub_get_channel_members_opts_t* opts);

/**
 * @brief Page result for @c pubnub_get_channel_members.
 *
 * @param future Completed future from @c pubnub_get_channel_members.
 * @return Page metadata. Zero-initialized on error.
 * @note Returns zero-initialized page (count=0) when the future is
 *       not ready or invalid. Check @c pubnub_future_status to
 *       distinguish empty results from errors.
 */
PUBNUB_API pubnub_app_context_page_t
pubnub_get_channel_members_result(pubnub_future_t future);

/**
 * @brief Retrieve a single member item by index.
 *
 * @param future Completed future from @c pubnub_get_channel_members.
 * @param index  Zero-based index (must be < page.count).
 * @return Member data. Zero-initialized if out of range.
 */
PUBNUB_API pubnub_member_t
pubnub_get_channel_members_result_member_at(pubnub_future_t future, size_t index);

/**
 * @brief Add, update, or remove members of a channel.
 *
 * The request body contains @c set (add/update) and @c remove arrays.
 * Both may be used in a single call.
 *
 * @param ctx  Initialized context (borrowed).
 * @param opts Options struct (borrowed, required).
 * @return Future handle. Release via @c pubnub_future_release.
 * @retval PUBNUB_ERR_INVALID_ARGUMENT if @c set_count and
 *         @c remove_count are both zero (no-op call).
 *
 * @see pubnub_set_channel_members_result
 * @see pubnub_future_release
 * @see pubnub_set_channel_members_opts_t
 * @see pubnub_context_t
 * @see pubnub_future_t
 */
PUBNUB_API pubnub_future_t
pubnub_set_channel_members(pubnub_context_t*                        ctx,
                           const pubnub_set_channel_members_opts_t* opts);

/**
 * @brief Page result for @c pubnub_set_channel_members.
 *
 * @param future Completed future from @c pubnub_set_channel_members.
 * @return Page metadata. Zero-initialized on error.
 * @note Returns zero-initialized page (count=0) when the future is
 *       not ready or invalid. Check @c pubnub_future_status to
 *       distinguish empty results from errors.
 */
PUBNUB_API pubnub_app_context_page_t
pubnub_set_channel_members_result(pubnub_future_t future);

/**
 * @brief Retrieve a single member item by index from a set result.
 *
 * @param future Completed future from @c pubnub_set_channel_members.
 * @param index  Zero-based index (must be < page.count).
 * @return Member data. Zero-initialized if out of range.
 */
PUBNUB_API pubnub_member_t
pubnub_set_channel_members_result_member_at(pubnub_future_t future, size_t index);

#if PUBNUB_ENABLE_SUBSCRIBE

/** Forward declaration for the subscribe app context event struct. */
struct pubnub_subscribe_app_context_event;

/**
 * @brief Extract UUID metadata from a subscribe app context event.
 *
 * Call when event->object_type == PUBNUB_APP_CONTEXT_OBJECT_UUID.
 * Populates @p out by walking event->data via the serialization vtable.
 *
 * @param ctx   Context (for serialization provider access).
 * @param event App context event from
 *              @c pubnub_subscribe_event_app_context.
 * @param out   Receives the parsed UUID metadata.
 * @return PUBNUB_OK on success, PUBNUB_ERR_INVALID_ARGUMENT if any
 *         arg is NULL, PUBNUB_ERR_SERIALIZATION on provider failure.
 */
PUBNUB_API pubnub_res_t pubnub_subscribe_app_context_uuid_metadata(
    pubnub_context_t*                                ctx,
    const struct pubnub_subscribe_app_context_event* event,
    pubnub_uuid_metadata_t*                          out);

/**
 * @brief Extract channel metadata from a subscribe app context event.
 *
 * Call when event->object_type == PUBNUB_APP_CONTEXT_OBJECT_CHANNEL.
 * Populates @p out by walking event->data via the serialization vtable.
 *
 * @param ctx   Context (for serialization provider access).
 * @param event App context event from
 *              @c pubnub_subscribe_event_app_context.
 * @param out   Receives the parsed channel metadata.
 * @return PUBNUB_OK on success, PUBNUB_ERR_INVALID_ARGUMENT if any
 *         arg is NULL, PUBNUB_ERR_SERIALIZATION on provider failure.
 */
PUBNUB_API pubnub_res_t pubnub_subscribe_app_context_channel_metadata(
    pubnub_context_t*                                ctx,
    const struct pubnub_subscribe_app_context_event* event,
    pubnub_channel_metadata_t*                       out);

/**
 * @brief Extract membership from a subscribe app context event.
 *
 * Call when event->object_type == PUBNUB_APP_CONTEXT_OBJECT_MEMBERSHIP.
 * Populates @p out by walking event->data via the serialization vtable.
 *
 * @param ctx   Context (for serialization provider access).
 * @param event App context event from
 *              @c pubnub_subscribe_event_app_context.
 * @param out   Receives the parsed membership.
 * @return PUBNUB_OK on success, PUBNUB_ERR_INVALID_ARGUMENT if any
 *         arg is NULL, PUBNUB_ERR_SERIALIZATION on provider failure.
 */
PUBNUB_API pubnub_res_t pubnub_subscribe_app_context_membership(
    pubnub_context_t*                                ctx,
    const struct pubnub_subscribe_app_context_event* event,
    pubnub_membership_t*                             out);

#endif /* PUBNUB_ENABLE_SUBSCRIBE */

#ifdef __cplusplus
// clang-format off
}
// clang-format on
#endif

#endif /* PUBNUB_ENABLE_APP_CONTEXT */

#endif /* PUBNUB_FEATURE_APP_CONTEXT_H */
