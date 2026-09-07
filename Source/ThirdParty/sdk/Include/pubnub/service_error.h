/* Copyright (c) PubNub Inc. */
/* See LICENSE in the root directory of this source tree. */

/**
 * @file service_error.h
 * @brief Normalized service-error accessors for completed responses.
 *
 * Folds the server's variant error envelope shapes into a single
 * flat @c pubnub_service_error_t with universal iterators for
 * validation details and affected channels.
 *
 * All returned views are valid until @c pubnub_future_release on
 * the same future. Classification is cached: repeated calls are O(1).
 */

#ifndef PUBNUB_SERVICE_ERROR_H
#define PUBNUB_SERVICE_ERROR_H

#include "pubnub/error.h"
#include "pubnub/future.h"
#include "pubnub/types.h"

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
// clang-format off
extern "C" {
// clang-format on
#endif

/**
 * @brief Folded service-error envelope (caller-allocated, no heap).
 *
 * Fields a variant cannot supply are zero-initialized.
 * Views are valid until @c pubnub_future_release.
 */
typedef struct pubnub_service_error {
    /** HTTP-equivalent body status, or 0 if no response received. */
    uint16_t status;

    /** 1 when the body unambiguously signals failure. */
    uint8_t error_flag;

    /** Reserved for alignment; do not read. */
    uint8_t _pad;

    /** Endpoint-specific sub-code (Files only today), or 0. */
    int32_t code;

    /** Server-assigned service tag, or empty. */
    pubnub_string_view_t service;

    /** Human-readable error message, or empty. NOT NUL-terminated. */
    pubnub_string_view_t message;

    /** Error source subsystem (PAM-style only), or empty. */
    pubnub_string_view_t source;
} pubnub_service_error_t;

/**
 * @brief One entry in a validation-details list.
 */
typedef struct pubnub_service_error_detail {
    /** Per-detail message text. */
    pubnub_string_view_t message;

    /** Dotted path of the offending field, or empty. */
    pubnub_string_view_t location;
} pubnub_service_error_detail_t;

/**
 * @brief Extract a normalized service-error view from a completed future.
 *
 * @param future Future to query (@b required, must be in a terminal state).
 * @param out    Caller-owned output envelope to populate (@b required).
 * @return PUBNUB_OK, PUBNUB_IN_PROGRESS, or an error code.
 */
PUBNUB_API pubnub_res_t pubnub_response_service_error(pubnub_future_t future,
                                                      pubnub_service_error_t* out);

/**
 * @brief Number of validation-detail entries on the response.
 *
 * @param future Future to query (must be in a terminal state).
 * @return Number of detail entries, or 0.
 */
PUBNUB_API size_t pubnub_service_error_detail_count(pubnub_future_t future);

/**
 * @brief Read a single detail entry by index.
 *
 * @param future Future to query (@b required, must be in a terminal state).
 * @param index  Zero-based index, must be less than
 *               @c pubnub_service_error_detail_count.
 * @param out    Caller-owned output struct (@b required).
 * @return PUBNUB_OK on success; PUBNUB_ERR_INVALID_ARGUMENT when
 *         @p out is NULL, the future is invalid, or @p index is
 *         out of range.
 */
PUBNUB_API pubnub_res_t
pubnub_service_error_detail_at(pubnub_future_t                future,
                               size_t                         index,
                               pubnub_service_error_detail_t* out);

/**
 * @brief Number of affected-channel entries on the response.
 *
 * @param future Future to query (must be in a terminal state).
 * @return Number of channel entries, or 0.
 */
PUBNUB_API size_t pubnub_service_error_channel_count(pubnub_future_t future);

/**
 * @brief Read a single channel name by index.
 *
 * @param future Future to query (must be in a terminal state).
 * @param index  Zero-based index.
 * @return Channel-name view, or `{NULL, 0}` on out-of-range/invalid.
 *
 * @note O(N) per call on the history object-keyed variant; cache
 *       results when iterating many entries.
 */
PUBNUB_API pubnub_string_view_t pubnub_service_error_channel_at(pubnub_future_t future,
                                                                size_t index);

#ifdef __cplusplus
// clang-format off
}
// clang-format on
#endif

#endif /* PUBNUB_SERVICE_ERROR_H */
