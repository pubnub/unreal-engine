/* Copyright (c) PubNub Inc. */
/* See LICENSE in the root directory of this source tree. */

/**
 * @file future.h
 * @brief Stack-allocatable handle returned by every async SDK API call.
 *
 * A small value type referencing a request slot. Trivially copyable;
 * becomes stale once the underlying slot is released.
 *
 * ## Lifecycle (required pattern)
 *
 * Every future issued by an SDK API call MUST be released exactly once
 * via @c pubnub_future_release — on all platforms and memory models.
 * Release returns the slot to the pool so the context can accept new
 * requests. On embedded targets with a fixed slot count
 * (@c PUBNUB_CFG_MAX_IN_FLIGHT_REQUESTS), failing to release exhausts
 * the pool.
 *
 * Result data (@c pubnub_*_result_* accessors, @c pubnub_response_body,
 * etc.) is valid from the moment the future is ready until
 * @c pubnub_future_release is called. After release, all accessors
 * return zero-initialised values — the generation counter detects the
 * released state and prevents access to freed slot memory.
 *
 * Correct ordering:
 * ```
 * issue → wait/poll → [check status] → [read result] → release
 * ```
 *
 * Three consumption styles are supported:
 *
 * **Cooperative polling (no threads, no sync primitives):**
 * ```c
 * pubnub_future_t fut = pubnub_publish(ctx, opts);
 * while (!pubnub_future_is_ready(fut)) { pubnub_process(ctx); }
 * pubnub_res_t st = pubnub_future_status(fut);
 * pubnub_timetoken_t tok = pubnub_publish_result_timetoken(fut);
 * pubnub_future_release(fut);
 * ```
 *
 * **Blocking await (requires platform sync primitives):**
 * ```c
 * pubnub_future_t fut = pubnub_publish(ctx, opts);
 * pubnub_res_t st = pubnub_await(fut);
 * pubnub_timetoken_t tok = pubnub_publish_result_timetoken(fut);
 * pubnub_future_release(fut);
 * ```
 *
 * **Asynchronous callback:**
 * ```c
 * pubnub_future_t fut = pubnub_publish(ctx, opts);
 * pubnub_async(fut, my_callback, my_data);
 * // Inside callback: read result, then release.
 * ```
 */

#ifndef PUBNUB_FUTURE_H
#define PUBNUB_FUTURE_H

#include "pubnub/error.h"
#include "pubnub/types_fwd.h"

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
// clang-format off
extern "C" {
// clang-format on
#endif

/**
 * @brief Stack-allocatable handle to an in-flight SDK operation.
 *
 * Treat fields as opaque; use @c pubnub_future_is_ready /
 * @c pubnub_future_status for queries.
 */
typedef struct pubnub_future {
    /** Owning context (borrowed). @c NULL for invalid futures. */
    pubnub_context_t* ctx;
    /** Slot index, or PUBNUB_SLOT_ID_INVALID if never bound. */
    uint16_t slot_id;
    /** Generation counter stamped at acquisition; mismatches indicate
     *  a stale future referencing a recycled slot (ABA protection). */
    uint16_t generation;
    /** PUBNUB_IN_PROGRESS while active; immediate error if slot
     *  acquisition failed. */
    pubnub_res_t status;
} pubnub_future_t;

/** @brief Sentinel slot index for futures never bound to a pool slot. */
#define PUBNUB_SLOT_ID_INVALID ((uint16_t)UINT16_MAX)

/** @brief Sentinel for validation failures (is_ready = true, status =
 * INVALID_ARGUMENT). */
#define PUBNUB_FUTURE_INVALID                                \
    ((pubnub_future_t){.ctx        = NULL,                   \
                       .slot_id    = PUBNUB_SLOT_ID_INVALID, \
                       .generation = 0,                      \
                       .status     = PUBNUB_ERR_INVALID_ARGUMENT})

/**
 * @brief Callback type for asynchronous future completion.
 *
 * Invoked exactly once when the future reaches a terminal state, including
 * during context teardown (@c pubnub_destroy / @c pubnub_deinit).
 * The callback fires outside any SDK-internal lock. Safe to call
 * @c pubnub_future_release from within this callback.
 *
 * @note When @p status is @c PUBNUB_ERR_CANCELLED (context teardown or
 * explicit cancellation), result data is unavailable. Do not call result
 * accessors (@c pubnub_publish_result_timetoken, etc.) — they return
 * zero-initialized values. Use the callback only for releasing user-side
 * resources.
 *
 * @param future    The completed future (by value).
 * @param status    Final SDK result code.
 * @param user_data Opaque pointer passed to @c pubnub_async.
 */
typedef void (*pubnub_async_cb_t)(pubnub_future_t future,
                                  pubnub_res_t    status,
                                  void*           user_data);

/**
 * @brief Test whether a future has reached a terminal state.
 *
 * @param future Future to query.
 * @return @c true if complete/failed/cancelled/invalid, @c false otherwise.
 */
PUBNUB_API bool pubnub_future_is_ready(pubnub_future_t future);

/**
 * @brief Read the current status of a future.
 *
 * Returns PUBNUB_IN_PROGRESS while active, or the final result code.
 *
 * @param future Future to query.
 * @return Current status code.
 * @note Reading result data (service error, timetoken, etc.) from a
 *       completed future concurrently from multiple threads is safe —
 *       the atomic acquire barrier on the lazy-init classification
 *       flag ensures parsed data is visible before the flag. Sharing
 *       an in-progress future across threads requires caller-level
 *       synchronization until the future reaches a terminal state.
 */
PUBNUB_API pubnub_res_t pubnub_future_status(pubnub_future_t future);

/**
 * @brief Release the slot referenced by @p future back to the pool.
 *
 * Call exactly once after reading result data. Safe to call from
 * inside a completion callback (release is deferred until after the
 * callback returns). Also safe to call on an in-flight future; the
 * slot is not reset while the transport is active — reclamation is
 * deferred until the operation completes and the next process tick
 * runs. Invalid futures and pending-queue futures are handled
 * gracefully.
 *
 * @param future Future to release.
 */
PUBNUB_API void pubnub_future_release(pubnub_future_t future);

/**
 * @brief Block until @p future completes or the configured timeout
 *        expires.
 *
 * When a background thread is running (PUBNUB_CFG_THREAD_SAFETY=1),
 * polls @c pubnub_future_is_ready with @c platform->sleep_ms
 * yielding between checks.
 *
 * On cooperative targets (no background thread), drives I/O via
 * @c pubnub_process internally with @c PUBNUB_CFG_MAX_POLL_MS
 * blocking in @c transport->poll() per iteration.
 *
 * Returns immediately when the future is already terminal or carries
 * an immediate error (ctx == NULL or status != PUBNUB_IN_PROGRESS).
 *
 * @param future Future to await.
 * @return Final status code from the completed future.
 */
PUBNUB_API pubnub_res_t pubnub_await(pubnub_future_t future);

/**
 * @brief Register an asynchronous completion callback on @p future.
 *
 * The callback fires exactly once when the future transitions to a
 * terminal state. If the future is already complete at call time,
 * the callback fires immediately (inline, outside any lock).
 *
 * Only one callback may be registered per future. Calling pubnub_async
 * on a future that already has a callback registered is undefined.
 *
 * @param future    Future to observe.
 * @param callback  Completion handler (must be non-NULL).
 * @param user_data Opaque pointer forwarded to @p callback.
 * @retval PUBNUB_OK Callback registered or fired immediately (future is valid
 *         and non-NULL callback provided).
 * @retval PUBNUB_ERR_INVALID_ARGUMENT @p callback is @c NULL, or @p future
 *         carries a @c NULL context (invalid issue sentinel).
 */
PUBNUB_API pubnub_res_t pubnub_async(pubnub_future_t   future,
                                     pubnub_async_cb_t callback,
                                     void*             user_data);

/**
 * @brief Cancel a pending or in-flight request.
 *
 * If the future is pending (waiting in the queue), it is removed and
 * the completion callback (if any) is invoked with
 * @c PUBNUB_ERR_CANCELLED. If the future is in-flight, the transport
 * is signalled to cancel; the completion callback fires when
 * cancellation is confirmed. Safe to call from within a completion
 * callback.
 *
 * The slot is NOT released by this call. After the completion callback
 * fires (or after @c pubnub_future_is_ready returns @c true), call
 * @c pubnub_future_release to return the slot to the pool.
 *
 * In cooperative mode (no background thread), the cancel is serviced on
 * your next @c pubnub_process or @c pubnub_await tick: the transport
 * teardown, terminal transition, and completion callback are all deferred
 * to that tick, and the future stays @c PUBNUB_IN_PROGRESS until then.
 *
 * @param future The future identifying the request to cancel.
 * @retval PUBNUB_OK                  Cancel was dispatched successfully.
 * @retval PUBNUB_ERR_INVALID_ARGUMENT @p future is invalid (NULL context
 *                                    or unresolvable slot).
 * @retval PUBNUB_IN_PROGRESS         The future is already in a terminal
 *                                    state; no cancel was needed.
 */
PUBNUB_API pubnub_res_t pubnub_future_cancel(pubnub_future_t future);

#ifdef __cplusplus
// clang-format off
}
// clang-format on
#endif

#endif /* PUBNUB_FUTURE_H */
