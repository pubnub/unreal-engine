/* Copyright (c) PubNub Inc. */
/* See LICENSE in the root directory of this source tree. */

/**
 * @file pubnub/json.h
 * @brief Ergonomic helpers for the universal JSON value tree.
 *
 * Pure call-throughs over the serialization vtable. Always-on:
 * @c pubnub_json_object_set, @c pubnub_json_array_append,
 * @c pubnub_json_destroy. Gated on @c PUBNUB_CFG_JSON_HELPERS:
 * @c pubnub_json_object_build, @c pubnub_json_clone,
 * @c pubnub_json_to_debug_string.
 *
 * Trees handed to an SDK feature are consumed (SDK frees them).
 * Trees never handed off must be released with @c pubnub_json_destroy.
 */

#ifndef PUBNUB_JSON_H
#define PUBNUB_JSON_H

#include "pubnub/config.h"
#include "pubnub/pubnub_compat.h"
#include "pubnub/providers/serialization.h"

#include <stddef.h>

#ifdef __cplusplus
// clang-format off
extern "C" {
// clang-format on
#endif

/**
 * @brief Attach a child value under a NUL-terminated key.
 *
 * Convenience wrapper around @c serial->object_set that computes the
 * key length via @c strlen on the caller's behalf. The provider COPIES
 * the key bytes; the caller's @p key buffer may be released or reused
 * after return.
 *
 * Ownership of @p child transfers to @p obj on success
 * (return @c PUBNUB_OK). On failure the caller continues to own
 * @p child and must release it via @c pubnub_json_destroy.
 *
 * @param serial Serialization provider; must be non-NULL.
 * @param obj    Target object node; must be non-NULL and of type
 *               @c PUBNUB_JSON_OBJECT.
 * @param key    NUL-terminated key string; must be non-NULL.
 * @param child  Caller-owned node to attach; must be non-NULL.
 * @return @c PUBNUB_OK on success;
 *         @c PUBNUB_ERR_INVALID_ARGUMENT when any required argument is
 *         @c NULL or @c object_set is unavailable on the active backend;
 *         a capacity-class error from the underlying mutator on
 *         allocation failure.
 *
 * @note The disabled-path of @c PUBNUB_CFG_JSON_HELPERS does NOT
 *       affect this helper -- it always compiles.
 */
PUBNUB_API pubnub_res_t pubnub_json_object_set(pubnub_serialization_provider_t* serial,
                                               pubnub_json_value_t* obj,
                                               const char*          key,
                                               pubnub_json_value_t* child);

/**
 * @brief Append an item to a JSON array.
 *
 * Direct passthrough to @c serial->array_append. Provided for grep
 * symmetry with @c pubnub_json_object_set so call sites can use
 * one naming convention across mutators.
 *
 * @param serial Serialization provider; must be non-NULL.
 * @param arr    Target array node; must be non-NULL and of type
 *               @c PUBNUB_JSON_ARRAY.
 * @param item   Caller-owned node to attach; must be non-NULL.
 * @retval PUBNUB_OK on success;
 * @retval PUBNUB_ERR_INVALID_ARGUMENT when any required argument is
 *         @c NULL or @c array_append is unavailable on the active backend.
 */
PUBNUB_API pubnub_res_t pubnub_json_array_append(pubnub_serialization_provider_t* serial,
                                                 pubnub_json_value_t* arr,
                                                 pubnub_json_value_t* item);

/**
 * @brief Release a JSON value tree.
 *
 * Wraps @c serial->value_destroy with explicit caller-side @c NULL
 * tolerance: when either @p serial or @p value is @c NULL the call is a
 * no-op. The wrap exists so user code that builds-then-cancels a tree
 * has a single, explicit cleanup spelling without reaching into the
 * vtable.
 *
 * @param serial Serialization provider (@c NULL is a no-op).
 * @param value  Tree root to free (@c NULL is a no-op).
 *
 * @warning Do NOT call this on a tree that has already been handed to an
 *          SDK feature -- features take ownership at the call site and
 *          release the tree internally.
 */
PUBNUB_API void pubnub_json_destroy(pubnub_serialization_provider_t* serial,
                                    pubnub_json_value_t*             value);

#if PUBNUB_CFG_JSON_HELPERS

/**
 * @brief Build a JSON object from a sentinel-terminated argument list.
 *
 * Variadic args are (key, child) pairs; @c NULL key terminates. On
 * success all children transfer to the returned object. On failure
 * all children in the list are freed -- caller references are always
 * invalidated regardless of outcome.
 *
 * @param serial Serialization provider (non-NULL).
 * @param ...    Sentinel-terminated key/child pairs.
 * @return Built object, or @c NULL on failure.
 *
 * @see @c PUBNUB_JSON_OBJ in pubnub/json_macros.h for the convenient macro.
 */
PUBNUB_API pubnub_json_value_t*
pubnub_json_object_build(pubnub_serialization_provider_t* serial, ...);

/**
 * @brief Deep-copy a JSON value tree.
 *
 * Strings are copied (not aliased); the clone is safe to outlive
 * the source. Partial copies are freed on allocation failure.
 *
 * @param serial Serialization provider (non-NULL).
 * @param src    Source tree (@c NULL returns @c NULL).
 * @return Caller-owned deep copy, or @c NULL on failure.
 */
PUBNUB_API pubnub_json_value_t* pubnub_json_clone(pubnub_serialization_provider_t* serial,
                                                  const pubnub_json_value_t* src);

/**
 * @brief Serialize a value to a NUL-terminated debug string.
 *
 * Convenience wrapper around @c serial->serialize that NUL-terminates
 * the output and returns a printf-friendly byte count. The returned
 * count excludes the trailing NUL.
 *
 * @param serial Serialization provider; must be non-NULL.
 * @param value  Tree to serialize; must be non-NULL.
 * @param buf    Caller-provided output buffer; must be non-NULL.
 * @param cap    Capacity of @p buf in bytes; must be >= 1 to leave
 *               room for the NUL terminator.
 * @return Number of bytes written excluding the trailing NUL on
 *         success, or 0 on failure (@c NULL argument, oversized output,
 *         backend error). On failure @p buf[0] is set to NUL when
 *         @p cap >= 1 and @p buf is non-NULL.
 *
 * @note The resulting string is intended for debug output. Users that
 *       need to send bytes on the wire should call
 *       @c serial->serialize directly to avoid the NUL-termination
 *       overhead.
 */
PUBNUB_API size_t pubnub_json_to_debug_string(pubnub_serialization_provider_t* serial,
                                              const pubnub_json_value_t* value,
                                              char*                      buf,
                                              size_t                     cap);

#endif /* PUBNUB_CFG_JSON_HELPERS */

#ifdef __cplusplus
// clang-format off
}
// clang-format on
#endif

#endif /* PUBNUB_JSON_H */
