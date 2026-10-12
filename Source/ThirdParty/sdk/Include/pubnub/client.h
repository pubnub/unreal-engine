/* Copyright (c) PubNub Inc. */
/* See LICENSE in the root directory of this source tree. */

/**
 * @file client.h
 * @brief PubNub client context: creation, initialization, lifecycle.
 *
 * Two lifecycle models are supported:
 *   - Hosted allocation: pubnub_create() / pubnub_destroy()
 *     (allocates context struct from cfg.allocator; unavailable when
 *      PUBNUB_CFG_NO_HEAP is 1 — use pubnub_init() instead)
 *   - Caller-provided:   pubnub_init()   / pubnub_deinit()
 *
 * Thread-safety: separate contexts are safe to use concurrently.
 * Same-context concurrent use requires external synchronization.
 */

#ifndef PUBNUB_CLIENT_H
#define PUBNUB_CLIENT_H

#include "pubnub/config.h"
#include "pubnub/error.h"
#include "pubnub/proxy.h"
#include "pubnub/tcp_keepalive.h"
#include "pubnub/types_fwd.h"
#include "pubnub/providers/allocator.h"
#include "pubnub/providers/transport.h"
#include "pubnub/providers/serialization.h"
#include "pubnub/providers/crypto.h"
#include "pubnub/providers/logger.h"
#include "pubnub/providers/platform.h"
/* Backward compat: logger functions moved to pubnub/log.h (remove in next
 * major version). */
#include "pubnub/log.h"

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
// clang-format off
extern "C" {
// clang-format on
#endif

/**
 * @brief Retry policy type.
 *
 * Runtime retry logic is only active when @c PUBNUB_ENABLE_RETRY is enabled at
 * compile time.
 */
typedef enum pubnub_retry_policy {
    /** No automatic retries (default when zero-initialized). */
    PUBNUB_RETRY_NONE = 0,

    /**
     * @brief Linear retry policy.
     *
     * Request retried with equal delay between each retry attempt.
     *
     * @b Default: @c PUBNUB_CFG_RETRY_DELAY_MS delay,
     *             @c PUBNUB_CFG_LINEAR_MAX_RETRIES max retries.
     */
    PUBNUB_RETRY_LINEAR,

    /**
     * @brief Exponential retry policy.
     *
     * Request retried with delay growing exponentially from base delay.
     *
     * @b Default: @c PUBNUB_CFG_RETRY_DELAY_MS base delay,
     *             @c PUBNUB_CFG_RETRY_MAX_DELAY_MS max delay,
     *             @c PUBNUB_CFG_EXPONENTIAL_MAX_RETRIES max retries.
     */
    PUBNUB_RETRY_EXPONENTIAL
} pubnub_retry_policy_t;

/**
 * @brief Endpoint groups for retry exclusion bitmask.
 *
 * Use bitwise OR to combine groups that should NOT be retried.
 */
typedef enum pubnub_endpoint_group {
    /** Publish / signal. */
    PUBNUB_ENDPOINT_MESSAGE_SEND = 0x01,
    /** Subscribe long-poll. */
    PUBNUB_ENDPOINT_SUBSCRIBE = 0x02,
    /** Presence (here-now, leave). */
    PUBNUB_ENDPOINT_PRESENCE = 0x04,
    /** History / message persistence. */
    PUBNUB_ENDPOINT_MESSAGE_STORAGE = 0x08,
    /** Channel-group management. */
    PUBNUB_ENDPOINT_CHANNEL_GROUPS = 0x10,
    /** App Context (objects). */
    PUBNUB_ENDPOINT_APP_CONTEXT = 0x20,
    /** Message actions / reactions. */
    PUBNUB_ENDPOINT_MESSAGE_REACTIONS = 0x40,
    /** Access Manager grants. */
    PUBNUB_ENDPOINT_PAM = 0x80,
    /** File upload / download / list / delete. */
    PUBNUB_ENDPOINT_FILES = 0x100
} pubnub_endpoint_group_t;

/**
 * @brief Retry policy configuration.
 *
 * Set @c policy to @c PUBNUB_RETRY_LINEAR or @c PUBNUB_RETRY_EXPONENTIAL to
 * enable retries with PubNub-standard defaults.
 *
 * @code
 * pubnub_retry_configuration_t retry = {
 *     .policy = PUBNUB_RETRY_EXPONENTIAL,
 *     .excluded_endpoints = PUBNUB_ENDPOINT_MESSAGE_SEND
 *                         | PUBNUB_ENDPOINT_MESSAGE_STORAGE,
 * };
 * @endcode
 *
 * @b Default: @c PUBNUB_RETRY_NONE (zero-initialized struct means no automatic
 *             retries).
 *
 * @attention All time values are in milliseconds, consistent with timeout
 *            fields elsewhere in @c pubnub_config_t.
 */
typedef struct pubnub_retry_configuration {
    /**
     * @brief Retry policy type.
     *
     * @see pubnub_retry_policy_t
     */
    pubnub_retry_policy_t policy;

    /**
     * @brief Delay between retries.
     *
     * Depending on from the @a policy has a bit different meaning:
     * - @c linear: delay between retries (milliseconds, min 2000).
     * - @c exponential: base delay (milliseconds, min 2000).
     *
     * @c 0 - use compile-time default (@c PUBNUB_CFG_RETRY_DELAY_MS).
     */
    unsigned int delay_ms;

    /**
     * @brief Maximum computed delay cap (milliseconds).
     *
     * @c 0 - use compile-time default (@c PUBNUB_CFG_RETRY_MAX_DELAY_MS).
     *
     * @note Ignored for linear policy.
     */
    unsigned int maximum_delay_ms;

    /**
     * @brief Maximum retry attempts before reporting an error.
     *
     * @c 0 - use compile-time default per policy type.
     *
     * @b Maximum: @c 10.
     */
    unsigned int maximum_retry;

    /**
     * @brief Upper bound on a server-provided @c Retry-After delay
     *        (milliseconds).
     *
     * A @c 429 or @c 503 response may carry a @c Retry-After header asking
     * you to wait an arbitrary number of seconds. Without a cap, a hostile
     * or misconfigured server could stall a request for hours. The SDK
     * clamps the honored @c Retry-After delay to this value.
     *
     * @c 0 - use compile-time default
     *        (@c PUBNUB_CFG_RETRY_MAX_RETRY_AFTER_MS).
     */
    unsigned int maximum_retry_after_ms;

    /**
     * @brief Bitmask of endpoint groups excluded from retry.
     *
     * Use bitwise OR of @c pubnub_endpoint_group_t values.
     *
     * @c 0 - retry all endpoint groups.
     */
    unsigned int excluded_endpoints;
} pubnub_retry_configuration_t;

/**
 * @brief Client configuration.
 *
 * After a context is initialized or created with this config, all fields in the
 * config struct are treated as immutable. Five fields are runtime-mutable via
 * dedicated setters after init:
 * - @c auth_token   (@c pubnub_set_auth_token)
 * - @c user_id      (@c pubnub_set_user_id)
 * - @c log_level    (@c pubnub_set_log_level)
 * - @c dns_primary, @c dns_secondary (@c pubnub_set_dns_servers)
 *
 * @b String @b ownership @b rules (@c const @c char* fields):
 * - @c pubnub_init: all string pointers are @b borrowed. The caller must
 *   keep the pointed-to strings valid for the lifetime of the context.
 *   The SDK does NOT copy them.
 * - @c pubnub_create: all string fields are @b deep-copied into memory
 *   obtained from the resolved @c allocator. The caller may free or reuse the
 *   originals immediately after @c pubnub_create returns.
 *
 * @b Important: Provider pointer fields are always borrowed references; the
 * caller must ensure provider structs remain valid for the lifetime of every
 * context initialized with this config.
 *
 * Provider pointers set to @c NULL use the compiled-in default
 * implementation (selected by the CMake @c PUBNUB_PROVIDER_* options).
 * If no default is available (@c PUBNUB_PROVIDER_<FAMILY>=custom), a @c NULL
 * required provider causes initialization to fail with
 * @c PUBNUB_ERR_PROVIDER_MISSING.
 *
 * @note Declare a struct literal with only the fields you need; all zero-valued
 * fields resolve to documented defaults at @c pubnub_create / @c pubnub_init
 * time.
 */
typedef struct pubnub_config {
    /** Subscribe key (@b required, null-terminated). */
    const char* subscribe_key;

    /**
     * @brief Publish key (@b optional, null-terminated).
     *
     * @note Required only for publish operations and PAM signing.
     */
    const char* publish_key;

    /**
     * @brief Secret key (@b optional, null-terminated).
     *
     * @note Required only for PAM signing.
     */
    const char* secret_key;

    /**
     * @brief SDK suffix appended to the `pnsdk` query parameter (@b optional,
     *        null-terminated).
     *
     * When non-NULL and non-empty, the value is appended (space-separated)
     * to the base SDK identifier (e.g. @c "PubNub-C/0.1.0 Chat/1.0.0").
     * The caller pre-formats the string as space-separated product
     * identifiers (e.g. @c "ProductName/1.0.0 AnotherSDK/2.0").
     */
    const char* pnsdk_suffix;

    /**
     * @brief Override for the base SDK identifier in the @c pnsdk query
     *        parameter (@b optional, null-terminated).
     *
     * When non-NULL and non-empty, completely replaces the compile-time
     * @c PUBNUB_SDK_IDENTIFIER as the base string sent in @c pnsdk.
     * The @c pnsdk_suffix (if set) is still appended after it. Use this
     * for wrapper SDKs that need their own identity (e.g.
     * @c "Unreal-PubNub/5.4" or @c "Unity/2.3.0").
     *
     * @c NULL (default, zero-init safe) uses the compile-time identifier
     * unchanged.
     */
    const char* pnsdk_override;

    /**
     * @brief User ID / UUID (@b required, null-terminated).
     *
     * @note Runtime-mutable after init via @c pubnub_set_user_id.
     */
    const char* user_id;

    /**
     * @brief PAM v3 auth token (@b optional, null-terminated).
     *
     * @note Runtime-mutable after init via @c pubnub_set_auth_token.
     */
    const char* auth_token;

    /**
     * @brief PubNub origin host (@b optional, null-terminated).
     *
     * @c NULL or empty - use compile-time default (@c PUBNUB_CFG_ORIGIN).
     */
    const char* origin;

    /**
     * @brief Primary DNS server address (@b optional, null-terminated).
     *
     * IPv4 ("8.8.8.8") or IPv6 ("2001:4860:4860::8888") address
     * string. When non-NULL, the transport uses this server for DNS
     * resolution instead of (or in addition to) system-discovered
     * servers. @c NULL means use system defaults.
     *
     * @note Runtime-mutable after init via @c pubnub_set_dns_servers.
     */
    const char* dns_primary;

    /**
     * @brief Secondary DNS server address (@b optional, null-terminated).
     *
     * Same format as @c dns_primary. Used as fallback when the primary
     * server is unreachable. @c NULL means no secondary server.
     *
     * @note Runtime-mutable after init via @c pubnub_set_dns_servers.
     */
    const char* dns_secondary;

    /**
     * @brief Timeout for short-lived transactional requests (@c publish,
     *        @c history, @c here-now, @c signal, etc.) in milliseconds.
     *
     * @c 0 - use compile-time default (@c PUBNUB_CFG_TRANSACTION_TIMEOUT_MS).
     */
    unsigned int transaction_timeout_ms;

    /**
     * @brief Timeout for long-running non-transactional requests (@c subscribe
     *        long-poll) in milliseconds.
     *
     * @c 0 - use compile-time default
     * (@c PUBNUB_CFG_NON_TRANSACTION_TIMEOUT_MS).
     */
    unsigned int non_transaction_timeout_ms;

    /**
     * @brief Retry policy.
     *
     * @b Default: @c PUBNUB_RETRY_EXPONENTIAL for subscribe only (all other
     *    endpoint groups excluded); numeric parameters filled from compile-time
     *    defaults. See @c pubnub_config_defaults().
     *
     * @note Ignored at runtime when @c PUBNUB_ENABLE_RETRY is disabled at
     *       compile time.
     *
     * @see pubnub_retry_configuration_t
     */
    pubnub_retry_configuration_t retry_configuration;

    /**
     * @brief Presence timeout in seconds.
     *
     * Sets only the @c ?heartbeat={value} parameter registered with the
     * server; it does not derive a client-side heartbeat interval. @c 0
     * (default) uses an internal default of 300 s for that server-side
     * parameter. To dispatch periodic client-side heartbeat REST calls,
     * set @c heartbeat_interval to a nonzero value.
     *
     * @note Ignored at runtime when @c PUBNUB_ENABLE_PRESENCE is
     *       disabled at compile time.
     */
    uint32_t presence_timeout;

    /**
     * @brief Heartbeat interval in seconds.
     *
     * Delay between periodic client-side heartbeat requests while
     * subscribed. @c 0 (default) disables automatic client-side heartbeats
     * entirely. Set a nonzero value in seconds to enable periodic heartbeat
     * dispatching.
     *
     * @note Ignored at runtime when @c PUBNUB_ENABLE_PRESENCE is disabled at
     *       compile time.
     */
    uint32_t heartbeat_interval;

    /**
     * @brief Subscribe filter expression (@b optional, null-terminated).
     *
     * Server-side filter applied to incoming subscribe messages. Only
     * messages matching the expression are delivered. See PubNub docs
     * for expression syntax.
     *
     * @note Immutable after init.
     */
    const char* filter_expression;

    /**
     * @brief Suppress presence leave events.
     *
     * When non-zero, the SDK does not send @c /v2/presence/.../leave
     * REST calls on unsubscribe. The server will time the client out
     * after @c presence_timeout seconds instead. Reduces network traffic
     * on constrained links.
     *
     * @note Ignored at runtime when @c PUBNUB_ENABLE_PRESENCE is disabled at
     *       compile time.
     */
    uint8_t suppress_leave_events;

    /**
     * @brief Proxy configuration.
     *
     * Zero-initialized struct means "no proxy" (@c PUBNUB_PROXY_NONE).
     * Each transport provider decides whether/how to honor the
     * configured proxy type; unsupported types are rejected at send
     * time.
     *
     * @see pubnub_proxy_config_t
     */
    pubnub_proxy_config_t proxy;

    /**
     * @brief TCP keepalive configuration.
     *
     * Controls OS-level keepalive probes on transport sockets to detect
     * dead peers (VPN disconnects, NAT timeouts, network partitions).
     *
     * @b Default:
     * - When zero-initialized (e.g., via direct @c pubnub_init or aggregate
     *   initialization with @c {0}): keepalive is disabled (@c enabled = 0).
     * - When initialized via @c pubnub_config_defaults(): keepalive is enabled
     *   with idle=60s, interval=20s, probes=3.
     *
     * @see pubnub_tcp_keepalive_config_t
     * @see PUBNUB_TCP_KEEPALIVE_CONFIG_INIT
     */
    pubnub_tcp_keepalive_config_t tcp_keepalive;

    /** Per-context allocator provider (@b optional). */
    pubnub_allocator_provider_t* allocator;

    /** Per-context transport provider (@b optional). */
    pubnub_transport_provider_t* transport;

    /** Per-context serialization provider (@b optional). */
    pubnub_serialization_provider_t* serialization;

    /** Shared platform provider (@b optional). */
    pubnub_platform_provider_t* platform;

    /**
     * @brief Crypto module for payload encryption / decryption (@b optional,
     *        @b borrowed).
     *
     * When set, features (@c publish, @c subscribe, @c history, @c files) will
     * automatically encrypt/decrypt message payloads.
     *
     * @attention Caller manages the module's lifetime.
     *
     * @note @c NULL disables payload encryption.
     */
    pubnub_crypto_module_t* crypto_module;

    /**
     * @brief Optional additional logger provider.
     *
     * When non-NULL, registered alongside the built-in default logger
     * at context creation. Set to NULL to use only the built-in default.
     * To add further providers after context creation, call
     * pubnub_logger_add().
     */
    pubnub_logger_provider_t* logger;

    /**
     * @brief Initial minimum log level applied at context creation.
     *
     * Set to suppress or restrict logging before any code in
     * @c pubnub_create or @c pubnub_init can emit a message.
     * Equivalent to calling @c pubnub_set_log_level immediately after
     * init, but takes effect before the first log line.
     *
     * Zero-initialised config (@c {0}) means @c PUBNUB_LOG_LEVEL_NONE
     * (silence). @c pubnub_config_defaults() sets @c PUBNUB_LOG_LEVEL_INFO.
     *
     * @see pubnub_set_log_level
     */
    pubnub_log_level_t log_level;
} pubnub_config_t;

/**
 * @brief Return a @c pubnub_config_t populated with compile-time defaults.
 *
 * Caller must set at least @c subscribe_key and @c user_id before use.
 *
 * @see pubnub_config_t
 */
PUBNUB_API pubnub_config_t pubnub_config_defaults(void);

#if !PUBNUB_CFG_NO_HEAP
/**
 * @brief Allocate and initialize a new client context.
 *
 * Allocates the context struct from @c config->allocator (or from
 * @c pn_allocator_default() when @c config->allocator is @c NULL).
 * Returns @c NULL when the allocator cannot satisfy the allocation.
 *
 * Not available when @c PUBNUB_CFG_NO_HEAP is @c 1. Use
 * @c pubnub_init() with a caller-provided buffer instead.
 *
 * @param config Client configuration (@b required, @b borrowed). See string
 *               ownership rules in @c pubnub_config_t.
 * @return Pointer to the new context, or @c NULL on failure. Caller must
 *         eventually call @c pubnub_destroy with returned value.
 *
 * @see pubnub_context_t
 * @see pubnub_config_t
 * @see pubnub_destroy
 * @see PUBNUB_CONTEXT_SIZE
 * @see pubnub_init
 */
PUBNUB_API pubnub_context_t* pubnub_create(const pubnub_config_t* config);

/**
 * @brief Destroy a previously created context.
 *
 * Joins the background thread (if active), delivers @c PUBNUB_ERR_CANCELLED
 * to all outstanding future callbacks, then releases all resources.
 *
 * @attention After this function returns, no further callbacks will fire
 * for this context.
 *
 * Not available when @c PUBNUB_CFG_NO_HEAP is @c 1.
 *
 * @param ctx Context to destroy (@b consumed). Do not use after this call.
 *
 * @see pubnub_context_t
 * @see pubnub_create
 */
PUBNUB_API void pubnub_destroy(pubnub_context_t* ctx);
#endif /* !PUBNUB_CFG_NO_HEAP */

/**
 * @brief Initialize a caller-provided context in-place.
 *
 * @param ctx    Pointer to caller-owned memory of at least
 *               @c pubnub_context_size bytes (@b required).
 * @param config Client configuration (@b required, @b borrowed). Strings are
 *               borrowed; keep them valid for the context lifetime.
 * @return @c PUBNUB_OK on success, or an error code.
 *
 * @see pubnub_context_t
 * @see pubnub_config_t
 * @see pubnub_res_t
 * @see pubnub_deinit
 */
PUBNUB_API pubnub_res_t pubnub_init(pubnub_context_t*      ctx,
                                    const pubnub_config_t* config);

/**
 * @brief De-initialize a context previously set up with pubnub_init().
 *
 * Joins the background thread (if active), delivers @c PUBNUB_ERR_CANCELLED
 * to all outstanding future callbacks, then releases internally-held resources.
 *
 * @attention After this function returns, no further callbacks will fire
 * for this context. Does NOT free the context memory (caller owns it).
 *
 * @note On platforms where @c transport->wake() is unavailable (FreeRTOS
 *       and Windows socket transport), this function blocks for up to
 *       @c PUBNUB_CFG_MAX_POLL_MS while the background thread completes
 *       its current poll iteration.
 *
 * @param ctx Context to de-initialize (@b borrowed). Do not use the context
 *            after this call; the caller retains ownership of the memory.
 *
 * @see pubnub_context_t
 * @see pubnub_init
 */
PUBNUB_API void pubnub_deinit(pubnub_context_t* ctx);

/**
 * @brief Return the size in bytes required for a @c pubnub_context_t.
 *
 * Use this to allocate memory for @c pubnub_init when the opaque struct size
 * is not known at compile time.
 *
 * @see pubnub_init
 */
PUBNUB_API size_t pubnub_context_size(void);

/**
 * @brief Return the current user ID.
 *
 * @attention The returned pointer is owned by the context. Do not free it.
 * The pointer is valid until the next @c pubnub_set_user_id call or context
 * destruction, whichever comes first.
 *
 * @param ctx Initialized context (@b borrowed).
 * @return Current user ID string, or @c NULL if @c ctx is @e uninitialized.
 *
 * @see pubnub_context_t
 */
PUBNUB_API const char* pubnub_get_user_id(const pubnub_context_t* ctx);

/**
 * @brief Return the current auth token.
 *
 * @attention The returned pointer is owned by the context. Do not free it.
 * The pointer is valid until the next @c pubnub_set_auth_token call or context
 * destruction, whichever comes first.
 *
 * @param ctx Initialized context (@b borrowed).
 * @return Current auth token string, or @c NULL if @c ctx is @e uninitialized
 *         or no token is set.
 *
 * @see pubnub_context_t
 */
PUBNUB_API const char* pubnub_get_auth_token(const pubnub_context_t* ctx);

/**
 * @brief Set or clear the PAM auth token.
 *
 * @param ctx   Initialized context (@b borrowed).
 * @param token NUL-terminated token string (@b optional, @b borrowed), or
 *              @c NULL to clear.
 * @return @c PUBNUB_OK on success; lifecycle-class error when @c ctx is not
 *         initialized; capacity-class error when the heap-backed deep-copy
 *         path runs out of memory.
 *
 * @note @b Deep-copy will be made from the value passed to the context created
 * with @c pubnub_create or @b borrowed if context was initialized with
 * @c pubnub_init (caller must keep the string alive for the lifetime of the
 * context).
 *
 * @see pubnub_context_t
 * @see pubnub_res_t
 * @see pubnub_create
 * @see pubnub_init
 */
PUBNUB_API pubnub_res_t pubnub_set_auth_token(pubnub_context_t* ctx,
                                              const char*       token);

/**
 * @brief Update the user ID.
 *
 * @param ctx     Initialized context (@b borrowed).
 * @param user_id Non-empty NUL-terminated user ID (@b required, @b borrowed).
 * @return @c PUBNUB_OK on success; lifecycle-class error when @c ctx is not
 *         initialized; argument-class error when @c user_id is @c NULL or
 *         empty; capacity-class error when the heap-backed deep-copy path runs
 *         out of memory.
 *
 * @note @b Deep-copy will be made from the value passed to the context created
 * with @c pubnub_create or @b borrowed if context was initialized with
 * @c pubnub_init (caller must keep the string alive for the lifetime of the
 * context).
 *
 * @see pubnub_context_t
 * @see pubnub_res_t
 * @see pubnub_create
 * @see pubnub_init
 */
PUBNUB_API pubnub_res_t pubnub_set_user_id(pubnub_context_t* ctx,
                                           const char*       user_id);

/**
 * @brief Set the minimum log level for this context.
 *
 * Messages below this level are suppressed regardless of the logger
 * provider's own threshold. Use @c PUBNUB_LOG_LEVEL_* constants.
 *
 * @param ctx   Initialized context (@b borrowed).
 * @param level Minimum severity threshold. Messages below this level are
 *              suppressed. Pass @c PUBNUB_LOG_LEVEL_NONE to silence all
 *              output, @c PUBNUB_LOG_LEVEL_TRACE to enable everything.
 * @return @c PUBNUB_OK on success.
 *
 * @see pubnub_context_t
 * @see pubnub_res_t
 */
PUBNUB_API pubnub_res_t pubnub_set_log_level(pubnub_context_t* ctx,
                                             unsigned int      level);

/**
 * @brief Set the PubNub origin (hostname) for subsequent requests.
 *
 * Copies the origin string into an internal fixed-size buffer. The new
 * origin takes effect on the next request dispatch. Requests already
 * in-flight are unaffected (they hold their own copy of the host).
 *
 * Pass NULL or an empty string to reset to the compile-time default
 * (@c PUBNUB_CFG_ORIGIN).
 *
 * @param ctx    Context to update. Must not be NULL.
 * @param origin New origin hostname, or NULL to reset to default.
 * @return PUBNUB_OK on success.
 * @retval PUBNUB_ERR_NOT_INITIALIZED if @p ctx is NULL or not
 *         initialized.
 * @retval PUBNUB_ERR_INVALID_ARGUMENT if @p origin exceeds
 *         @c PUBNUB_CFG_MAX_HOSTNAME_LEN.
 */
PUBNUB_API pubnub_res_t pubnub_set_origin(pubnub_context_t* ctx, const char* origin);

/**
 * @brief Get the current PubNub origin (hostname).
 *
 * The returned pointer is valid until the next call to
 * pubnub_set_origin() on the same context, or until the context is
 * destroyed.
 *
 * @param ctx Context to query. Must not be NULL.
 * @return Current origin string, or NULL if @p ctx is NULL or not
 *         initialized.
 */
PUBNUB_API const char* pubnub_get_origin(const pubnub_context_t* ctx);

/**
 * @brief Return the context's resolved serialization provider.
 *
 * Optional @c vtable entries MAY be @c NULL -- callers MUST @c NULL-check
 * before invoking. The returned pointer is @b borrowed (valid until
 * context destruction).
 *
 * @param ctx Initialized context (@b borrowed).
 * @return Serialization provider, or @c NULL when @p ctx
 *         is @c NULL or uninitialized.
 *
 * @see pubnub_serialization_provider_t
 * @see pubnub_context_t
 */
PUBNUB_API pubnub_serialization_provider_t* pubnub_serialization(pubnub_context_t* ctx);

/**
 * @brief Set custom DNS servers for this context.
 *
 * Overrides the active DNS server list used by the transport. Takes
 * effect on the next DNS resolution attempt. Pass NULL for primary to
 * clear both servers and revert to system discovery.
 *
 * Strings are borrowed or deep-copied following the same ownership
 * rules as pubnub_set_auth_token(): deep-copied when the context was
 * created via pubnub_create(), borrowed when initialized via
 * pubnub_init().
 *
 * Thread-safe.
 *
 * @param ctx       Initialized context (@b borrowed).
 * @param primary   IPv4 ("8.8.8.8") or IPv6 ("2001:4860:4860::8888")
 *                  string, or @c NULL to clear both servers.
 * @param secondary IPv4 or IPv6 string, or @c NULL.
 * @return @c PUBNUB_OK on success.
 * @retval PUBNUB_ERR_NOT_INITIALIZED if @p ctx is uninitialized.
 * @retval PUBNUB_ERR_OUT_OF_MEMORY on deep-copy allocation failure.
 * @retval PUBNUB_ERR_NOT_SUPPORTED if the active transport cannot
 *         honour custom DNS.
 * @retval PUBNUB_ERR_INVALID_ARGUMENT if @p primary or @p secondary is
 *         non-NULL but is not a valid IPv4 or IPv6 address string.
 *
 * @note Thread-safe on all supported targets including multi-core RTOS
 *       (dual-core ESP32-S3, Cortex-A SMP). The implementation uses
 *       acquire/release atomic ordering for the server count field.
 *
 * @see pubnub_context_t
 * @see pubnub_res_t
 */
PUBNUB_API pubnub_res_t pubnub_set_dns_servers(pubnub_context_t* ctx,
                                               const char*       primary,
                                               const char*       secondary);

/**
 * @brief Replace the TLS CA bundle for future connections.
 *
 * Pass a NUL-terminated PEM certificate chain to use a custom CA
 * (enterprise proxy, self-signed server). Pass NULL to revert to
 * the platform system certificate store. Takes effect on connections
 * opened after this call; in-flight connections are unaffected.
 *
 * @param ctx     Initialized context (@b borrowed).
 * @param ca_pem  PEM chain, or NULL for system certs.
 * @return @c PUBNUB_OK on success.
 * @retval PUBNUB_ERR_NOT_INITIALIZED if @p ctx is uninitialized.
 */
PUBNUB_API pubnub_res_t pubnub_set_tls_ca_bundle(pubnub_context_t* ctx,
                                                 const char*       ca_pem);

/**
 * @brief Skip (disable) TLS peer certificate verification.
 *
 * Pass a non-zero @p skip_verify to disable peer certificate
 * verification; pass zero to re-enable it (the secure default).
 *
 * @warning Disabling verification exposes connections to
 *          man-in-the-middle attacks. Only use in development and
 *          testing -- never in production.
 *
 * @param ctx         Initialized context (@b borrowed).
 * @param skip_verify Non-zero to skip (disable) verification; zero to
 *                    re-enable it.
 * @return @c PUBNUB_OK on success.
 * @retval PUBNUB_ERR_NOT_INITIALIZED if @p ctx is uninitialized.
 * @retval PUBNUB_ERR_NOT_SUPPORTED if the active transport does not
 *         support toggling TLS verification.
 */
PUBNUB_API pubnub_res_t pubnub_set_tls_skip_verify(pubnub_context_t* ctx,
                                                   uint8_t skip_verify);

/**
 * @brief Pump the context event loop one non-blocking tick.
 *
 * Dispatches pending requests, drives transport I/O, and observes
 * completions. Returns without blocking.
 *
 * @param ctx Initialized context (@b borrowed).
 * @retval PUBNUB_OK when quiescent.
 * @retval PUBNUB_IN_PROGRESS when work remains.
 * @retval PUBNUB_ERR_NOT_INITIALIZED PubNub context not
 *         ready / @e uninitialized.
 *
 * @see pubnub_context_t
 * @see pubnub_res_t
 */
PUBNUB_API pubnub_res_t pubnub_process(pubnub_context_t* ctx);

#ifdef __cplusplus
// clang-format off
}
// clang-format on
#endif

#endif /* PUBNUB_CLIENT_H */
