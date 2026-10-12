/* Copyright (c) PubNub Inc. */
/* See LICENSE in the root directory of this source tree. */

/**
 * @file providers/platform.h
 * @brief Platform abstraction provider (time, sleep, random, lock, thread).
 *
 * Exposes primitives only -- no business logic or retry policy.
 * All locks are non-recursive. All callbacks are from non-ISR context.
 *
 * **Mandatory methods:** monotonic_ms, wall_clock_ms, sleep_ms, random_bytes.
 *
 * **Optional method groups (all-or-nothing):**
 * - Lock: lock_size, lock_init, lock_destroy, lock_acquire,
 *   lock_release. If any is non-NULL, all five must be non-NULL.
 * - Thread: thread_create, thread_join. If thread_create is non-NULL,
 *   thread_join must also be non-NULL.
 *
 * **Standalone optional:** secure_zero, file_load, thread_is_current
 * (may be @c NULL independently).
 */

#ifndef PUBNUB_PROVIDER_PLATFORM_H
#define PUBNUB_PROVIDER_PLATFORM_H

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

struct pubnub_allocator_provider;

/**
 * @brief Platform provider function table.
 *
 * All callbacks are invoked from normal (non-ISR) context only.
 *
 * This is a **shared** provider: the SDK never calls lifecycle
 * callbacks on it. Create, configure, and release the provider
 * externally; it is not owned by any context. Multiple contexts may share
 * the same platform instance.
 *
 * Implementation-specific state should be stored in an extended
 * struct with this vtable as the first member (cast `self` to
 * recover your type).
 */
typedef struct pubnub_platform_provider {
    /**
     * @brief Return monotonic time in milliseconds.
     *
     * Boot-relative clock used for timeouts and deadlines.
     * MUST NOT wrap or decrease within a session.
     *
     * @param self Pointer to this provider instance.
     * @return Milliseconds since an arbitrary fixed point (typically
     *         boot). The value has no relationship to wall-clock time.
     */
    pubnub_milliseconds_t (*monotonic_ms)(struct pubnub_platform_provider* self);

    /**
     * @brief Return the current wall-clock time in milliseconds.
     *
     * Returns the number of milliseconds since the Unix epoch
     * (1970-01-01T00:00:00 UTC). Used by the PAM signature middleware
     * to generate the @c timestamp= HMAC signing parameter.
     *
     * Implementations:
     * - Return the correct wall-clock time when a real-time clock or
     *   SNTP synchronization is available.
     * - Return @c 0 when wall-clock time is not available (bare-metal
     *   without RTC/NTP, not yet synchronized). The PAM middleware
     *   treats @c 0 as an error and returns @c PUBNUB_ERR_NO_WALL_CLOCK.
     *
     * @note This is separate from @c monotonic_ms. @c monotonic_ms is
     *       boot-relative and used for timeouts and deadlines.
     *       @c wall_clock_ms is epoch-anchored and used only for PAM.
     *
     * @param self Platform provider instance.
     * @return Milliseconds since Unix epoch, or @c 0 if unavailable.
     */
    pubnub_milliseconds_t (*wall_clock_ms)(struct pubnub_platform_provider* self);

    /**
     * @brief Sleep/yield for at least @p ms milliseconds.
     *
     * On bare-metal targets this may be a busy-wait or task yield.
     *
     * @param self Pointer to this provider instance.
     * @param ms   Minimum sleep duration in milliseconds.
     */
    void (*sleep_ms)(struct pubnub_platform_provider* self, uint32_t ms);

    /**
     * @brief Generate random bytes.
     *
     * Used for jitter, nonces, etc. Must fill @p buf with @p len
     * cryptographically-suitable random bytes where available;
     * best-effort PRNG otherwise.
     *
     * @param self Pointer to this provider instance.
     * @param buf  Output buffer.
     * @param len  Number of bytes to generate.
     * @return 0 on success, non-zero on failure.
     */
    int (*random_bytes)(struct pubnub_platform_provider* self,
                        uint8_t*                         buf,
                        size_t                           len);

    /**
     * @brief Secure-zeroize a buffer.
     *
     * Overwrites @p buf with zeros in a way that is not optimized out
     * by the compiler. Used for clearing sensitive data (keys, tokens).
     *
     * Optional: may be NULL. When NULL, the SDK core falls back to a
     * volatile memset loop.
     *
     * @param self Pointer to this provider instance.
     * @param buf  Buffer to zeroize.
     * @param len  Buffer length in bytes.
     */
    void (*secure_zero)(struct pubnub_platform_provider* self, void* buf, size_t len);

    /**
     * @brief Return the size in bytes required for a platform lock.
     *
     * The SDK allocates this many bytes via the allocator and passes
     * the resulting memory to lock_init(). The returned size must
     * be stable for the lifetime of the provider instance.
     *
     * Optional: may be @c NULL. When @c NULL (or when
     * PUBNUB_CFG_THREAD_SAFETY is 0), no lock is created and
     * same-context thread safety is the caller's responsibility.
     *
     * @param self Pointer to this provider instance.
     * @return Size in bytes required by the platform lock implementation.
     */
    size_t (*lock_size)(struct pubnub_platform_provider* self);

    /**
     * @brief Initialize a lock in caller-provided memory.
     *
     * @p lock points to at least lock_size() bytes of allocated
     * (but uninitialized) memory. The provider initializes its
     * platform lock in-place -- no internal allocation.
     *
     * All locks are non-recursive. Attempting to acquire a lock
     * already held by the same thread results in undefined behavior.
     *
     * @param self Pointer to this provider instance.
     * @param lock Caller-allocated memory of at least lock_size()
     *             bytes.
     * @return 0 on success, non-zero on failure.
     */
    int (*lock_init)(struct pubnub_platform_provider* self, pubnub_lock_t* lock);

    /**
     * @brief Destroy a lock previously initialized by lock_init().
     *
     * Releases any OS resources but does NOT free @p lock -- the
     * caller (SDK core) manages the memory lifetime via the
     * allocator.
     *
     * @param self Pointer to this provider instance.
     * @param lock Memory containing an initialized lock.
     */
    void (*lock_destroy)(struct pubnub_platform_provider* self, pubnub_lock_t* lock);

    /**
     * @brief Acquire the lock.
     *
     * Blocks the calling thread until the lock is acquired. Must
     * not be called from ISR context. The lock is non-recursive;
     * do not call while already holding the same lock.
     *
     * @param self Pointer to this provider instance.
     * @param lock Memory containing an initialized lock.
     */
    void (*lock_acquire)(struct pubnub_platform_provider* self, pubnub_lock_t* lock);

    /**
     * @brief Release the lock.
     *
     * @param self Pointer to this provider instance.
     * @param lock Memory containing an initialized lock.
     */
    void (*lock_release)(struct pubnub_platform_provider* self, pubnub_lock_t* lock);

    /**
     * @brief Create a new thread executing @p fn(arg).
     *
     * Optional: may be @c NULL on cooperative-only targets. When @c NULL,
     * pubnub_async degrades to callback-registration-only (user must
     * drive pubnub_process manually).
     *
     * @note The allocator's free() may be invoked from the spawned
     *       thread to release internal bookkeeping. Ensure the
     *       allocator is safe for cross-thread deallocation when
     *       thread support is enabled.
     *
     * @param self      Pointer to this provider instance.
     * @param allocator Allocator for dynamic memory (borrowed,
     *                  non-NULL). Used to allocate the thread handle
     *                  and any internal trampoline structures.
     * @param fn        Thread entry point (non-NULL).
     * @param arg       Opaque argument forwarded to @p fn.
     * @return Opaque thread handle on success, or @c NULL on failure.
     *         The handle must be passed to thread_join() for cleanup.
     */
    void* (*thread_create)(struct pubnub_platform_provider*  self,
                           struct pubnub_allocator_provider* allocator,
                           void (*fn)(void*),
                           void* arg);

    /**
     * @brief Join (wait for) a previously created thread and release
     *        its resources.
     *
     * Blocks until the thread terminates. Must be called exactly once
     * per successful thread_create(). May be @c NULL only when
     * thread_create is also @c NULL.
     *
     * @param self          Pointer to this provider instance.
     * @param allocator     Allocator used in thread_create (borrowed,
     *                      non-NULL). Used to free the thread handle.
     * @param thread_handle Handle returned by thread_create().
     */
    void (*thread_join)(struct pubnub_platform_provider*  self,
                        struct pubnub_allocator_provider* allocator,
                        void*                             thread_handle);

    /**
     * @brief Report whether @p thread_handle refers to the calling thread.
     *
     * Lets the SDK avoid a self-join during teardown: joining the calling
     * thread returns @c EDEADLK on POSIX (silently doing nothing) and
     * deadlocks on Windows/Zephyr. Called before thread_join() when a
     * context is torn down.
     *
     * Standalone optional: may be @c NULL even when thread_create and
     * thread_join are provided. When @c NULL the SDK skips the self-join
     * check and joins unconditionally (the pre-existing behavior).
     *
     * @param self          Pointer to this provider instance.
     * @param thread_handle Handle returned by thread_create().
     * @retval non-zero @p thread_handle is the calling thread.
     * @retval 0        @p thread_handle is a different thread, or is
     *                  @c NULL.
     */
    int (*thread_is_current)(struct pubnub_platform_provider* self,
                             void*                            thread_handle);

    /**
     * @brief Load entire file content into an allocator-owned buffer.
     *
     * Opens @p path, reads all bytes into a buffer allocated via
     * @p allocator, and closes the file. The caller is responsible for
     * freeing @c *out_data through the same allocator.
     *
     * Standalone optional: may be NULL. When NULL, @c file_path in
     * @c pubnub_send_file_opts_t returns @c PUBNUB_ERR_NOT_SUPPORTED.
     *
     * @param self      Pointer to this provider instance.
     * @param path      NUL-terminated filesystem path (non-NULL).
     * @param allocator Allocator used to allocate @c *out_data (non-NULL).
     * @param out_data  Receives a pointer to the allocated file bytes.
     *                  Set to @c NULL on failure.
     * @param out_len   Receives the number of bytes loaded.
     *                  Set to 0 on failure.
     * @return @c PUBNUB_OK on success.
     * @retval PUBNUB_ERR_INVALID_ARGUMENT  @p path not found or not
     *         readable.
     * @retval PUBNUB_ERR_OUT_OF_MEMORY     Allocation failed.
     * @retval PUBNUB_ERR_INTERNAL          I/O error during read.
     */
    pubnub_res_t (*file_load)(struct pubnub_platform_provider*  self,
                              const char*                       path,
                              struct pubnub_allocator_provider* allocator,
                              uint8_t**                         out_data,
                              size_t*                           out_len);

} pubnub_platform_provider_t;

#ifdef __cplusplus
// clang-format off
}
// clang-format on
#endif

#endif /* PUBNUB_PROVIDER_PLATFORM_H */
