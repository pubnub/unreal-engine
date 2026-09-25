/* Copyright (c) PubNub Inc. */
/* See LICENSE in the root directory of this source tree. */

/**
 * @file error.h
 * @brief Error codes and status types for the PubNub C SDK.
 *
 * Result values are range-coded by class so that future additions can
 * land in their class without disturbing already-published numeric
 * values. Values are stable within a major version.
 *
 * Class layout:
 * - 0..15   completion-style outcomes (success, in-progress, cancelled)
 * - 16..31  argument / lifecycle
 * - 32..47  memory / capacity
 * - 48..63  timing
 * - 64..79  transport / network (TLS failures fold into this class)
 * - 80..95  server-reported errors
 * - 96..111 payload (serialization, crypto)
 * - 240..255 invariant breaks (internal SDK bugs)
 */

#ifndef PUBNUB_ERROR_H
#define PUBNUB_ERROR_H

#include "pubnub/pubnub_compat.h"

#ifdef __cplusplus
// clang-format off
extern "C" {
// clang-format on
#endif

/**
 * @brief Result/error codes returned by SDK functions.
 *
 * Values are stable within a major version for ABI compatibility.
 */
typedef enum pubnub_res {
    /* 0..15 : completion-style outcomes */

    /** Operation completed successfully. */
    PUBNUB_OK = 0,

    /** Operation is still in progress (non-blocking). */
    PUBNUB_IN_PROGRESS = 1,

    /** Operation was cancelled. */
    PUBNUB_ERR_CANCELLED = 2,

    /* 16..31 : argument / lifecycle */

    /** Invalid argument supplied by caller. */
    PUBNUB_ERR_INVALID_ARGUMENT = 16,

    /** Client context is not initialized. */
    PUBNUB_ERR_NOT_INITIALIZED = 17,

    /** A required provider was not supplied and no compiled-in default exists. */
    PUBNUB_ERR_PROVIDER_MISSING = 18,

    /** Feature is not enabled in this build. */
    PUBNUB_ERR_NOT_SUPPORTED = 19,

    /* 32..47 : memory / capacity */

    /** Memory allocation failure. */
    PUBNUB_ERR_OUT_OF_MEMORY = 32,

    /** Buffer too small for the requested operation. */
    PUBNUB_ERR_BUFFER_TOO_SMALL = 33,

    /** Maximum in-flight or pending request limit reached. */
    PUBNUB_ERR_QUEUE_FULL = 34,

    /* 48..63 : timing */

    /** Request timed out. */
    PUBNUB_ERR_TIMEOUT = 48,

    /** Wall-clock time unavailable -- required for PAM signing. */
    PUBNUB_ERR_NO_WALL_CLOCK = 49,

    /* 64..79 : transport / network (TLS folded in) */

    /** Transport / network failure. Includes TLS handshake and
     *  certificate-verification failures; diagnostic detail surfaces
     *  via @c pubnub_response_error_message(). */
    PUBNUB_ERR_TRANSPORT = 64,

    /* 80..95 : server */

    /** Server returned an error response. */
    PUBNUB_ERR_SERVER = 80,

    /* 96..111 : payload */

    /** Serialization / deserialization failure. */
    PUBNUB_ERR_SERIALIZATION = 96,

    /** Crypto operation failure. */
    PUBNUB_ERR_CRYPTO = 97,

    /* 240..255 : invariant breaks */

    /** Internal/unspecified SDK invariant break. */
    PUBNUB_ERR_INTERNAL = 240
} pubnub_res_t;

/**
 * @brief Return a static, NUL-terminated string for @p res.
 *
 * @param res Any @c pubnub_res_t value, including unknown values
 *            from forward-compatibility scenarios where new headers
 *            are linked against an older binary.
 * @return Static string, never @c NULL. When @c PUBNUB_CFG_RES_STR
 *         is @c 1, returns @c "Unknown error" for values outside the
 *         committed set. When @c 0, returns @c "" for every input.
 *
 * @note The function is always linkable, but the per-value label table is
 * compiled out when @c PUBNUB_CFG_RES_STR is 0 - in that build the function
 * returns the empty string @c "" for every input. This lets diagnostic call
 * sites compile cleanly across build configurations.
 */
PUBNUB_API const char* pubnub_res_str(pubnub_res_t res);

#ifdef __cplusplus
// clang-format off
}
// clang-format on
#endif

#endif /* PUBNUB_ERROR_H */
