/* Copyright (c) PubNub Inc. */
/* See LICENSE in the root directory of this source tree. */

/**
 * @file pubnub/json_macros.h
 * @brief Declarative macros for building JSON trees.
 *
 * Provides a thin macro layer over @c pubnub_json_object_build (and
 * the provider's value constructors) so call sites can express small
 * JSON objects in a few lines without writing the boilerplate.
 *
 * Sample usage
 * @code
 * pubnub_json_value_t* msg = PUBNUB_JSON_OBJ(serial,
 *     PUBNUB_JSON_KV_STR(serial, "device", "sensor-1"),
 *     PUBNUB_JSON_KV_INT(serial, "temp_c", 42),
 *     PUBNUB_JSON_KV_BOOL(serial, "alarm", 0));
 * @endcode
 *
 * @pre @c PUBNUB_CFG_JSON_HELPERS flag should be set to @c ON.
 *
 * @see pubnub/json.h
 */

#ifndef PUBNUB_JSON_MACROS_H
#define PUBNUB_JSON_MACROS_H

#include "pubnub/config.h"
#include "pubnub/json.h"

#if PUBNUB_CFG_JSON_HELPERS

#include <stdint.h>
#include <string.h>

/**
 * @brief Build a JSON object from a list of @c PUBNUB_JSON_KV_* pairs.
 *
 * Wraps @c pubnub_json_object_build and auto-appends the @c NULL
 * sentinel that terminates the variadic list.
 *
 * Returns the constructed object on success or @c NULL on failure.
 * On failure all child nodes referenced in the macro arguments are
 * released by the helper (see @c pubnub_json_object_build for the
 * full ownership contract).
 *
 * @param serial Serialization provider expression.
 * @param ...    One or more @c PUBNUB_JSON_KV_* expansions.
 */
#define PUBNUB_JSON_OBJ(serial, ...) \
    pubnub_json_object_build((serial), __VA_ARGS__, (const char*)NULL)

/**
 * @brief Expand to a (key, value) pair carrying a NUL-terminated string.
 *
 * Constructs a JSON string node from the NUL-terminated literal
 * @p str via the provider's @c value_create_string entry, copying the
 * bytes into provider-owned storage. The expansion produces TWO
 * comma-separated arguments suitable for @c PUBNUB_JSON_OBJ:
 *
 *   "key", provider->value_create_string(provider, str, strlen(str))
 *
 * @param serial Serialization provider expression.
 * @param key    NUL-terminated key string literal or @c const @c char*.
 * @param str    NUL-terminated string literal or @c const @c char*.
 *
 * @warning Arguments must be side-effect-free expressions; the macro
 *          may evaluate @p serial and @p str more than once. Pass
 *          lvalues or literals, not function calls or `i++`.
 */
#define PUBNUB_JSON_KV_STR(serial, key, str) \
    (key), (serial)->value_create_string((serial), (str), strlen(str))

/**
 * @brief Expand to a (key, value) pair carrying an integer.
 *
 * The expansion casts @p n to @c int. Values that exceed @c INT_MAX
 * or fall below @c INT_MIN are silently truncated -- callers must
 * ensure the value fits. Timetokens are strings and do not flow
 * through this macro.
 *
 * @param serial Serialization provider expression.
 * @param key    NUL-terminated key string literal or @c const @c char*.
 * @param n      Integer expression. Cast to @c int inside the
 *               expansion.
 *
 * @warning @p serial must be a side-effect-free expression; the macro
 *          evaluates it more than once.
 */
#define PUBNUB_JSON_KV_INT(serial, key, n) \
    (key), (serial)->value_create_int((serial), (int)(n))

/**
 * @brief Expand to a (key, value) pair carrying a boolean.
 *
 * @c b is forwarded as-is; the provider's @c value_create_bool entry
 * accepts any non-zero value as truthy.
 *
 * @param serial Serialization provider expression.
 * @param key    NUL-terminated key string literal or @c const @c char*.
 * @param b      Boolean expression (any non-zero value is truthy).
 *
 * @warning @p serial must be a side-effect-free expression; the macro
 *          evaluates it more than once.
 */
#define PUBNUB_JSON_KV_BOOL(serial, key, b) \
    (key), (serial)->value_create_bool((serial), (b))

/**
 * @brief Expand to a (key, value) pair carrying JSON @c null.
 *
 * @param serial Serialization provider expression.
 * @param key    NUL-terminated key string literal or @c const @c char*.
 *
 * @warning @p serial must be a side-effect-free expression; the macro
 *          evaluates it more than once.
 */
#define PUBNUB_JSON_KV_NULL(serial, key) \
    (key), (serial)->value_create_null((serial))

#endif /* PUBNUB_CFG_JSON_HELPERS */

#if PUBNUB_CFG_JSON_HELPERS && PUBNUB_CFG_JSON_DOUBLE
/**
 * @brief Expand to a (key, value) pair carrying a double-precision float.
 *
 * Only available when @c PUBNUB_CFG_JSON_DOUBLE is enabled (default ON
 * for hosted profiles, OFF for embedded profiles without FP hardware).
 *
 * @param serial Serialization provider expression.
 * @param key    NUL-terminated key string literal or @c const @c char*.
 * @param v      Floating-point expression. Cast to @c double inside
 *               the expansion.
 *
 * @warning @p serial must be a side-effect-free expression; the macro
 *          evaluates it more than once.
 */
#define PUBNUB_JSON_KV_DOUBLE(serial, key, v) \
    (key), (serial)->value_create_double((serial), (double)(v))
#endif /* PUBNUB_CFG_JSON_HELPERS && PUBNUB_CFG_JSON_DOUBLE */

#endif /* PUBNUB_JSON_MACROS_H */
