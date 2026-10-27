/* Copyright (c) PubNub Inc. */
/* See LICENSE in the root directory of this source tree. */

/**
 * @file providers/provider_deps.h
 * @brief Shared dependency bag passed to per-context provider init.
 *
 * Per-context providers (transport, serialization, crypto) receive
 * this struct during their init callback so they can access the
 * shared infrastructure providers (allocator, logger, platform).
 *
 * Custom provider authors include this header to implement init.
 */

#ifndef PUBNUB_PROVIDER_DEPS_H
#define PUBNUB_PROVIDER_DEPS_H

#ifdef __cplusplus
// clang-format off
extern "C" {
// clang-format on
#endif

/* Forward-declare so we don't pull in full headers. */
struct pubnub_proxy_config;
struct pubnub_tcp_keepalive_config;

/**
 * @brief Shared infrastructure providers passed to per-context init.
 *
 * Providers may store these references in their extended struct
 * (via first-member embedding) for use during their lifetime.
 * The pointers remain valid for the lifetime of the context.
 *
 * Uses struct tags directly to avoid C99 duplicate-typedef errors
 * regardless of include order.
 */
typedef struct pubnub_provider_deps {
    /** Memory allocator (always non-NULL). */
    struct pubnub_allocator_provider* allocator;
    /** Logger (may be @c NULL if logging is disabled). */
    struct pubnub_logger_provider* logger;
    /** Platform primitives (always non-NULL). */
    struct pubnub_platform_provider* platform;

    /**
     * Proxy configuration (may be @c NULL when no proxy is configured).
     *
     * Points into the owning context's config storage; valid for the
     * lifetime of the context. Transport providers read this during
     * init to configure upstream proxy negotiation.
     */
    const struct pubnub_proxy_config* proxy;

    /**
     * TCP keepalive settings (may be @c NULL when keepalive is disabled).
     *
     * Points into the owning context's config storage; valid for the
     * lifetime of the context. Transport providers copy these values
     * during init and apply them to new connections.
     */
    const struct pubnub_tcp_keepalive_config* tcp_keepalive;

    /**
     * Primary DNS server address string (may be @c NULL).
     *
     * IPv4 ("8.8.8.8") or IPv6 ("2001:4860:4860::8888"). Points into
     * the owning context's shadow storage; valid for the lifetime of
     * the context. Transport providers parse this at init time.
     */
    const char* dns_primary;

    /**
     * Secondary DNS server address string (may be @c NULL).
     *
     * Same format as dns_primary. Used as fallback when the primary
     * server is unreachable.
     */
    const char* dns_secondary;
} pubnub_provider_deps_t;

#ifdef __cplusplus
// clang-format off
}
// clang-format on
#endif

#endif /* PUBNUB_PROVIDER_DEPS_H */
