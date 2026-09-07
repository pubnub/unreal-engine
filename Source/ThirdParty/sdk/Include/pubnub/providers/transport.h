/* Copyright (c) PubNub Inc. */
/* See LICENSE in the root directory of this source tree. */

/**
 * @file providers/transport.h
 * @brief High-level HTTP transport provider interface.
 *
 * Responsibilities: connection management, TLS negotiation (when
 * PUBNUB_ENABLE_SECURE_TRANSPORT is 1), HTTP I/O, cooperative polling,
 * and HTTP redirect following (when request->follow_redirects is set).
 *
 * Supports middleware chaining: a middleware IS a
 * pubnub_transport_provider_t wrapping another transport (first-member
 * embedding pattern).
 *
 * All callbacks are invoked from normal (non-ISR) context only.
 *
 * On transport-layer failures (status_code == 0), the transport may
 * populate body/body_len with diagnostic text, surfaced verbatim via
 * @c pubnub_response_error_message.
 */

#ifndef PUBNUB_PROVIDER_TRANSPORT_H
#define PUBNUB_PROVIDER_TRANSPORT_H

#include "pubnub/config.h"
#include "pubnub/providers/transport_types.h"
#include "pubnub/providers/provider_deps.h"

#ifdef __cplusplus
// clang-format off
extern "C" {
// clang-format on
#endif

/**
 * @brief Opaque handle for an in-flight transport request.
 *
 * Returned by send() and passed to cancel(). Concrete type is
 * provider-defined.
 */
typedef void pubnub_transport_handle_t;

/**
 * @brief Transport provider function table.
 *
 * Implementers populate this struct and pass it via pubnub_config_t.
 * All function pointers are required unless documented otherwise.
 *
 * This is a **per-context** provider. The SDK calls init() during
 * context initialization and deinit() during context teardown.
 * Each context should have its own transport instance.
 *
 * The `self` pointer passed as the first argument to each function
 * points to this struct instance, enabling middleware decorators to
 * recover their own state (see module-level documentation).
 *
 * Implementation-specific state (connection pools, TLS contexts,
 * etc.) should be stored in an extended struct with this vtable as
 * the first member.
 */
typedef struct pubnub_transport_provider {
    /**
     * @brief Submit an HTTP request.
     *
     * The transport provider takes ownership of driving the request
     * to completion. The caller must keep @p request valid until
     * completion is signalled via poll() or the request is cancelled.
     *
     * Middleware decorators in the chain MAY mutate the request
     * before delegating (e.g. appending query parameters). The
     * request descriptor is therefore non-const at the transport
     * boundary -- the caller must be prepared for the inner
     * transport to see a mutated request.
     *
     * @param self     Pointer to this provider instance.
     * @param request  Structured HTTP request descriptor (borrowed,
     *                 may be mutated by middleware).
     * @param response Response descriptor to populate on completion
     *                 (caller-owned, zeroed by caller before call).
     * @return Opaque transport handle for tracking, or @c NULL on
     *         immediate failure (e.g. resource exhaustion). On
     *         immediate failure, implementations MUST set
     *         response->completion to PUBNUB_HTTP_ERROR before
     *         returning.
     *
     * @see Diagnostic-body convention in the file header.
     */
    pubnub_transport_handle_t* (*send)(struct pubnub_transport_provider* self,
                                       pubnub_http_request_t*  request,
                                       pubnub_http_response_t* response);

    /**
     * @brief Drive I/O and return the number of completed requests.
     *
     * Implementations MUST support timeout_ms=0 as a non-blocking
     * poll for cooperative/no-thread targets.
     *
     * @param self       Pointer to this provider instance.
     * @param timeout_ms Maximum wait time in milliseconds.
     * @return Number of requests that completed during this call,
     *         or a negative value on fatal provider error.
     *
     * @see Diagnostic-body convention in the file header.
     */
    int (*poll)(struct pubnub_transport_provider* self, unsigned int timeout_ms);

    /**
     * @brief Cancel a pending request.
     *
     * After cancel returns, the transport handle is invalid and
     * the associated response descriptor will not be written.
     *
     * @param self             Pointer to this provider instance.
     * @param transport_handle Handle returned by send().
     */
    void (*cancel)(struct pubnub_transport_provider* self,
                   pubnub_transport_handle_t*        transport_handle);

    /**
     * @brief Interrupt a blocking poll() call from another thread.
     *
     * When non-NULL and a background thread is active, called by the
     * user thread after new work is enqueued so the background thread
     * picks it up immediately instead of waiting for the poll timeout.
     *
     * Implementations must be safe to call from any thread at any
     * time, including while poll() is not currently blocking.
     * Spurious wakes must be harmless.
     *
     * Optional: @c NULL means the background thread discovers new
     * work at the next PUBNUB_CFG_MAX_POLL_MS timeout boundary.
     *
     * @param self Pointer to this provider instance.
     */
    void (*wake)(struct pubnub_transport_provider* self);

    /**
     * @brief Per-context initialization.
     *
     * Called by the SDK core after all providers are resolved.
     * The provider may allocate per-context resources using
     * deps->allocator and store dep references in its extended
     * struct for later use.
     *
     * Optional: @c NULL = no per-context init needed.
     *
     * @param self Pointer to this provider instance.
     * @param deps Shared infrastructure providers (valid for
     *             the lifetime of the context).
     * @return 0 on success, non-zero on failure.
     */
    int (*init)(struct pubnub_transport_provider* self,
                const pubnub_provider_deps_t*     deps);

    /**
     * @brief Per-context de-initialization.
     *
     * Called by the SDK core during pubnub_deinit(). Release
     * per-context resources allocated during init.
     *
     * Optional: @c NULL = no cleanup needed.
     *
     * @param self Pointer to this provider instance.
     */
    void (*deinit)(struct pubnub_transport_provider* self);

    /**
     * @brief Update DNS server addresses (optional).
     *
     * Called by pubnub_set_dns_servers() outside the context lock.
     * The primary and secondary strings are valid for the context
     * lifetime (owned copy) or until the next setter call (borrowed).
     * NULL primary means revert to system DNS discovery.
     *
     * Implementations that do not support custom DNS should set this
     * to NULL or return PUBNUB_ERR_NOT_SUPPORTED.
     *
     * @param self      Pointer to this provider instance.
     * @param primary   IPv4 or IPv6 address string, or NULL to clear.
     * @param secondary IPv4 or IPv6 address string, or NULL.
     * @return PUBNUB_OK on success, or PUBNUB_ERR_NOT_SUPPORTED when
     *         the transport cannot honour custom DNS.
     *
     * @note Write ordering: release-store count=0, write server
     *       address structs, then release-store final count.
     *       Readers use acquire-load on the count before dereferencing
     *       the address array. This is safe on all targets including
     *       multi-core ARM and dual-core Xtensa.
     */
    pubnub_res_t (*set_dns_servers)(struct pubnub_transport_provider* self,
                                    const char*                       primary,
                                    const char* secondary);

    /**
     * @brief Replace the CA bundle used for TLS verification.
     *
     * Pass a NUL-terminated PEM certificate chain. Pass NULL to revert
     * to the platform system certificate store. The transport copies
     * the string internally; the caller may free it after this call.
     *
     * Takes effect on connections opened after this call; in-flight
     * connections are unaffected.
     *
     * Optional: NULL means the transport does not support runtime CA
     * replacement.
     *
     * @param self    Pointer to this provider instance.
     * @param ca_pem  PEM certificate chain, or NULL for system certs.
     */
    void (*set_tls_ca_bundle)(struct pubnub_transport_provider* self,
                              const char*                       ca_pem);

    /**
     * @brief Enable or disable TLS peer certificate verification.
     *
     * @warning Disabling verification exposes connections to MITM
     *          attacks. Only use in development and testing.
     *
     * Optional: NULL means the transport does not support runtime
     * verification changes.
     *
     * @param self        Pointer to this provider instance.
     * @param skip_verify Non-zero to skip verification; zero to
     *                    re-enable.
     */
    void (*set_tls_verify)(struct pubnub_transport_provider* self,
                           uint8_t                           skip_verify);
} pubnub_transport_provider_t;

#ifdef __cplusplus
// clang-format off
}
// clang-format on
#endif

#endif /* PUBNUB_PROVIDER_TRANSPORT_H */
