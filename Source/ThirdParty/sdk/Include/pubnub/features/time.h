/* Copyright (c) PubNub Inc. */
/* See LICENSE in the root directory of this source tree. */

#ifndef PUBNUB_FEATURE_TIME_H
#define PUBNUB_FEATURE_TIME_H

#include "pubnub/config.h"

#if PUBNUB_ENABLE_TIME

#include "pubnub/error.h"
#include "pubnub/future.h"
#include "pubnub/types.h"

#ifdef __cplusplus
// clang-format off
extern "C" {
// clang-format on
#endif

/**
 * @brief Submit a time request to retrieve the current PubNub server
 *        timetoken.
 *
 * Sends an HTTP GET to the PubNub `/time/0` endpoint and returns the
 * server's 17-digit timetoken. The result is a @c pubnub_timetoken_t
 * view accessible via @c pubnub_time_result_timetoken.
 *
 * Drive the returned future via cooperative polling (@c pubnub_process +
 * @c pubnub_future_is_ready), blocking await (@c pubnub_await), or
 * async callback (@c pubnub_async).
 *
 * Cooperative polling example
 * @code
 * pubnub_context_t* ctx = pubnub_create(&cfg);
 * pubnub_future_t fut = pubnub_time(ctx);
 * while (!pubnub_future_is_ready(fut)) {
 *     pubnub_process(ctx);
 * }
 * if (PUBNUB_OK == pubnub_future_status(fut)) {
 *     pubnub_timetoken_t tt = pubnub_time_result_timetoken(fut);
 *     printf("server time: %.*s\n", (int)tt.len, tt.ptr);
 * }
 * pubnub_future_release(fut);
 * pubnub_destroy(ctx);
 * @endcode
 *
 * @note On validation failure (NULL ctx, queue full) the returned future
 *       carries an immediate error code readable via
 *       @c pubnub_future_status.
 * @param ctx Initialized context (@b borrowed).
 * @return Future handle (owned by caller). Release via
 *         @c pubnub_future_release after reading results.
 *
 * @see pubnub_time_result_timetoken
 * @see pubnub_future_release
 * @see pubnub_context_t
 * @see pubnub_future_t
 */
PUBNUB_API pubnub_future_t pubnub_time(pubnub_context_t* ctx);

/**
 * @brief Current server timetoken from a completed time request.
 *
 * The returned view points into storage owned by the future and stays
 * valid until you call @c pubnub_future_release on the same future. It
 * does not alias any transport buffer, so it remains valid even after
 * the connection is closed and its receive buffer is reclaimed.
 *
 * @param future Future returned from @c pubnub_time.
 * @return Timetoken view on success; a zero-initialised view
 *         (`{.ptr = NULL, .len = 0}`) if the future is not ready,
 *         carries an error, or the server response did not parse.
 */
PUBNUB_API pubnub_timetoken_t pubnub_time_result_timetoken(pubnub_future_t future);

#ifdef __cplusplus
// clang-format off
}
// clang-format on
#endif

#endif /* PUBNUB_ENABLE_TIME */

#endif /* PUBNUB_FEATURE_TIME_H */
