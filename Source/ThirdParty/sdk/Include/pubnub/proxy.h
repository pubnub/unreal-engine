/* Copyright (c) PubNub Inc. */
/* See LICENSE in the root directory of this source tree. */

/**
 * @file proxy.h
 * @brief Proxy configuration types for the PubNub C SDK.
 *
 * Proxy settings are specified per-context via `pubnub_config_t`.
 * Each transport provider decides how (or whether) to honour a given
 * proxy type; unsupported types may be rejected at init time (preferred
 * for embedded — gives immediate feedback) or at send time with
 * `PUBNUB_HTTP_ERROR`.
 */

#ifndef PUBNUB_PROXY_H
#define PUBNUB_PROXY_H

#include <stdint.h>

#ifdef __cplusplus
// clang-format off
extern "C" {
// clang-format on
#endif

/**
 * @brief Proxy protocol type.
 *
 * Values are intentionally kept small (single-byte range) so the
 * struct packs without padding on 32-bit targets.
 */
typedef enum pubnub_proxy_type {
    /** No proxy configured (default when zero-initialized). */
    PUBNUB_PROXY_NONE = 0,

    /** HTTP CONNECT proxy (RFC 7231 section 4.3.6). */
    PUBNUB_PROXY_HTTP_CONNECT = 1,

    /** SOCKS5 proxy (RFC 1928). */
    PUBNUB_PROXY_SOCKS5 = 2,

    /**
     * Auto-discover proxy via OS WPAD/PAC settings (blocking at init).
     * Falls back to direct connection if discovery fails or if the
     * platform has no auto-discovery API.
     *
     * @note The built-in curl transport does not support WPAD/PAC
     *       discovery. Configuring this value returns
     *       @c PUBNUB_ERR_NOT_SUPPORTED at send time.
     */
    PUBNUB_PROXY_AUTO = 3
} pubnub_proxy_type_t;

/**
 * @brief Proxy authentication scheme.
 *
 * Determines how `username` / `password` fields are interpreted.
 */
typedef enum pubnub_proxy_auth {
    /** No authentication (username/password ignored). */
    PUBNUB_PROXY_AUTH_NONE = 0,

    /** HTTP Basic authentication (RFC 7617). */
    PUBNUB_PROXY_AUTH_BASIC = 1,

    /**
     * HTTP Digest authentication (RFC 7616, MD5 algorithm).
     *
     * @note The built-in curl transport does not support Digest auth.
     *       Configuring this value with credentials returns
     *       @c PUBNUB_ERR_NOT_SUPPORTED at send time.
     */
    PUBNUB_PROXY_AUTH_DIGEST = 2,

    /**
     * NTLM authentication (NTLMv2, ASCII credentials only).
     *
     * @note The built-in curl transport does not support NTLM auth.
     *       Configuring this value with credentials returns
     *       @c PUBNUB_ERR_NOT_SUPPORTED at send time.
     */
    PUBNUB_PROXY_AUTH_NTLM = 3
} pubnub_proxy_auth_t;

/**
 * @brief Proxy configuration.
 *
 * Zero-initialized struct means "no proxy" (`PUBNUB_PROXY_NONE`).
 * Transport providers read this via `pubnub_provider_deps_t::proxy`
 * at init time.
 *
 * String fields (`host`, `username`, `password`) follow the same
 * ownership rules as `pubnub_config_t` string fields:
 *   - `pubnub_init()`:   borrowed -- caller must keep strings alive.
 *   - `pubnub_create()`: deep-copied by the SDK.
 *
 * @code
 * pubnub_proxy_config_t proxy = {
 *     .type     = PUBNUB_PROXY_HTTP_CONNECT,
 *     .host     = "proxy.corp.example.com",
 *     .port     = 3128,
 *     .auth     = PUBNUB_PROXY_AUTH_BASIC,
 *     .username = "device-001",
 *     .password = "s3cr3t",
 * };
 * pubnub_config_t cfg = pubnub_config_defaults();
 * cfg.proxy = proxy;
 * @endcode
 */
typedef struct pubnub_proxy_config {
    /** Proxy protocol type. */
    pubnub_proxy_type_t type;

    /** Authentication scheme. */
    pubnub_proxy_auth_t auth;

    /** Proxy hostname or IP address (null-terminated, may be @c NULL
     *  when type == @c PUBNUB_PROXY_NONE). */
    const char* host;

    /** Proxy port number (e.g. 3128 for HTTP, 1080 for SOCKS5). */
    uint16_t port;

    /**
     * Username for proxy authentication (null-terminated, may be
     * @c NULL when auth == @c PUBNUB_PROXY_AUTH_NONE).
     */
    const char* username;

    /**
     * Password for proxy authentication (null-terminated, may be
     * @c NULL when auth == @c PUBNUB_PROXY_AUTH_NONE).
     */
    const char* password;
} pubnub_proxy_config_t;

#ifdef __cplusplus
// clang-format off
}
// clang-format on
#endif

#endif /* PUBNUB_PROXY_H */
