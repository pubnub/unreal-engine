/* Copyright (c) PubNub Inc. */
/* See LICENSE in the root directory of this source tree. */

#ifndef PUBNUB_ALLOCATOR_ARENA_H
#define PUBNUB_ALLOCATOR_ARENA_H

#include "pubnub/config.h"
#include "pubnub/providers/allocator.h"
#include "pubnub/pubnub_compat.h"

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
// clang-format off
extern "C" {
// clang-format on
#endif

/**
 * @brief Minimum pool size recommended by the SDK build system for the
 *        current feature and concurrency configuration.
 *
 * Alias for @c PUBNUB_CFG_ARENA_POOL_SIZE. Use to size your pool buffer
 * on platforms without Kconfig:
 * @code
 * static uint8_t pool[PUBNUB_ARENA_RECOMMENDED_POOL_SIZE];
 * @endcode
 */
#define PUBNUB_ARENA_RECOMMENDED_POOL_SIZE PUBNUB_CFG_ARENA_POOL_SIZE

/**
 * @brief Opaque free-list block header; layout defined in the allocator
 *        implementation. Callers must not dereference this pointer.
 */
struct pubnub_arena_free_block;

/**
 * @brief Number of RX (response body) buffer slots in Zone A.
 *
 * One slot per in-flight request. Pending requests do not hold an RX
 * buffer because they have no active HTTP transfer.
 */
#define PUBNUB_ARENA_RX_SLOTS PUBNUB_CFG_MAX_IN_FLIGHT_REQUESTS

/**
 * @brief Number of OBJ (request body) buffer slots in Zone A.
 *
 * When request compression is disabled: one slot per in-flight request
 * plus one per pending request. A POST/PATCH body buffer is acquired
 * before a request enters the pending queue and held throughout queueing.
 *
 * When request compression is enabled: two slots per in-flight request
 * (original body + compressed copy) plus one per pending request.
 */
#if PUBNUB_ENABLE_REQUEST_COMPRESSION
#define PUBNUB_ARENA_OBJ_SLOTS                     \
    ((size_t)PUBNUB_CFG_MAX_IN_FLIGHT_REQUESTS * 2 \
     + (size_t)PUBNUB_CFG_MAX_PENDING_REQUESTS)
#else
#define PUBNUB_ARENA_OBJ_SLOTS                 \
    ((size_t)PUBNUB_CFG_MAX_IN_FLIGHT_REQUESTS \
     + (size_t)PUBNUB_CFG_MAX_PENDING_REQUESTS)
#endif

/**
 * @brief Number of SCRATCH (temporary workspace) buffer slots in Zone A.
 *
 * One slot per in-flight request. Scratch memory is acquired during
 * serialization and middleware processing and released immediately after.
 */
#define PUBNUB_ARENA_SCRATCH_SLOTS PUBNUB_CFG_MAX_IN_FLIGHT_REQUESTS

/**
 * @brief Two-zone arena allocator instance.
 *
 * **Zone A** — fixed-size, purpose-tagged slot pools. Serves
 * @c buf_acquire / @c buf_release in O(slots) time with no heap activity.
 *
 * **Zone B** — free-list bump allocator. Serves @c alloc / @c free for
 * both per-request objects (freed in @c feature_state_cleanup) and
 * context-lifetime dynamic objects (freed on rotation). Freed blocks are
 * prepended to an intrusive singly-linked list; the next @c alloc scans
 * for a first-fit before advancing the bump cursor.
 *
 * Declare as a file-scope @c static variable or embed in a larger struct.
 * Pass @c &instance.base to @c pubnub_config_t.allocator.
 *
 * @note @c realloc and @c buf_grow are @c NULL in the vtable. The SDK
 *       handles @c NULL realloc via alloc+copy+free, and treats @c NULL
 *       @c buf_grow as @c PUBNUB_ERR_BUFFER_TOO_SMALL (fixed partitions
 *       cannot grow).
 *
 * @warning **Single-tenant only.** Each arena instance serves exactly one
 *          @c pubnub_context_t. Sharing an arena across multiple contexts
 *          causes undefined behavior — @c deinit on one context rewinds
 *          the bump cursor, invalidating pointers held by the other.
 *          Create a separate arena (with its own pool) per context.
 */
typedef struct pubnub_arena_allocator {
    /**
     * @brief Provider vtable — MUST be the first member.
     *
     * Pass <tt>&instance.base</tt> wherever a
     * @c pubnub_allocator_provider_t* is expected.
     */
    pubnub_allocator_provider_t base;

    /** @brief Backing memory pool supplied by the caller. */
    uint8_t* pool;

    /** @brief Zone A: in-use flag per RX slot (0 = free, 1 = acquired). */
    uint8_t rx_in_use[PUBNUB_ARENA_RX_SLOTS];

    /** @brief Zone A: in-use flag per OBJ slot. */
    uint8_t obj_in_use[PUBNUB_ARENA_OBJ_SLOTS];

    /** @brief Zone A: in-use flag per SCRATCH slot. */
    uint8_t scratch_in_use[PUBNUB_ARENA_SCRATCH_SLOTS];

    /** @brief Zone B: first byte of the bump region. */
    uint8_t* zone_b_base;

    /** @brief Zone B: bump cursor; next allocation starts here. */
    uint8_t* zone_b_cursor;

    /**
     * @brief One-past-the-end of the pool (@c pool + @c pool_size).
     *
     * Also serves as the upper bound for Zone B. The effective Zone B
     * budget is @c zone_b_end - @c zone_b_base bytes.
     */
    uint8_t* zone_b_end;

    /** @brief Zone B: head of the intrusive free-list (@c NULL = empty). */
    struct pubnub_arena_free_block* free_list;
} pubnub_arena_allocator_t;

/**
 * @brief Initialise an arena allocator backed by a caller-owned pool.
 *
 * Wires the vtable, computes the Zone A / Zone B layout from
 * compile-time constants, zeroes @p pool, and clears all slot flags and
 * free-list state. Safe to call more than once on the same @p arena —
 * each call performs a full reset and any live allocations are
 * discarded.
 *
 * @note **Ownership model.** The SDK supports two arena pool ownership
 *       modes controlled by @c PUBNUB_CFG_ARENA_POOL_OWNER_SDK:
 *       - **SDK-owned (1, hosted profiles):** The SDK declares a static
 *         pool in BSS and @c pn_allocator_default() returns it. You do
 *         not need to call this function unless you need multiple
 *         contexts.
 *       - **User-owned (0, embedded profile):** No static pool exists in
 *         the SDK; @c pn_allocator_default() returns @c NULL. Call this
 *         function with your own buffer and assign the result to
 *         @c pubnub_config_t.allocator before creating a context.
 *
 * Typical embedded usage:
 * @code
 * static uint8_t pool[PUBNUB_ARENA_RECOMMENDED_POOL_SIZE];
 * static pubnub_arena_allocator_t arena;
 *
 * pubnub_arena_allocator_init(&arena, pool, sizeof(pool));
 * cfg.allocator = &arena.base;
 * @endcode
 *
 * @param arena     Arena instance to initialise. Must remain valid for
 *                  the lifetime of every context that uses this allocator.
 * @param pool      Backing memory block (caller-owned). The SDK never
 *                  frees this buffer; it must outlive @p arena. Declare
 *                  as @c static or place via linker script on bare-metal
 *                  targets.
 * @param pool_size Size of @p pool in bytes. Must exceed the Zone A
 *                  footprint
 *                  (<tt>PUBNUB_ARENA_RX_SLOTS * PUBNUB_CFG_RESPONSE_BUFFER_SIZE
 *                   + PUBNUB_ARENA_OBJ_SLOTS * PUBNUB_CFG_OBJECT_BUFFER_SIZE
 *                   + PUBNUB_ARENA_SCRATCH_SLOTS *
 *                   PUBNUB_CFG_SCRATCH_BUFFER_SIZE</tt>). Use at least
 *                  @c PUBNUB_ARENA_RECOMMENDED_POOL_SIZE bytes.
 * @return Pointer to the embedded vtable, ready for assignment to
 *         @c pubnub_config_t.allocator. Returns @c NULL when any
 *         argument is invalid or @p pool_size is too small.
 *
 * @warning Do not pass the same @p arena to multiple contexts. Each
 *          context must have its own arena instance backed by its own
 *          pool.
 */
PUBNUB_API pubnub_allocator_provider_t*
pubnub_arena_allocator_init(pubnub_arena_allocator_t* arena,
                            uint8_t*                  pool,
                            size_t                    pool_size);

#ifdef __cplusplus
// clang-format off
}
// clang-format on
#endif

#endif /* PUBNUB_ALLOCATOR_ARENA_H */
