/* Copyright (c) PubNub Inc. */
/* See LICENSE in the root directory of this source tree. */

/**
 * @file providers/transport_types.h
 * @brief Structured HTTP request/response types for the transport provider.
 *
 * These types define the high-level HTTP interface between the SDK core
 * and the transport provider. The transport provider is responsible for
 * secure channel negotiation, connection management, and HTTP framing.
 *
 * Array sizes are governed by PUBNUB_CFG_* compile-time knobs so that
 * embedded profiles can reduce memory footprint.
 */

#ifndef PUBNUB_PROVIDER_TRANSPORT_TYPES_H
#define PUBNUB_PROVIDER_TRANSPORT_TYPES_H

#include "pubnub/config.h"
#include "pubnub/error.h"
#include "pubnub/types.h"

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
// clang-format off
extern "C" {
// clang-format on
#endif

/** Key-value pair using non-owning string views. */
typedef struct pubnub_kv {
    /** Key (borrowed view). */
    pubnub_string_view_t key;
    /** Value (borrowed view). */
    pubnub_string_view_t value;
} pubnub_kv_t;

/** HTTP request method. */
typedef enum pubnub_http_method {
    /** HTTP GET. */
    PUBNUB_HTTP_GET = 0,
    /** HTTP POST. */
    PUBNUB_HTTP_POST = 1,
    /** HTTP PATCH. */
    PUBNUB_HTTP_PATCH = 2,
    /** HTTP DELETE. */
    PUBNUB_HTTP_DELETE = 3
} pubnub_http_method_t;

/**
 * @brief Structured HTTP request descriptor.
 *
 * Populated by the SDK core/features before being handed to the
 * transport provider's send() function. All string views and the
 * body pointer must remain valid until the transport provider signals
 * completion (via poll()) or the request is cancelled.
 *
 * The `scratch` buffer is caller-owned workspace that the SDK uses to
 * build URL-encoded path segments, query values, etc. The transport
 * provider should not modify it.
 */
typedef struct pubnub_http_request {
    /** HTTP method. */
    pubnub_http_method_t method;

    /** Target host (null-terminated borrowed string).
     *
     * For custom ports (debugging), embed the port in this field as
     * "hostname:port" (e.g. "localhost:8080") or bracketed IPv6
     * literal "[::1]:port" (e.g. "[::1]:8080"); the transport
     * parses and extracts the port in both forms.
     */
    const char* host;

    /**
     * @brief Whether to use a secure (TLS) connection for this request.
     *
     * Features set this from the compile-time default
     * (PUBNUB_ENABLE_SECURE_TRANSPORT) or a runtime context setting.
     * The transport uses this to select the connection scheme and
     * default port (e.g. HTTPS/443 vs HTTP/80).
     */
    uint8_t secure;

    /**
     * @brief When non-zero, middlewares skip request decoration.
     *
     * Set for requests targeting external hosts (e.g., S3 presigned
     * uploads) where PubNub auth/pnsdk/userid query params would
     * invalidate the request signature.
     */
    uint8_t external;

    /**
     * @brief When non-zero, transport follows HTTP redirects (3xx).
     *
     * Set by features that expect redirect responses (e.g., file
     * download returns 307 to a presigned S3 URL). Transport providers
     * should follow up to 3 redirects when this flag is set.
     */
    uint8_t follow_redirects;

    /**
     * @brief When non-zero, the compression middleware request body will
     *        be gzip-compressed.
     *
     * Set by features that benefit from body compression (e.g.,
     * publish with large JSON payloads). Zero-initialized default
     * means no compression. Features set this to
     * PUBNUB_ENABLE_REQUEST_COMPRESSION to tie the decision to the
     * compile-time toggle.
     */
    uint8_t compress_body;

    /**
     * @brief When non-zero, the core deadline timer skips this request.
     *
     * Set by the retry middleware during backoff (WAITING state) so
     * the core's per-request timeout does not fire while a retry is
     * pending. Cleared before each redispatch and on cancel.
     * Zero-initialized default means the deadline is active.
     */
    uint8_t deadline_suspended;

    /**
     * URL path segments (joined with '/' by the transport).
     *
     * Segments arrive **pre-encoded** by the feature/middleware layer
     * (percent-encoded per RFC 3986 where needed). Transport
     * implementations MUST NOT re-encode path segments -- concatenate
     * them verbatim with '/' separators.
     */
    pubnub_string_view_t path_segments[PUBNUB_CFG_HTTP_MAX_PATH_SEGMENTS];
    /** Number of populated path segments. */
    unsigned int path_segment_count;

    /**
     * Query parameters (key=value pairs).
     *
     * Values are percent-encoded per RFC 3986 by the SDK middleware
     * before populating this array. Keys are always unreserved ASCII
     * and are stored verbatim (unencoded).
     *
     * Transport implementations MUST NOT re-encode these values —
     * assemble the query string by concatenating key=value pairs
     * separated by '&' without further transformation.
     */
    pubnub_kv_t query_params[PUBNUB_CFG_HTTP_MAX_QUERY_PARAMS];
    /** Number of populated query parameters. */
    unsigned int query_param_count;

    /** Request headers. */
    pubnub_kv_t headers[PUBNUB_CFG_HTTP_MAX_HEADERS];
    /** Number of populated headers. */
    unsigned int header_count;

    /** Scratch buffer for URL encoding / path assembly. */
    char scratch[PUBNUB_CFG_HTTP_SCRATCH_SIZE];
    /** Bytes currently used in the scratch buffer. */
    unsigned int scratch_used;

    /** Request body (may be @c NULL for bodyless methods). */
    const uint8_t* body;
    /** Request body length in bytes. */
    size_t body_len;

    /**
     * @brief Per-request timeout override in milliseconds.
     *
     * Features set this to express how long they are willing to
     * wait for a response (subscribe long-polls want minutes,
     * publish wants seconds). The transport provider should use
     * this value when configuring its wire-level timeout.
     *
     * 0 means "use the build-time default"
     * (@c PUBNUB_CFG_TRANSACTION_TIMEOUT_MS). Callers that do not
     * care about overriding simply leave the field zero-initialised.
     */
    uint32_t timeout_ms;
} pubnub_http_request_t;

/** Transport-level completion status for a response descriptor. */
typedef enum pubnub_http_completion {
    /** Request is still in-flight (not yet completed). */
    PUBNUB_HTTP_PENDING = 0,
    /** Request completed successfully (status_code is valid). */
    PUBNUB_HTTP_COMPLETE = 1,
    /** Request failed due to transport/network error. */
    PUBNUB_HTTP_ERROR = 2
} pubnub_http_completion_t;

/**
 * @brief Structured HTTP response descriptor.
 *
 * Populated by the transport provider after a successful request.
 * The SDK core reads this to process the server response.
 *
 * The `completion` field is set by the transport provider when the
 * request finishes. The SDK core uses this to determine which
 * requests completed after a poll() call. Callers must zero the
 * response struct before passing it to send(); the initial
 * PUBNUB_HTTP_PENDING (0) state is set by the zero-fill.
 *
 * Body ownership: the transport provider owns the body buffer.
 * The body pointer is valid until the next call to send() or
 * cancel() on the same transport handle, or until the provider
 * is destroyed.
 *
 * When @c status_code == 0, the body carries the transport
 * provider's diagnostic detail (or is @c NULL/0 when none is
 * available); see @c providers/transport.h.
 */
typedef struct pubnub_http_response {
    /** Response body (owned by transport provider). */
    const uint8_t* body;
    /** Response body length in bytes. */
    size_t body_len;

    /** Response headers. */
    pubnub_kv_t headers[PUBNUB_CFG_HTTP_MAX_RESP_HEADERS];
    /** Number of populated response headers. */
    unsigned int header_count;

    /** Completion status (set by transport provider). */
    pubnub_http_completion_t completion;

    /** HTTP status code (e.g. 200, 403, 500). Valid when completion == PUBNUB_HTTP_COMPLETE. */
    int status_code;

    /**
     * @brief SDK error code set by the transport when completion ==
     *        PUBNUB_HTTP_ERROR.
     *
     * Distinguishes timeout (PUBNUB_ERR_TIMEOUT), cancellation
     * (PUBNUB_ERR_CANCELLED), and generic network/TLS failure
     * (PUBNUB_ERR_TRANSPORT). Providers SHOULD always set this
     * when reporting PUBNUB_HTTP_ERROR; if left at the zero-init
     * default, the core treats it as PUBNUB_ERR_TRANSPORT.
     */
    pubnub_res_t transport_error;
} pubnub_http_response_t;

#ifdef __cplusplus
// clang-format off
}
// clang-format on
#endif

#endif /* PUBNUB_PROVIDER_TRANSPORT_TYPES_H */
