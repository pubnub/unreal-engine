/* Copyright (c) PubNub Inc. */
/* See LICENSE in the root directory of this source tree. */

/**
 * @file pubnub_compat.h
 * @brief Portability helpers for code that must compile across C99/C11 and
 *        across compilers with varying preprocessor support.
 *
 * Public, installed alongside @c pubnub/error.h. Pure macros only -- no
 * function declarations, no runtime cost.
 */

#ifndef PUBNUB_COMPAT_H
#define PUBNUB_COMPAT_H

#include <stdint.h>

/* Two-level paste so __LINE__ / __COUNTER__ expand inside ## . */
#define PUBNUB_PASTE_(a, b) a##b // NOLINT(readability-identifier-naming)
#define PUBNUB_PASTE(a, b)  PUBNUB_PASTE_(a, b)

/**
 * @brief Compile-time assertion macro.
 *
 * Resolves to C11 @c _Static_assert when available. On C99-only toolchains
 * it falls back to a typedef'd negative-size array trick whose name is
 * disambiguated by @c __COUNTER__ (when supported and accepted by the
 * compiler in the selected language mode) or @c __LINE__.
 *
 * @param cond Compile-time integer-constant expression. Asserted non-zero.
 * @param msg  Diagnostic message (used by the C11 path; ignored by the
 *             typedef fallback because C99 forbids string literals in
 *             typedef names).
 *
 * @note Same-line restriction: on compilers without @c __COUNTER__
 *       (notably IAR pre-9.30 and ARMCC pre-6), and on Clang in C99 mode
 *       (where @c __COUNTER__ is rejected under strict pedantic checks),
 *       two @c PUBNUB_STATIC_ASSERT invocations on the same source line
 *       will collide on the typedef name. Place each on its own line. C11
 *       and the @c __COUNTER__ path are immune.
 */
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
#define PUBNUB_STATIC_ASSERT(cond, msg) _Static_assert((cond), msg)
#elif defined(__COUNTER__) && !defined(__clang__)
#define PUBNUB_STATIC_ASSERT(cond, msg) \
    typedef char PUBNUB_PASTE(pubnub_sa_, __COUNTER__)[(cond) ? 1 : -1]
#else
#define PUBNUB_STATIC_ASSERT(cond, msg) \
    typedef char PUBNUB_PASTE(pubnub_sa_, __LINE__)[(cond) ? 1 : -1]
#endif

/**
 * @brief Atomic uint8 type and load/store accessors for lazy-init flags.
 *
 * Three tiers based on compiler/standard support:
 *
 * - **C11 with atomics** (`__STDC_NO_ATOMICS__` absent): uses
 *   `_Atomic uint8_t` with `memory_order_acquire` / `memory_order_release`.
 * - **C99 GCC/Clang**: `volatile uint8_t` with `__sync_*` barriers.
 * - **Fallback**: plain `volatile uint8_t`; safe only for
 *   single-threaded or caller-serialized use — document the
 *   limitation at the call site.
 *
 * @c PUBNUB_ATOMIC_EXCHANGE_U8 atomically stores a new value and returns
 * the prior one. Use it to claim-and-clear a publication gate in a single
 * step: a plain load-then-store leaves a window in which a concurrent
 * writer can re-arm the gate between the read and the clear, silently
 * dropping the staged data.
 *
 * @note The GCC/Clang `__sync_fetch_and_add((p), 0)` pattern is the
 *       canonical way to emit an acquire-load without a compare-exchange
 *       on pre-C11 ABIs. The double-barrier store is conservative; a
 *       single `__sync_synchronize()` before the write suffices for
 *       release, but two barriers match what other portable SDKs use.
 *       `__sync_lock_test_and_set` carries acquire semantics, which is
 *       sufficient for claiming a gate whose staged data was published
 *       with a release store.
 */
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L \
    && !defined(__STDC_NO_ATOMICS__) && !defined(__ZEPHYR__)
#include <stdatomic.h>
#define PUBNUB_ATOMIC_UINT8      _Atomic uint8_t
#define PUBNUB_ATOMIC_LOAD_U8(p) atomic_load_explicit((p), memory_order_acquire)
#define PUBNUB_ATOMIC_STORE_U8(p, v) \
    atomic_store_explicit((p), (uint8_t)(v), memory_order_release)
#define PUBNUB_ATOMIC_EXCHANGE_U8(p, v) \
    atomic_exchange_explicit((p), (uint8_t)(v), memory_order_acq_rel)
#elif defined(__GNUC__) || defined(__clang__)
#define PUBNUB_ATOMIC_UINT8      volatile uint8_t
#define PUBNUB_ATOMIC_LOAD_U8(p) __sync_fetch_and_add((p), 0)
#define PUBNUB_ATOMIC_STORE_U8(p, v) \
    do {                             \
        __sync_synchronize();        \
        *(p) = (uint8_t)(v);         \
        __sync_synchronize();        \
    } while (0)
#define PUBNUB_ATOMIC_EXCHANGE_U8(p, v) \
    __sync_lock_test_and_set((p), (uint8_t)(v))
#else
/* Non-atomic fallback: single-threaded or caller-serialized contexts only. */
#define PUBNUB_ATOMIC_UINT8          volatile uint8_t
#define PUBNUB_ATOMIC_LOAD_U8(p)     (*(p))
#define PUBNUB_ATOMIC_STORE_U8(p, v) (*(p) = (uint8_t)(v))
#define PUBNUB_ATOMIC_EXCHANGE_U8(p, v) \
    pubnub_atomic_exchange_u8_((p), (uint8_t)(v))

/**
 * @brief Serialized-context exchange helper for the non-atomic fallback tier.
 *
 * Reads the current value, stores @p v, and returns the prior value. Safe
 * only for single-threaded or caller-serialized use — matches the ordering
 * guarantees (none) of the fallback load/store macros.
 *
 * @param p Pointer to the byte to exchange. Must not be NULL.
 * @param v New value to store.
 * @return The value held before the store.
 */
static inline uint8_t pubnub_atomic_exchange_u8_(volatile uint8_t* p, uint8_t v)
{
    uint8_t old = *p;
    *p          = v;
    return old;
}
#endif

/**
 * @brief Portable alignment specifier for static storage buffers.
 *
 * Use @c PUBNUB_ALIGNAS(max_align_t) when declaring a @c uint8_t[]
 * buffer that will be cast to a struct pointer via @c pubnub_init().
 * Without an alignment specifier, @c alignof(uint8_t)==1 by the C
 * standard; casting such a buffer to a struct with stricter alignment
 * requirements is undefined behaviour and causes a HardFault on
 * Cortex-M0 and similar targets.
 *
 * Three tiers based on compiler/standard support:
 *
 * - **C11**: uses @c _Alignas (standardised).
 * - **GCC/Clang C99**: uses @c __attribute__((aligned(__alignof__(type)))).
 * - **MSVC**: uses @c __declspec(align(8)), a safe conservative bound.
 * - **Fallback**: expands to nothing; verify alignment manually on the
 *   target toolchain and document the decision.
 */
#if defined(_MSC_VER)
/* MSVC _Alignas does not accept type operands reliably; use __declspec. */
#define PUBNUB_ALIGNAS(type) __declspec(align(8))
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
#define PUBNUB_ALIGNAS(type) _Alignas(type)
#elif defined(__GNUC__) || defined(__clang__)
#define PUBNUB_ALIGNAS(type) __attribute__((aligned(__alignof__(type))))
#else
/* Fallback: verify alignment manually on this toolchain. */
#define PUBNUB_ALIGNAS(type)
#endif

/**
 * @brief Portable "do not inline this function" specifier.
 *
 * Apply to a cold function that owns a large stack buffer. Without it an
 * optimizing compiler may inline the function into a hot caller, which
 * hoists the buffer into the caller's prologue so it is allocated on
 * every call — including the paths that never reach the cold code. On a
 * constrained target that turns a rarely-used buffer into a permanent
 * cost on the hot path.
 *
 * Use sparingly and only with a stack-usage measurement to back it up:
 * blocking inlining costs a call and can cost code size.
 *
 * Place the macro at the very START of the declaration, before the
 * storage-class specifier (@c PUBNUB_NOINLINE @c static @c int @c f(void)).
 * On IAR the macro expands to a pragma, which is only honoured when it
 * precedes the whole declaration — @c static @c PUBNUB_NOINLINE @c int
 * silently loses the guarantee there.
 *
 * - **MSVC**: @c __declspec(noinline).
 * - **GCC/Clang**: @c __attribute__((noinline)).
 * - **IAR (ARM/RX)**: @c _Pragma("inline=never").
 * - **Arm Compiler 5**: @c __attribute__((noinline)).
 * - **Fallback**: expands to nothing; the buffer may be hoisted, so
 *   verify stack usage on the target toolchain.
 */
#if defined(_MSC_VER)
#define PUBNUB_NOINLINE __declspec(noinline)
#elif defined(__GNUC__) || defined(__clang__)
#define PUBNUB_NOINLINE __attribute__((noinline))
#elif defined(__ICCARM__) || defined(__ICCRX__)
#define PUBNUB_NOINLINE _Pragma("inline=never")
#elif defined(__CC_ARM)
#define PUBNUB_NOINLINE __attribute__((noinline))
#else
/* Fallback: verify stack usage manually on this toolchain. */
#define PUBNUB_NOINLINE
#endif

/**
 * @brief Shared-library symbol decoration macro.
 *
 * When building the SDK as a shared library, public API functions are
 * decorated with platform-specific export/import attributes:
 *
 * - @c PUBNUB_SHARED_EXPORT (defined on SDK build targets): marks
 *   symbols for export from the shared library.
 * - @c PUBNUB_SHARED (defined on consumer targets via CMake INTERFACE
 *   property): marks symbols for import from the shared library.
 * - Neither defined (static build): no decoration.
 *
 * Apply to every non-static function declaration in public headers.
 * Do NOT apply to struct/typedef/enum declarations, macros, or
 * static inline functions.
 */
#if defined(PUBNUB_SHARED_EXPORT)
#if defined(_WIN32) || defined(__CYGWIN__)
#define PUBNUB_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
#define PUBNUB_API __attribute__((visibility("default")))
#else
#define PUBNUB_API
#endif
#elif defined(PUBNUB_SHARED)
#if defined(_WIN32) || defined(__CYGWIN__)
#define PUBNUB_API __declspec(dllimport)
#else
#define PUBNUB_API
#endif
#else
#define PUBNUB_API
#endif

#endif /* PUBNUB_COMPAT_H */
