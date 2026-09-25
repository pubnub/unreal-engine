/* Copyright (c) PubNub Inc. */
/* See LICENSE in the root directory of this source tree. */

#ifndef PUBNUB_FEATURE_ACCESS_H
#define PUBNUB_FEATURE_ACCESS_H

/**
 * @file features/access.h
 * @brief PubNub Access Manager (PAM) — server-side environments only.
 *
 * @warning **PAM requires a secret key embedded in the calling process.**
 *          Secret keys must NEVER reside in device firmware, mobile apps,
 *          or any code distributed to end-users. Exposure of a secret key
 *          gives an attacker unrestricted administrative access to your
 *          PubNub keyset.
 *
 * @par Intended environments
 * PAM is designed for **trusted server-side** processes (backend APIs,
 * cloud functions, server daemons) where the secret key can be stored
 * securely via environment variables or secrets managers.
 *
 * @par Embedded / IoT devices
 * Devices must NOT store or use the secret key. The correct pattern is:
 *  1. A server-side service calls @c pubnub_grant_token() to issue a
 *     short-lived, least-privilege token for the device.
 *  2. The device receives the token through a secure provisioning channel.
 *  3. The device calls @c pubnub_set_auth_token() — no secret key involved.
 *
 * The SDK enforces this at build time: @c PUBNUB_ENABLE_PAM is a
 * compile-time @c FATAL_ERROR on the @c embedded profile.
 */

#include "pubnub/config.h"

#if PUBNUB_ENABLE_PAM

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
 * @brief Access permission bitmask values.
 *
 * Values match the PubNub wire format so grant body builders can
 * use them directly without translation. Combine with bitwise OR.
 *
 * @code
 * uint32_t perms = PUBNUB_ACCESS_READ | PUBNUB_ACCESS_WRITE;
 * @endcode
 */
typedef enum pubnub_access_permission {
    /** Read messages and presence events. */
    PUBNUB_ACCESS_READ = 1,
    /** Publish messages. */
    PUBNUB_ACCESS_WRITE = 2,
    /** Add/remove channels in channel groups. */
    PUBNUB_ACCESS_MANAGE = 4,
    /** Delete messages from history. */
    PUBNUB_ACCESS_DELETE = 8,
    /** Create resources (Objects). */
    PUBNUB_ACCESS_CREATE = 16,
    /** Read resource metadata (Objects). */
    PUBNUB_ACCESS_GET = 32,
    /** Update resource metadata (Objects). */
    PUBNUB_ACCESS_UPDATE = 64,
    /** Join a channel (presence). */
    PUBNUB_ACCESS_JOIN = 128
} pubnub_access_permission_t;

/** A resource name paired with its permission bitmask. */
typedef struct pubnub_access_resource_permission {
    /**
     * @brief Resource name or regex pattern (@b required, @b borrowed,
     *        NUL-terminated).
     *
     * For exact resources, pass the literal name. For patterns, pass
     * the regex string (e.g., @c "^chat\\..*$").
     */
    const char* name;

    /** OR'd @c pubnub_access_permission_t bits. */
    uint32_t permissions;
} pubnub_access_resource_permission_t;

/**
 * @brief Options for @c pubnub_grant_token.
 *
 * Initialize with @c PUBNUB_GRANT_TOKEN_OPTS_INIT (zero-init
 * produces valid defaults for all optional fields).
 *
 * @note At least one resource or pattern permission must be
 *       specified; otherwise the request fails with
 *       @c PUBNUB_ERR_INVALID_ARGUMENT.
 *
 * @see pubnub_grant_token
 */
typedef struct pubnub_grant_token_opts {
    /**
     * @brief Token time-to-live in minutes (@b required).
     *
     * Valid range: 1 to 43200 (30 days). A value of 0 causes
     * @c PUBNUB_ERR_INVALID_ARGUMENT.
     */
    uint32_t ttl;

    /**
     * @brief Exact channel permissions array (@b optional, @b borrowed).
     *
     * @see pubnub_access_resource_permission_t
     */
    const pubnub_access_resource_permission_t* channels;

    /** Number of entries in @c channels. */
    size_t channel_count;

    /**
     * @brief Exact channel-group permissions array (@b optional, @b borrowed).
     *
     * @see pubnub_access_resource_permission_t
     */
    const pubnub_access_resource_permission_t* groups;

    /** Number of entries in @c groups. */
    size_t group_count;

    /**
     * @brief Exact UUID permissions array (@b optional, @b borrowed).
     *
     * @see pubnub_access_resource_permission_t
     */
    const pubnub_access_resource_permission_t* uuids;

    /** Number of entries in @c uuids. */
    size_t uuid_count;

    /**
     * @brief Pattern-based channel permissions array (@b optional,
     *        @b borrowed).
     *
     * Pattern names are regex strings matched server-side.
     *
     * @see pubnub_access_resource_permission_t
     */
    const pubnub_access_resource_permission_t* channel_patterns;

    /** Number of entries in @c channel_patterns. */
    size_t channel_pattern_count;

    /**
     * @brief Pattern-based channel-group permissions array (@b optional,
     *        @b borrowed).
     *
     * @see pubnub_access_resource_permission_t
     */
    const pubnub_access_resource_permission_t* group_patterns;

    /** Number of entries in @c group_patterns. */
    size_t group_pattern_count;

    /**
     * @brief Pattern-based UUID permissions array (@b optional, @b borrowed).
     *
     * @see pubnub_access_resource_permission_t
     */
    const pubnub_access_resource_permission_t* uuid_patterns;

    /** Number of entries in @c uuid_patterns. */
    size_t uuid_pattern_count;

    /**
     * @brief Restrict token to this UUID (@b optional, @b borrowed,
     *        NUL-terminated).
     *
     * With @c NULL token is not restricted to a single UUID.
     */
    const char* authorized_uuid;

    /**
     * @brief JSON metadata string (@b optional, @b borrowed,
     *        NUL-terminated).
     *
     * Embedded in the token and available after parsing.
     */
    const char* meta;

    /**
     * @brief Per-request timeout override in milliseconds (@b optional).
     *
     * @note When non-zero, takes priority over the context-level
     *       @c pubnub_config_t::transaction_timeout_ms.
     */
    uint32_t timeout_ms;
} pubnub_grant_token_opts_t;

/** Designated-initializer macro for @c pubnub_grant_token_opts_t. */
#define PUBNUB_GRANT_TOKEN_OPTS_INIT {0}

/**
 * @brief Options for @c pubnub_revoke_token.
 *
 * Initialize with @c PUBNUB_REVOKE_TOKEN_OPTS_INIT (zero-init
 * produces valid defaults for optional fields).
 *
 * @see pubnub_revoke_token
 */
typedef struct pubnub_revoke_token_opts {
    /** Token string to revoke (@b required, @b borrowed, NUL-terminated). */
    const char* token;

    /**
     * @brief Per-request timeout override in milliseconds (@b optional).
     *
     * @note When non-zero, takes priority over the context-level
     *       @c pubnub_config_t::transaction_timeout_ms.
     */
    uint32_t timeout_ms;
} pubnub_revoke_token_opts_t;

/** Designated-initializer macro for @c pubnub_revoke_token_opts_t. */
#define PUBNUB_REVOKE_TOKEN_OPTS_INIT {0}

/**
 * @brief Options for @c pubnub_parse_token.
 *
 * Initialize with @c PUBNUB_PARSE_TOKEN_OPTS_INIT (zero-init produces valid
 * defaults for optional fields).
 *
 * @see pubnub_parse_token
 */
typedef struct pubnub_parse_token_opts {
    /** Base64url-encoded token string (@b required, @b borrowed,
     * NUL-terminated). */
    const char* token;
} pubnub_parse_token_opts_t;

/** Designated-initializer macro for @c pubnub_parse_token_opts_t. */
#define PUBNUB_PARSE_TOKEN_OPTS_INIT {0}

/**
 * @brief Result from a successful @c pubnub_grant_token call.
 *
 * @note Response data valid until @c pubnub_future_release is called on the
 *       owning future.
 *
 * @see pubnub_grant_token
 * @see pubnub_future_release
 */
typedef struct pubnub_grant_token_result {
    /** The issued token string. */
    pubnub_string_view_t token;
} pubnub_grant_token_result_t;

/**
 * @brief A single resource entry from a parsed access token.
 *
 * @note Parsed data valid until the next call to @c pubnub_parse_token on the
 *       same context or until @c pubnub_destroy / @c pubnub_deinit.
 *
 * @see pubnub_parse_token
 * @see pubnub_parsed_token_channel_at
 * @see pubnub_destroy
 * @see pubnub_deinit
 */
typedef struct pubnub_parsed_token_resource {
    /** Resource name (aliases parsed token data). */
    pubnub_string_view_t name;

    /**
     * @brief OR'd @c pubnub_access_permission_t bits.
     *
     * @see pubnub_access_permission_t
     */
    uint32_t permissions;
} pubnub_parsed_token_resource_t;

/**
 * @brief Top-level result from @c pubnub_parse_token.
 *
 * Use the @c *_count fields as loop bounds for indexed resource accessors.
 *
 * @code
 * pubnub_parsed_token_t tok = {0};
 * pubnub_parse_token_opts_t opts = PUBNUB_PARSE_TOKEN_OPTS_INIT;
 * opts.token = "p0thisIsAToken...";
 * if (PUBNUB_OK == pubnub_parse_token(ctx, &opts, &tok)) {
 *     for (size_t i = 0; i < tok.channel_count; ++i) {
 *         pubnub_parsed_token_resource_t r =
 *             pubnub_parsed_token_channel_at(ctx, i);
 *         printf("%.*s perms=0x%x\n",
 *                (int)r.name.len, r.name.ptr,
 *                r.permissions);
 *     }
 * }
 * @endcode
 *
 * @see pubnub_parse_token
 * @see pubnub_parsed_token_channel_at
 */
typedef struct pubnub_parsed_token {
    /** Token format version. */
    int32_t version;

    /** Token creation timestamp (Unix seconds). */
    uint64_t timestamp;

    /** Token time-to-live in minutes. */
    uint32_t ttl;

    /**
     * @brief Authorized UUID embedded in the token.
     *
     * Empty (@c .len == 0) when no UUID restriction was set
     * during grant.
     */
    pubnub_string_view_t authorized_uuid;

    /** Number of exact channel permissions. */
    uint32_t channel_count;

    /** Number of exact channel-group permissions. */
    uint32_t group_count;

    /** Number of exact UUID permissions. */
    uint32_t uuid_count;

    /** Number of pattern-based channel permissions. */
    uint32_t channel_pattern_count;

    /** Number of pattern-based channel-group permissions. */
    uint32_t group_pattern_count;

    /** Number of pattern-based UUID permissions. */
    uint32_t uuid_pattern_count;
} pubnub_parsed_token_t;

/**
 * @brief Grant an access token with specified permissions.
 *
 * Example (cooperative polling)
 * @code
 * pubnub_access_resource_permission_t ch_perms[] = {
 *     {"my-channel", PUBNUB_ACCESS_READ | PUBNUB_ACCESS_WRITE},
 * };
 * pubnub_grant_token_opts_t opts = PUBNUB_GRANT_TOKEN_OPTS_INIT;
 * opts.ttl = 60;
 * opts.channels = ch_perms;
 * opts.channel_count = 1;
 *
 * pubnub_future_t fut = pubnub_grant_token(ctx, &opts);
 * while (!pubnub_future_is_ready(fut)) {
 *     pubnub_process(ctx);
 * }
 *
 * if (PUBNUB_OK == pubnub_future_status(fut)) {
 *     pubnub_grant_token_result_t r = pubnub_grant_token_result(fut);
 *     printf("token: %.*s\n", (int)r.token.len, r.token.ptr);
 * }
 * pubnub_future_release(fut);
 * @endcode
 *
 * On validation failure (zero TTL, no permissions specified,
 * missing @c secret_key or @c subscribe_key, queue full) the returned
 * future carries an immediate error code readable via
 * @c pubnub_future_status.
 *
 * @param ctx  Initialized context (@b borrowed).
 * @param opts Grant-token options struct (@b borrowed).
 * @return Future handle (owned by caller). Release via
 *         @c pubnub_future_release after reading results.
 *
 * @note Required context configuration: @c subscribe_key,
 *       @c publish_key, and @c secret_key must be set.
 *
 * @see pubnub_grant_token_result
 * @see pubnub_future_release
 * @see pubnub_grant_token_opts_t
 * @see pubnub_context_t
 * @see pubnub_future_t
 */
PUBNUB_API pubnub_future_t pubnub_grant_token(pubnub_context_t* ctx,
                                              const pubnub_grant_token_opts_t* opts);

/**
 * @brief Revoke an existing access token.
 *
 * Example
 * @code
 * pubnub_revoke_token_opts_t opts = PUBNUB_REVOKE_TOKEN_OPTS_INIT;
 * opts.token = "p0thisIsAToken...";
 *
 * pubnub_future_t fut = pubnub_revoke_token(ctx, &opts);
 * pubnub_res_t st = pubnub_await(fut);
 * if (PUBNUB_OK != st) {
 *     pubnub_string_view_t err = pubnub_response_error_message(fut);
 *     printf("revoke failed: %.*s\n", (int)err.len, err.ptr);
 * }
 * pubnub_future_release(fut);
 * @endcode
 *
 * @param ctx  Initialized context (@b borrowed).
 * @param opts Revoke-token options struct (@b borrowed).
 * @return Future handle (owned by caller). Release via
 *         @c pubnub_future_release after reading results.
 *
 * @note Required context configuration: @c subscribe_key and
 *       @c secret_key must be set.
 *
 * @see pubnub_future_release
 * @see pubnub_revoke_token_opts_t
 * @see pubnub_context_t
 * @see pubnub_future_t
 */
PUBNUB_API pubnub_future_t pubnub_revoke_token(pubnub_context_t* ctx,
                                               const pubnub_revoke_token_opts_t* opts);

/**
 * @brief Parse and decode a base64url-encoded access token locally.
 *
 * @note Calling @c pubnub_parse_token again on the same context
 * replaces the previously cached parse result.
 *
 * Example
 * @code
 * pubnub_parsed_token_t tok = {0};
 * pubnub_parse_token_opts_t opts = PUBNUB_PARSE_TOKEN_OPTS_INIT;
 * opts.token = received_token;
 *
 * pubnub_res_t st = pubnub_parse_token(ctx, &opts, &tok);
 * if (PUBNUB_OK == st) {
 *     printf("version=%d ttl=%u channels=%u\n",
 *            tok.version, tok.ttl, tok.channel_count);
 * }
 * @endcode
 *
 * @param ctx        Initialized context (@b required, @b borrowed). Used
 *                   for allocator access and feature-state caching.
 * @param opts       Parse-token options struct (@b required, @b borrowed).
 * @param out_result Caller-owned output struct (@b required), populated on
 *                   success and zero-initialized on failure.
 * @retval PUBNUB_OK on success
 * @retval PUBNUB_ERR_INVALID_ARGUMENT if @c opts->token is @c NULL.
 * @retval PUBNUB_ERR_SERIALIZATION on decode failure.
 * @retval PUBNUB_ERR_OUT_OF_MEMORY if token allocation fails.
 * @retval PUBNUB_ERR_NOT_INITIALIZED if required providers are absent.
 *
 * @see pubnub_parsed_token_channel_at
 * @see pubnub_parsed_token_group_at
 * @see pubnub_parsed_token_uuid_at
 * @see pubnub_parse_token_opts_t
 * @see pubnub_parsed_token_t
 * @see pubnub_context_t
 */
PUBNUB_API pubnub_res_t pubnub_parse_token(pubnub_context_t* ctx,
                                           const pubnub_parse_token_opts_t* opts,
                                           pubnub_parsed_token_t* out_result);

/**
 * @brief Read an exact channel permission from the last parsed token.
 *
 * @param ctx   Context that holds the parsed token state (@b required,
 *              @b borrowed).
 * @param index Channel index in [0, @c channel_count].
 * @return Resource permission entry, or zero-initialized if
 *         @p index is out of range or no token has been parsed.
 *
 * @see pubnub_parsed_token_resource_t
 * @see pubnub_context_t
 */
PUBNUB_API pubnub_parsed_token_resource_t
pubnub_parsed_token_channel_at(pubnub_context_t* ctx, size_t index);

/**
 * @brief Read an exact channel-group permission from the last parsed token.
 *
 * @param ctx   Context that holds the parsed token state (@b required,
 *              @b borrowed).
 * @param index Group index in [0, @c group_count].
 * @return Resource permission entry, or zero-initialized if
 *         @p index is out of range or no token has been parsed.
 *
 * @see pubnub_parsed_token_resource_t
 * @see pubnub_context_t
 */
PUBNUB_API pubnub_parsed_token_resource_t
pubnub_parsed_token_group_at(pubnub_context_t* ctx, size_t index);

/**
 * @brief Read an exact UUID permission from the last parsed token.
 *
 * @param ctx   Context that holds the parsed token state (@b required,
 *              @b borrowed).
 * @param index UUID index in [0, @c uuid_count].
 * @return Resource permission entry, or zero-initialized if
 *         @p index is out of range or no token has been parsed.
 *
 * @see pubnub_parsed_token_resource_t
 * @see pubnub_context_t
 */
PUBNUB_API pubnub_parsed_token_resource_t
pubnub_parsed_token_uuid_at(pubnub_context_t* ctx, size_t index);

/**
 * @brief Read a pattern-based channel permission from the last parsed token.
 *
 * @param ctx   Context that holds the parsed token state (@b required,
 *              @b borrowed).
 * @param index Pattern index in [0, @c channel_pattern_count].
 * @return Resource permission entry, or zero-initialized if
 *         @p index is out of range or no token has been parsed.
 *
 * @see pubnub_parsed_token_resource_t
 * @see pubnub_context_t
 */
PUBNUB_API pubnub_parsed_token_resource_t
pubnub_parsed_token_channel_pattern_at(pubnub_context_t* ctx, size_t index);

/**
 * @brief Read a pattern-based channel-group permission from the
 *        last parsed token.
 *
 * @param ctx   Context that holds the parsed token state (@b required,
 *              @b borrowed).
 * @param index Pattern index in [0, @c group_pattern_count].
 * @return Resource permission entry, or zero-initialized if
 *         @p index is out of range or no token has been parsed.
 *
 * @see pubnub_parsed_token_resource_t
 * @see pubnub_context_t
 */
PUBNUB_API pubnub_parsed_token_resource_t
pubnub_parsed_token_group_pattern_at(pubnub_context_t* ctx, size_t index);

/**
 * @brief Read a pattern-based UUID permission from the last parsed token.
 *
 * @param ctx   Context that holds the parsed token state (@b required,
 *              @b borrowed).
 * @param index Pattern index in [0, @c uuid_pattern_count).
 * @return Resource permission entry, or zero-initialized if
 *         @p index is out of range or no token has been parsed.
 *
 * @see pubnub_parsed_token_resource_t
 * @see pubnub_context_t
 */
PUBNUB_API pubnub_parsed_token_resource_t
pubnub_parsed_token_uuid_pattern_at(pubnub_context_t* ctx, size_t index);

/**
 * @brief Read the grant-token result from a completed future.
 *
 * @param future Future returned from @c pubnub_grant_token.
 * @return Result struct with token view, or zero-initialized if
 *         the future is not ready or carries an error.
 *
 * @note Response data valid until @c pubnub_future_release is called on the
 *       same future.
 *
 * @see pubnub_future_release
 * @see pubnub_grant_token
 * @see pubnub_context_t
 * @see pubnub_future_t
 */
PUBNUB_API pubnub_grant_token_result_t pubnub_grant_token_result(pubnub_future_t future);

#ifdef __cplusplus
// clang-format off
}
// clang-format on
#endif

#endif /* PUBNUB_ENABLE_PAM */

#endif /* PUBNUB_FEATURE_ACCESS_H */
