/* Copyright (c) PubNub Inc. */
/* See LICENSE in the root directory of this source tree. */

/**
 * @file response.h
 * @brief Core-level response accessors for any completed future.
 *
 * Returned views are valid until @c pubnub_future_release.
 */

#ifndef PUBNUB_RESPONSE_H
#define PUBNUB_RESPONSE_H

#include "pubnub/future.h"
#include "pubnub/types.h"

#include <stddef.h>

#ifdef __cplusplus
// clang-format off
extern "C" {
// clang-format on
#endif

/**
 * @brief HTTP status code from the completed response.
 *
 * @param future Future to query (must be in a terminal state).
 * @return HTTP status code (e.g. 200, 403, 500), or 0 if the
 *         future is not complete, is invalid, or no HTTP response
 *         was received (transport-level failure before any server
 *         reply).
 */
PUBNUB_API int pubnub_response_status_code(pubnub_future_t future);

/**
 * @brief Raw response body bytes as a string view (NOT NUL-terminated).
 *
 * @param future Future to query (must be in a terminal state).
 * @return Body view, or `{NULL, 0}` if not complete or no body.
 */
PUBNUB_API pubnub_string_view_t pubnub_response_body(pubnub_future_t future);

/**
 * @brief Extract an error message from a completed response.
 *
 * Shim over @c pubnub_response_service_error -- returns the
 * message field from whichever wire variant the server emitted.
 * For the full envelope use @c pubnub_response_service_error.
 *
 * @param future Future to query (must be in a terminal state).
 * @return Error message view, or `{NULL, 0}` if not extractable.
 */
PUBNUB_API pubnub_string_view_t pubnub_response_error_message(pubnub_future_t future);

#ifdef __cplusplus
// clang-format off
}
// clang-format on
#endif

#endif /* PUBNUB_RESPONSE_H */
