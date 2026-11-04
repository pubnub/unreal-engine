/* Copyright (c) PubNub Inc. */
/* See LICENSE in the root directory of this source tree. */

/**
 * @file tcp_keepalive.h
 * @brief TCP keepalive configuration for transport providers.
 *
 * Controls TCP-level keepalive probes on sockets managed by the SDK's
 * transport layer. Embedded as a value type in `pubnub_config_t`;
 * transport providers read the config during init and apply the
 * settings to every new connection.
 */

#ifndef PUBNUB_TCP_KEEPALIVE_H
#define PUBNUB_TCP_KEEPALIVE_H

#include <stdint.h>

#ifdef __cplusplus
// clang-format off
extern "C" {
// clang-format on
#endif

/**
 * @brief TCP keepalive configuration.
 *
 * Controls TCP keepalive probe behavior on transport sockets. When
 * enabled, the OS sends keepalive probes after @c idle_sec seconds
 * of inactivity, with @c interval_sec between probes, up to
 * @c probe_count probes before declaring the connection dead.
 *
 * Zero-initialized (`= {0}`) disables keepalive entirely (enabled=0).
 * Use @c PUBNUB_TCP_KEEPALIVE_CONFIG_INIT for sensible defaults.
 *
 * @note Immutable after context initialization. Transport providers
 *       copy this value during init; subsequent changes have no effect.
 */
typedef struct pubnub_tcp_keepalive_config {
    /** 1 = keepalive enabled, 0 = disabled. */
    uint8_t enabled;

    /** Seconds of idle before first probe. */
    uint32_t idle_sec;

    /** Seconds between probes. */
    uint32_t interval_sec;

    /** Number of probes before declaring connection dead. */
    uint32_t probe_count;
} pubnub_tcp_keepalive_config_t;

/**
 * @brief Default TCP keepalive configuration.
 *
 * Enabled with 60s idle, 20s interval, 3 probes. These values match
 * common Linux/BSD OS defaults and provide reasonable dead-peer
 * detection for PubNub's long-lived subscribe connections.
 */
#define PUBNUB_TCP_KEEPALIVE_CONFIG_INIT {1, 60, 20, 3}

#ifdef __cplusplus
// clang-format off
}
// clang-format on
#endif

#endif /* PUBNUB_TCP_KEEPALIVE_H */
