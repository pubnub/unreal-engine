/* Copyright (c) PubNub Inc. */
/* See LICENSE in the root directory of this source tree. */

/**
 * @file providers/allocator.h
 * @brief Memory allocator provider interface.
 *
 * Two tiers: general (alloc/realloc/free) and purpose-tagged buffers
 * (buf_acquire/buf_release/buf_grow). Arena allocators may @c NULL out
 * realloc/buf_grow. All callbacks from non-ISR context only.
 *
 * **Mandatory methods:** alloc, free, buf_acquire, buf_release.
 *
 * **Optional methods:** realloc, buf_grow, init, deinit.
 * Arena allocators set realloc and buf_grow to @c NULL (fixed partitions
 * cannot relocate). The SDK handles @c NULL realloc via alloc+copy+free
 * and @c NULL buf_grow as PUBNUB_ERR_BUFFER_TOO_SMALL.
 */

#ifndef PUBNUB_PROVIDER_ALLOCATOR_H
#define PUBNUB_PROVIDER_ALLOCATOR_H

#include "pubnub/config.h"

#include <stddef.h>
#include <stdint.h>

struct pubnub_platform_provider;

#ifdef __cplusplus
// clang-format off
extern "C" {
// clang-format on
#endif

/**
 * @brief Purpose tag for buffer acquisition.
 *
 * Arena allocators use this to select a pre-partitioned region.
 * Heap allocators may ignore the tag entirely.
 */
typedef enum pubnub_buf_purpose {
    /** Incoming HTTP response body. */
    PUBNUB_BUF_RX = 0,
    /** POST/PATCH request body (JSON payload). */
    PUBNUB_BUF_OBJ,
    /** Temporary working memory. */
    PUBNUB_BUF_SCRATCH
} pubnub_buf_purpose_t;

/**
 * @brief Tagged buffer descriptor returned by buf_acquire.
 *
 * The `data` pointer is owned by the allocator; the caller may read
 * and write up to `cap` bytes, and tracks actual usage in `len`.
 */
typedef struct pubnub_buffer {
    /** Pointer to buffer memory (owned by allocator). */
    uint8_t* data;
    /** Bytes currently used. */
    size_t len;
    /** Total capacity in bytes. */
    size_t cap;
    /** Purpose tag for this buffer. */
    pubnub_buf_purpose_t purpose;
} pubnub_buffer_t;

/**
 * @brief Allocator provider function table.
 *
 * Shared provider (not owned by any context; multiple contexts may
 * share one instance). Store implementation state in an extended
 * struct with this vtable as the first member.
 */
typedef struct pubnub_allocator_provider {
    /**
     * @brief Allocate @p size bytes with @p align alignment.
     *
     * @param self  Pointer to this provider instance.
     * @param size  Requested allocation size in bytes.
     * @param align Required alignment (must be power of 2, 0 or 1 = default).
     * @return Pointer to allocated memory, or @c NULL on failure.
     */
    void* (*alloc)(struct pubnub_allocator_provider* self, size_t size, size_t align);

    /**
     * @brief Reallocate a previously allocated block.
     *
     * Optional: @c NULL for arena allocators (SDK falls back to
     * alloc+copy+free).
     *
     * @param self     Pointer to this provider instance.
     * @param ptr      Pointer from a previous alloc/realloc.
     * @param old_size Size of existing allocation (for arenas).
     * @param new_size Requested new size.
     * @param align    Required alignment (power of 2; 0/1 = default).
     * @return Pointer to reallocated memory, or @c NULL on failure.
     */
    void* (*realloc)(struct pubnub_allocator_provider* self,
                     void*                             ptr,
                     size_t                            old_size,
                     size_t                            new_size,
                     size_t                            align);

    /**
     * @brief Free a previously allocated block.
     *
     * @p ptr may be @c NULL (no-op). Arena allocators may treat this
     * as a no-op if they only support bulk-free on destroy.
     *
     * @param self Pointer to this provider instance.
     * @param ptr  Pointer to free.
     */
    void (*free)(struct pubnub_allocator_provider* self, void* ptr);

    /**
     * @brief Acquire a purpose-tagged buffer.
     *
     * Arena allocators use @p purpose to select a pre-partitioned
     * region. Heap allocators may allocate dynamically and ignore
     * the tag.
     *
     * The returned buffer has `len` set to 0 and `cap` set to the
     * available capacity. If the buffer cannot be acquired (e.g.
     * arena region is already in use), `data` is @c NULL and `cap` is 0.
     *
     * @param self    Pointer to this provider instance.
     * @param purpose Buffer usage tag.
     * @return Buffer descriptor.
     */
    pubnub_buffer_t (*buf_acquire)(struct pubnub_allocator_provider* self,
                                   pubnub_buf_purpose_t              purpose);

    /**
     * @brief Release a previously acquired buffer.
     *
     * After this call the buffer's data pointer is invalid.
     *
     * @param self Pointer to this provider instance.
     * @param buf  Buffer to release.
     */
    void (*buf_release)(struct pubnub_allocator_provider* self,
                        pubnub_buffer_t*                  buf);

    /**
     * @brief Attempt to grow a buffer to @p new_cap bytes.
     *
     * Optional: may be @c NULL. When @c NULL, the SDK core treats a grow
     * request as a failure (PUBNUB_ERR_BUFFER_TOO_SMALL). Arena
     * allocators with fixed-size partitions typically set this to @c NULL.
     *
     * @param self    Pointer to this provider instance.
     * @param buf     Buffer to grow (cap is updated on success).
     * @param new_cap Requested new capacity.
     * @return 0 on success, non-zero on failure.
     */
    int (*buf_grow)(struct pubnub_allocator_provider* self,
                    pubnub_buffer_t*                  buf,
                    size_t                            new_cap);

    /**
     * @brief Initialize internal allocator state.
     *
     * Optional: @c NULL = no init needed. Called once during context init.
     *
     * @note The allocator takes only @c platform (not the full
     *       @c pubnub_provider_deps_t bundle) because the allocator must
     *       be initialized before the deps bundle can be constructed —
     *       the bundle itself contains the allocator pointer.
     *
     * @param self     Pointer to this allocator provider instance.
     * @param platform Platform provider (borrowed, non-NULL).
     * @return 0 on success, non-zero on failure.
     */
    int (*init)(struct pubnub_allocator_provider* self,
                struct pubnub_platform_provider*  platform);

    /**
     * @brief Tear down internal allocator state.
     *
     * Optional: @c NULL = no cleanup needed. Called during context deinit.
     *
     * @note Takes @c platform explicitly (rather than a single @c self)
     *       for the same bootstrapping reason as @c init — symmetry with
     *       the init signature keeps the teardown path predictable.
     *
     * @param self     Pointer to this allocator provider instance.
     * @param platform Platform provider (borrowed, non-NULL).
     */
    void (*deinit)(struct pubnub_allocator_provider* self,
                   struct pubnub_platform_provider*  platform);
} pubnub_allocator_provider_t;

#ifdef __cplusplus
// clang-format off
}
// clang-format on
#endif

#endif /* PUBNUB_PROVIDER_ALLOCATOR_H */
