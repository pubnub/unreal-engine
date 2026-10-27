/* Copyright (c) PubNub Inc. */
/* See LICENSE in the root directory of this source tree. */

#ifndef PUBNUB_ALLOCATOR_ARENA_H
#define PUBNUB_ALLOCATOR_ARENA_H

#include "pubnub/config.h"
#include "pubnub/providers/allocator.h"
#include "pubnub/pubnub_compat.h"

#ifndef PUBNUB_ARENA_MAX_ZONE_B_CELLS
#error "allocator_arena.h requires arena allocator configuration. " \
       "Ensure PUBNUB_PROVIDER_ALLOCATOR=arena in your build."
#endif

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
 * **Zone B** — fixed-cell pool allocator. Serves @c alloc / @c free for
 * both per-request objects (freed in @c feature_state_cleanup) and
 * context-lifetime dynamic objects (freed on rotation). Zone B is
 * divided into 256-byte cells; an allocation of N bytes claims
 * ceil(N/256) consecutive cells. Freed cells are individually
 * reclaimed and can be reassembled into any consecutive run.
 *
 * Declare as a file-scope @c static variable or embed in a larger struct.
 * Pass @c &instance.base to @c pubnub_config_t.allocator.
 *
 * @note @c buf_grow is @c NULL in the vtable (fixed-size partitions
 *       cannot grow; the SDK treats this as
 *       @c PUBNUB_ERR_BUFFER_TOO_SMALL). @c realloc IS implemented:
 *       it attempts in-place expansion by claiming adjacent free cells
 *       and falls back to alloc-copy-free when that is not possible.
 *       Returns @c NULL without touching the old block on failure.
 *
 * @warning **Single-tenant only.** Each arena instance serves exactly one
 *          @c pubnub_context_t. Sharing an arena across multiple contexts
 *          causes undefined behavior — @c deinit on one context zeroes
 *          all cell tracking, invalidating pointers held by the other.
 *          Create a separate arena (with its own pool) per context.
 *
 * @warning **No internal locking.** The arena itself performs no
 *          synchronization. Within the standard SDK lifecycle all arena
 *          access is serialised by the SDK's per-context mutex, so no
 *          additional locking is needed. External locking is required only
 *          when you call arena functions directly outside the SDK — for
 *          example, concurrent crypto operations sharing the same arena
 *          instance without going through a @c pubnub_context_t.
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

    /** @brief Zone B: first byte of the cell pool region. */
    uint8_t* zone_b_base;

    /**
     * @brief One-past-the-end of the pool (@c pool + @c pool_size).
     *
     * Also serves as the upper bound for Zone B. The effective Zone B
     * budget is @c zone_b_end - @c zone_b_base bytes.
     */
    uint8_t* zone_b_end;

    /**
     * @brief Runtime cell count in Zone B.
     *
     * Computed at init from the actual pool size; always
     * <= @c PUBNUB_ARENA_MAX_ZONE_B_CELLS.
     */
    size_t cell_total;

    /** @brief Zone B: per-cell in-use flag (1 = allocated, 0 = free). */
    uint8_t cell_in_use[PUBNUB_ARENA_MAX_ZONE_B_CELLS];

    /**
     * @brief Zone B: allocation size in cells at first cell of each alloc.
     *
     * The first cell of an N-cell allocation stores N; trailing cells
     * store 0. Used by @c arena_free to determine how many cells to
     * reclaim.
     */
    uint8_t cell_count[PUBNUB_ARENA_MAX_ZONE_B_CELLS];

    /**
     * @brief Init reference count for multi-context sharing.
     *
     * Incremented by the vtable @c init and decremented by @c deinit.
     * When a single arena instance backs several concurrently-live
     * contexts, the tracking arrays are cleared only when the last
     * context deinits (count reaches 0), so one context's teardown does
     * not corrupt another's live allocations. Reset to 0 by
     * @c pubnub_arena_allocator_init.
     */
    uint8_t ref_count;
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
 *                  targets. The pool buffer must be aligned to at least
 *                  @c sizeof(double) (use @c PUBNUB_ALIGNAS(double) to
 *                  ensure proper alignment); otherwise allocations requesting
 *                  alignment > the pool's natural alignment may silently
 * receive misaligned memory.
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
