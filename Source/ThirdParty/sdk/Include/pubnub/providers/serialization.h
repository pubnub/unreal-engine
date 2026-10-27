/* Copyright (c) PubNub Inc. */
/* See LICENSE in the root directory of this source tree. */

/**
 * @file providers/serialization.h
 * @brief Serialization provider interface (JSON / alternative backends).
 *
 * Owns the universal `pubnub_json_value_t` data model. Vtable groups:
 *   - Lifecycle: init/deinit (optional)
 *   - Wire I/O: parse, serialize (mandatory)
 *   - Constructors: value_create_* (optional, @c NULL = not supported)
 *   - Mutators: object_set, array_append, etc. (optional)
 *   - Accessors: value_type, value_as_*, object_get, etc. (optional)
 *   - Destructor: value_destroy (mandatory)
 *
 * All optional methods may be @c NULL -- callers MUST check before invoking.
 * All callbacks from non-ISR context only.
 */

#ifndef PUBNUB_PROVIDER_SERIALIZATION_H
#define PUBNUB_PROVIDER_SERIALIZATION_H

#include "pubnub/config.h"
#include "pubnub/error.h"
#include "pubnub/providers/provider_deps.h"

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
// clang-format off
extern "C" {
// clang-format on
#endif

/** Opaque JSON value handle. Concrete layout is backend-specific. */
typedef struct pubnub_json_value pubnub_json_value_t;

/**
 * @brief Type tag for `pubnub_json_value_t` nodes.
 *
 * Returned by the @c value_type accessor. Mirrors JSON's value
 * taxonomy plus a @c RAW variant for verbatim pre-serialized bytes
 * (see @c value_create_raw).
 */
typedef enum pubnub_json_type {
    /** JSON null literal. */
    PUBNUB_JSON_NULL = 0,

    /** JSON boolean (true / false). */
    PUBNUB_JSON_BOOL = 1,

    /** JSON integer; int-precision in the SDK's data model. */
    PUBNUB_JSON_INT = 2,

    /** JSON floating-point. The enum value is always present for ABI
     *  stability; producing and consuming this type is gated by
     *  `PUBNUB_CFG_JSON_DOUBLE` (see config.h). */
    PUBNUB_JSON_DOUBLE = 3,

    /** JSON string. */
    PUBNUB_JSON_STRING = 4,

    /** JSON array. */
    PUBNUB_JSON_ARRAY = 5,

    /** JSON object. */
    PUBNUB_JSON_OBJECT = 6,

    /** Pre-serialized verbatim bytes; emitted unchanged by
     *  @c serialize regardless of backend. */
    PUBNUB_JSON_RAW = 7
} pubnub_json_type_t;

/**
 * @brief Opaque iterator for object key/value walks.
 *
 * Caller-owned POD struct, stack-allocatable. The provider fills the
 * opaque storage in @c object_iter_init. The 16-byte payload is sized
 * to accommodate various backend state representations and is verified
 * by each backend via `PUBNUB_STATIC_ASSERT` to ensure it fits.
 *
 * @note Iterators are invalidated by ANY mutation of the underlying
 *       object. The SDK does not enforce this at runtime; document
 *       and rely on caller discipline.
 *
 * @note Do not access @c opaque directly. The contents are private
 *       to the provider that initialized the iterator.
 */
typedef struct pubnub_json_iter {
    /** Provider-private storage; do not access directly. */
    uint8_t opaque[16];
} pubnub_json_iter_t;

/**
 * @brief Opaque iterator for array element walks.
 *
 * Caller-owned POD struct, stack-allocatable. The provider fills the
 * opaque storage in @c array_iter_init. The 16-byte payload is sized
 * to accommodate various backend state representations and is verified
 * by each backend via `PUBNUB_STATIC_ASSERT` to ensure it fits.
 *
 * @note Iterators are invalidated by ANY mutation of the underlying
 *       array. The SDK does not enforce this at runtime; document
 *       and rely on caller discipline.
 *
 * @note Do not access @c opaque directly. The contents are private
 *       to the provider that initialized the iterator.
 */
typedef struct pubnub_json_array_iter {
    /** Provider-private storage; do not access directly. */
    uint8_t opaque[16];
} pubnub_json_array_iter_t;

/**
 * @brief Serialization provider function table.
 *
 * Per-context provider: SDK calls init/deinit during context lifecycle.
 * Store implementation state in an extended struct with this vtable
 * as the first member.
 */
typedef struct pubnub_serialization_provider {
    /**
     * @brief Parse a JSON document from a buffer.
     *
     * @param self Pointer to this provider instance.
     * @param data Input buffer (not necessarily null-terminated).
     * @param len  Input length in bytes.
     * @return Parsed value handle owned by the caller, or @c NULL on
     *         parse error or allocation failure. Release with
     *         @c value_destroy.
     */
    pubnub_json_value_t* (*parse)(struct pubnub_serialization_provider* self,
                                  const uint8_t*                        data,
                                  size_t                                len);

    /**
     * @brief Serialize a value to a caller-provided buffer.
     *
     * Nodes constructed via @c value_create_raw are emitted verbatim;
     * all other node types are formatted according to JSON syntax
     * rules.
     *
     * @param self    Pointer to this provider instance.
     * @param value   Value to serialize.
     * @param buf     Output buffer.
     * @param buf_len Output buffer capacity in bytes.
     * @param out_len Receives the number of bytes written on success.
     * @return PUBNUB_OK on success, or a payload-class error on
     *         insufficient buffer / serialization failure.
     */
    pubnub_res_t (*serialize)(struct pubnub_serialization_provider* self,
                              const pubnub_json_value_t*            value,
                              uint8_t*                              buf,
                              size_t                                buf_len,
                              size_t*                               out_len);

    /**
     * @brief Free a parsed or constructed value tree.
     *
     * Walks the tree recursively. Releasing a child node that has
     * already been transferred to a parent (via @c object_set or
     * @c array_append SUCCESS) is a use-after-free and undefined.
     *
     * @param self  Pointer to this provider instance.
     * @param value Tree root to free; @c NULL is a no-op.
     */
    void (*value_destroy)(struct pubnub_serialization_provider* self,
                          pubnub_json_value_t*                  value);

    /**
     * @brief Per-context initialization.
     *
     * Called by the SDK core after all providers are resolved.
     * The provider may allocate per-context resources using
     * deps->allocator.
     *
     * Optional: @c NULL = no per-context init needed.
     *
     * @param self Pointer to this provider instance.
     * @param deps Shared infrastructure providers.
     * @return 0 on success, non-zero on failure.
     */
    int (*init)(struct pubnub_serialization_provider* self,
                const pubnub_provider_deps_t*         deps);

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
    void (*deinit)(struct pubnub_serialization_provider* self);

    /**
     * @brief Construct an empty JSON object node.
     *
     * @param self Pointer to this provider instance.
     * @return Caller-owned node, or @c NULL on allocation failure.
     *         May be @c NULL when the backend does not support
     *         tree construction; callers must check.
     */
    pubnub_json_value_t* (*value_create_object)(
        struct pubnub_serialization_provider* self);

    /**
     * @brief Construct an empty JSON array node.
     *
     * @param self Pointer to this provider instance.
     * @return Caller-owned node, or @c NULL on allocation failure.
     */
    pubnub_json_value_t* (*value_create_array)(struct pubnub_serialization_provider* self);

    /**
     * @brief Construct a JSON null literal node.
     *
     * @param self Pointer to this provider instance.
     * @return Caller-owned node, or @c NULL on allocation failure.
     */
    pubnub_json_value_t* (*value_create_null)(struct pubnub_serialization_provider* self);

    /**
     * @brief Construct a JSON boolean node.
     *
     * @param self   Pointer to this provider instance.
     * @param truthy 0 produces JSON @c false; any non-zero value
     *               produces JSON @c true.
     * @return Caller-owned node, or @c NULL on allocation failure.
     */
    pubnub_json_value_t* (*value_create_bool)(struct pubnub_serialization_provider* self,
                                              int truthy);

    /**
     * @brief Construct a JSON integer node.
     *
     * The SDK's integer data model uses @c int (at least 32 bits on all
     * targets). PubNub timetokens are 17-digit decimal strings and do
     * not flow through the integer API.
     *
     * @param self Pointer to this provider instance.
     * @param v    Signed integer value.
     * @return Caller-owned node, or @c NULL on allocation failure.
     */
    pubnub_json_value_t* (*value_create_int)(struct pubnub_serialization_provider* self,
                                             int v);

    /**
     * @brief Construct a JSON floating-point node.
     *
     * May be @c NULL -- caller must check before invoking. Set to @c NULL
     * when @c PUBNUB_CFG_JSON_DOUBLE is 0 at compile time, or when
     * the backend does not support floating-point values
     * (e.g., bare-metal targets without an FPU).
     *
     * @param self Pointer to this provider instance.
     * @param v    IEEE-754 double value. NaN / infinity handling is
     *             backend-defined.
     * @return Caller-owned node, or @c NULL on allocation failure.
     */
    pubnub_json_value_t* (*value_create_double)(struct pubnub_serialization_provider* self,
                                                double v);

    /**
     * @brief Construct a JSON string node by COPYING the input bytes.
     *
     * The provider duplicates @p str into provider-owned storage; the
     * caller's buffer may be released or reused immediately after
     * return. JSON-escape encoding happens at @c serialize time, not
     * at construction.
     *
     * @param self Pointer to this provider instance.
     * @param str  Source bytes (need not be NUL-terminated).
     * @param len  Length of @p str in bytes (excludes any NUL).
     * @return Caller-owned node, or @c NULL on allocation failure.
     */
    pubnub_json_value_t* (*value_create_string)(struct pubnub_serialization_provider* self,
                                                const char* str,
                                                size_t      len);

    /**
     * @brief Construct a non-copying JSON string node.
     *
     * May be @c NULL (caller must check). Caller MUST keep @p str alive
     * for the tree's lifetime. @c NULL return means "alias not
     * available; use value_create_string instead".
     *
     * @param self Pointer to this provider instance.
     * @param str  Source bytes (caller-owned, must outlive tree).
     * @param len  Length in bytes (0 = caller asserts NUL-terminated).
     * @return Caller-owned node, or @c NULL.
     */
    pubnub_json_value_t* (*value_create_string_view)(
        struct pubnub_serialization_provider* self,
        const char*                           str,
        size_t                                len);

    /**
     * @brief Construct a node wrapping pre-formatted JSON bytes (copied).
     *
     * Emitted verbatim on serialize (byte-stable round-trip).
     *
     * @param self  Pointer to this provider instance.
     * @param bytes Pre-formatted JSON (must be valid; not re-validated).
     * @param len   Length (must be > 0).
     * @return Caller-owned node, or @c NULL on failure.
     */
    pubnub_json_value_t* (*value_create_raw)(struct pubnub_serialization_provider* self,
                                             const uint8_t* bytes,
                                             size_t         len);

    /**
     * @brief Insert or replace a key in a JSON object.
     *
     * If @p key already exists in @p obj, the prior child is freed
     * via @c value_destroy before @p child is attached. If @p key
     * is new and the object can grow, @p child is attached.
     *
     * @param self    Pointer to this provider instance.
     * @param obj     Target object node. Must be of type
     *                @c PUBNUB_JSON_OBJECT.
     * @param key     Key bytes (need not be NUL-terminated). The
     *                provider COPIES the key.
     * @param key_len Length of @p key in bytes (excludes any NUL).
     * @param child   Caller-owned node to attach. On success ownership
     *                transfers to @p obj; on failure the caller still
     *                owns @p child.
     * @return PUBNUB_OK on success, or argument / capacity-class
     *         error on failure.
     */
    pubnub_res_t (*object_set)(struct pubnub_serialization_provider* self,
                               pubnub_json_value_t*                  obj,
                               const char*                           key,
                               size_t                                key_len,
                               pubnub_json_value_t*                  child);

    /**
     * @brief Append a value to the end of a JSON array.
     *
     * @param self Pointer to this provider instance.
     * @param arr  Target array node. Must be of type
     *             @c PUBNUB_JSON_ARRAY.
     * @param item Caller-owned node to attach. On success ownership
     *             transfers to @p arr; on failure the caller still
     *             owns @p item.
     * @return PUBNUB_OK on success, or argument / capacity-class
     *         error on failure.
     */
    pubnub_res_t (*array_append)(struct pubnub_serialization_provider* self,
                                 pubnub_json_value_t*                  arr,
                                 pubnub_json_value_t*                  item);

    /**
     * @brief Remove a key from a JSON object.
     *
     * Frees the value subtree associated with @p key.
     *
     * @param self    Pointer to this provider instance.
     * @param obj     Target object node.
     * @param key     Key bytes (need not be NUL-terminated).
     * @param key_len Length of @p key in bytes.
     * @return PUBNUB_OK if the key existed and was removed,
     *         PUBNUB_ERR_INVALID_ARGUMENT if @p key is not present
     *         or @p obj is not an object.
     */
    pubnub_res_t (*object_remove)(struct pubnub_serialization_provider* self,
                                  pubnub_json_value_t*                  obj,
                                  const char*                           key,
                                  size_t key_len);

    /**
     * @brief Remove an element at a given index from a JSON array.
     *
     * Later elements shift down to fill the gap; the index of any
     * borrowed pointer obtained via @c array_get from a higher
     * position is invalidated.
     *
     * @param self  Pointer to this provider instance.
     * @param arr   Target array node.
     * @param index Zero-based index of the element to remove.
     * @return PUBNUB_OK on success, or
     *         PUBNUB_ERR_INVALID_ARGUMENT if @p index is out of range
     *         or @p arr is not an array.
     */
    pubnub_res_t (*array_remove)(struct pubnub_serialization_provider* self,
                                 pubnub_json_value_t*                  arr,
                                 size_t                                index);

    /**
     * @brief Hint that @p obj should pre-check capacity for @p n keys.
     *
     * May be @c NULL -- caller must check before invoking. Backends that
     * cannot pre-reserve (e.g. bump allocators or per-node
     * implementations) set this to @c NULL; the caller treats @c NULL as
     * "no early-fail check available" and proceeds with @c object_set,
     * which will still fail with @c PUBNUB_ERR_OUT_OF_MEMORY if
     * capacity is actually exhausted.
     *
     * @param self Pointer to this provider instance.
     * @param obj  Target object node.
     * @param n    Number of additional keys the caller intends to
     *             insert.
     * @return PUBNUB_OK if the reservation succeeds (or is a no-op),
     *         PUBNUB_ERR_OUT_OF_MEMORY if the requested capacity
     *         cannot be guaranteed.
     */
    pubnub_res_t (*object_reserve)(struct pubnub_serialization_provider* self,
                                   pubnub_json_value_t*                  obj,
                                   size_t                                n);

    /**
     * @brief Hint that @p arr should pre-check capacity for @p n items.
     *
     * Same @c NULL semantics as @c object_reserve.
     *
     * @param self Pointer to this provider instance.
     * @param arr  Target array node.
     * @param n    Number of additional elements the caller intends to
     *             append.
     * @return PUBNUB_OK if the reservation succeeds (or is a no-op),
     *         PUBNUB_ERR_OUT_OF_MEMORY if the requested capacity
     *         cannot be guaranteed.
     */
    pubnub_res_t (*array_reserve)(struct pubnub_serialization_provider* self,
                                  pubnub_json_value_t*                  arr,
                                  size_t                                n);

    /**
     * @brief Discriminate the JSON type of a node.
     *
     * @param value Node to inspect; @c NULL is treated as
     *              @c PUBNUB_JSON_NULL by convention but providers
     *              MAY return any value -- callers SHOULD null-check
     *              before invoking.
     * @return The @c pubnub_json_type_t tag for @p value.
     */
    pubnub_json_type_t (*value_type)(const pubnub_json_value_t* value);

    /**
     * @brief Borrow the bytes of a JSON string node.
     *
     * The returned pointer aliases provider-owned storage and remains
     * valid until @c value_destroy on the tree root. NUL-termination
     * is NOT guaranteed; callers MUST use the returned length.
     *
     * @param value   Node to inspect.
     * @param out_len On success, receives the byte length of the
     *                returned span. Required.
     * @return Pointer to the string bytes, or @c NULL if @p value is
     *         not a string or @p out_len is @c NULL.
     */
    const char* (*value_as_string)(const pubnub_json_value_t* value,
                                   size_t*                    out_len);

    /**
     * @brief Read an integer value from a JSON integer node.
     *
     * @param value Node to inspect.
     * @param out   On success, receives the integer value. Untouched
     *              on failure.
     * @return PUBNUB_OK if @p value is an integer representable as
     *         @c int, PUBNUB_ERR_INVALID_ARGUMENT if @p value is not
     *         an integer or @p out is @c NULL.
     */
    pubnub_res_t (*value_as_int)(const pubnub_json_value_t* value, int* out);

    /**
     * @brief Read a floating-point value from a JSON double node.
     *
     * May be @c NULL -- caller must check before invoking. Set to @c NULL
     * when @c PUBNUB_CFG_JSON_DOUBLE is 0.
     *
     * @param value Node to inspect.
     * @param out   On success, receives the double value. Untouched
     *              on failure.
     * @return PUBNUB_OK if @p value is a double,
     *         PUBNUB_ERR_INVALID_ARGUMENT otherwise.
     */
    pubnub_res_t (*value_as_double)(const pubnub_json_value_t* value, double* out);

    /**
     * @brief Read a boolean value from a JSON bool node.
     *
     * @param value      Node to inspect.
     * @param out_truthy On success, receives 0 for JSON @c false and
     *                   1 for JSON @c true. Untouched on failure.
     * @return PUBNUB_OK if @p value is a boolean,
     *         PUBNUB_ERR_INVALID_ARGUMENT otherwise.
     */
    pubnub_res_t (*value_as_bool)(const pubnub_json_value_t* value, int* out_truthy);

    /**
     * @brief Borrow the value associated with @p key in a JSON object.
     *
     * @param obj     Object node.
     * @param key     Key bytes (need not be NUL-terminated).
     * @param key_len Length of @p key in bytes.
     * @return Borrowed pointer to the value subtree (valid until
     *         @c value_destroy on the root), or @c NULL if @p key is
     *         absent or @p obj is not an object.
     */
    pubnub_json_value_t* (*object_get)(const pubnub_json_value_t* obj,
                                       const char*                key,
                                       size_t                     key_len);

    /**
     * @brief Count the keys in a JSON object.
     *
     * @param obj Object node.
     * @return Number of keys, or 0 if @p obj is not an object.
     */
    size_t (*object_size)(const pubnub_json_value_t* obj);

    /**
     * @brief Borrow the element at @p index in a JSON array.
     *
     * @param arr   Array node.
     * @param index Zero-based index.
     * @return Borrowed pointer to the element subtree, or @c NULL if
     *         @p index is out of range or @p arr is not an array.
     */
    pubnub_json_value_t* (*array_get)(const pubnub_json_value_t* arr, size_t index);

    /**
     * @brief Count the elements in a JSON array.
     *
     * @param arr Array node.
     * @return Number of elements, or 0 if @p arr is not an array.
     */
    size_t (*array_size)(const pubnub_json_value_t* arr);

    /**
     * @brief Initialize an iterator over the keys of a JSON object.
     *
     * The iterator captures the current state of @p obj. ANY
     * subsequent mutation of @p obj invalidates the iterator;
     * iteration after mutation is undefined.
     *
     * @param obj  Object node.
     * @param iter Iterator storage to initialize. Required.
     * @return 1 if @p obj is a non-empty object and the iterator is
     *         positioned before the first entry,
     *         0 if @p obj is empty / not an object / @p iter is @c NULL
     *         (in which case @c object_iter_next will also return 0).
     */
    int (*object_iter_init)(const pubnub_json_value_t* obj,
                            pubnub_json_iter_t*        iter);

    /**
     * @brief Advance an iterator to the next key/value pair.
     *
     * @param iter        Iterator initialized via @c object_iter_init.
     * @param out_key     On a valid step, receives a borrowed pointer
     *                    to the key bytes (NOT NUL-terminated). May be
     *                    @c NULL if the caller does not need the key.
     * @param out_key_len On a valid step, receives the length of
     *                    @p *out_key. May be @c NULL.
     * @param out_value   On a valid step, receives a borrowed pointer
     *                    to the value subtree. May be @c NULL.
     * @return 1 on a valid step (out_* populated when non-NULL),
     *         0 on exhaustion (out_* untouched).
     */
    int (*object_iter_next)(pubnub_json_iter_t*   iter,
                            const char**          out_key,
                            size_t*               out_key_len,
                            pubnub_json_value_t** out_value);

    /**
     * @brief Initialize an iterator over the elements of a JSON array.
     *
     * The iterator captures the current state of @p arr. ANY
     * subsequent mutation of @p arr invalidates the iterator;
     * iteration after mutation is undefined. Prefer this over repeated
     * @c array_get calls when walking every element: backends may
     * implement @c array_get as an O(n) traversal, making an
     * index-driven loop O(n^2), whereas the iterator advances in O(1)
     * per step.
     *
     * Optional: a backend that does not implement array iteration
     * leaves this @c NULL. Callers MUST NULL-check before use.
     *
     * @param arr  Array node.
     * @param iter Iterator storage to initialize. Required.
     * @return 1 if @p arr is a non-empty array and the iterator is
     *         positioned before the first element,
     *         0 if @p arr is empty / not an array / @p iter is @c NULL
     *         (in which case @c array_iter_next will also return 0).
     */
    int (*array_iter_init)(const pubnub_json_value_t* arr,
                           pubnub_json_array_iter_t*  iter);

    /**
     * @brief Advance an iterator to the next array element.
     *
     * @param iter      Iterator initialized via @c array_iter_init.
     * @param out_value On a valid step, receives a borrowed pointer to
     *                  the element subtree. May be @c NULL if the
     *                  caller does not need the value.
     * @return 1 on a valid step (out_value populated when non-NULL),
     *         0 on exhaustion (out_value untouched).
     */
    int (*array_iter_next)(pubnub_json_array_iter_t* iter,
                           pubnub_json_value_t**     out_value);
} pubnub_serialization_provider_t;

#ifdef __cplusplus
// clang-format off
}
// clang-format on
#endif

#endif /* PUBNUB_PROVIDER_SERIALIZATION_H */
