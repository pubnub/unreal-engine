/* Copyright (c) PubNub Inc. */
/* See LICENSE in the root directory of this source tree. */

/**
 * @file providers/logger.h
 * @brief Logger provider interface (structured logging with levels).
 *
 * The logger provider receives structured log entries via a single
 * `log` function pointer. Entries are discriminated by type
 * (text, object, error, network request/response) and carry a
 * severity level for runtime filtering.
 *
 * Compile-time level stripping is supported via the
 * PUBNUB_CFG_LOG_LEVEL_COMPILED bitmask (defined in logger_types.h).
 * When a level bit is not set, the corresponding log macro expands to
 * nothing, enabling dead-code elimination.
 *
 * Callbacks in this vtable are never called from ISR context.
 *
 * ## Logger assignment semantics
 *
 * Setting `cfg.logger = X` registers X as an additional logger at
 * context creation (alongside the built-in default). To add or remove
 * loggers after creation, use pubnub_logger_add() / pubnub_logger_remove().
 *
 * Setting `cfg.logger = NULL` (the default) uses only the compiled-in
 * default logger selected at build time (stdout when
 * PUBNUB_PROVIDER_LOGGER=stdout).
 *
 * Convenience macros:
 *   PUBNUB_LOG_TEXT()   -- emit a plain text log entry
 *   PUBNUB_LOG_OBJECT() -- emit a structured data log entry
 *   PUBNUB_LOG_ERR()    -- emit an error log entry
 *
 * For printf-style convenience, PUBNUB_LOG() formats into a stack
 * buffer and emits a text entry.
 */

#ifndef PUBNUB_PROVIDER_LOGGER_H
#define PUBNUB_PROVIDER_LOGGER_H

#include "pubnub/config.h"
#include "pubnub/providers/logger_types.h"

#include <stdarg.h>
#include <stddef.h>

#ifdef __cplusplus
// clang-format off
extern "C" {
// clang-format on
#endif

/**
 * @brief Logger provider function table.
 *
 * All callbacks are invoked from normal (non-ISR) context only.
 *
 * This is a **shared** provider: the SDK never calls lifecycle
 * callbacks on it. Create, configure, and release the provider
 * externally; it is not owned by any context. Multiple contexts may share
 * the same logger instance.
 *
 * Implementation-specific state should be stored in an extended
 * struct with this vtable as the first member (cast `self` to
 * recover your type).
 */
typedef struct pubnub_logger_provider {
    /**
     * @brief Emit a structured log entry.
     *
     * The provider receives a base pubnub_log_entry_t pointer whose
     * `type` field identifies the concrete subtype. The provider may
     * cast to the appropriate concrete type to access subtype-specific
     * fields.
     *
     * The entry and all data it references (strings, value trees) are
     * valid only for the duration of this call. The provider must
     * not store pointers into the entry after returning.
     *
     * @note Invoked while the per-context mutex MAY be held. Do not
     *       call any SDK function that acquires the context mutex from
     *       within this callback (including pubnub_logger_add,
     *       pubnub_logger_remove, or any pubnub_* API on the same
     *       context) — doing so deadlocks on
     *       PUBNUB_CFG_THREAD_SAFETY=1 builds. Safe operations:
     *       printf/write(2), ring-buffer enqueue, semaphore signal to
     *       a dedicated logging thread.
     *
     * @note When a single provider instance is shared across multiple
     *       contexts, log() may be called concurrently from different
     *       threads — each thread holds its own context's mutex.
     *       Shared-provider implementations must be reentrant or use
     *       their own internal lock.
     *
     * @param self  Pointer to this provider instance.
     * @param entry Log entry (base pointer; cast per entry->type).
     */
    void (*log)(struct pubnub_logger_provider* self,
                const pubnub_log_entry_t*      entry);

    /**
     * @brief Set the minimum runtime log level (threshold model).
     *
     * Messages with a level value numerically below @p min_level are
     * dropped. Since levels are ordered by severity (TRACE=0x01 <
     * DEBUG=0x02 < INFO=0x04 < WARNING=0x08 < ERROR=0x10),
     * set_level(PUBNUB_LOG_LEVEL_WARNING) accepts WARNING and ERROR.
     *
     * This is orthogonal to compile-time stripping via
     * PUBNUB_CFG_LOG_LEVEL_COMPILED.
     *
     * @note Same concurrency constraint as log(): may be invoked while
     *       the per-context mutex is held. Do not call SDK functions
     *       that acquire the context mutex from within this callback.
     *
     * @param self      Pointer to this provider instance.
     * @param min_level Minimum severity to accept.
     */
    void (*set_level)(struct pubnub_logger_provider* self,
                      pubnub_log_level_t             min_level);

} pubnub_logger_provider_t;

/**
 * @brief Evaluates to non-zero if the given level is compiled in.
 *
 * Use inside #if preprocessor directives is not possible because the
 * level values are enum constants. Instead, use this as a C boolean
 * expression; the compiler will constant-fold and eliminate dead code
 * when the level is stripped.
 */
#define PUBNUB_LOG_LEVEL_ENABLED(lvl) \
    (((unsigned)(lvl) & (unsigned)(PUBNUB_CFG_LOG_LEVEL_COMPILED)) != 0u)

/**
 * @brief Emit a plain text log entry.
 *
 * @param prov  Pointer to pubnub_logger_provider_t (may be NULL).
 * @param lvl   pubnub_log_level_t severity.
 * @param msg   NUL-terminated message string (borrowed).
 */
#define PUBNUB_LOG_TEXT(prov, lvl, msg)                               \
    do {                                                              \
        if (PUBNUB_LOG_LEVEL_ENABLED(lvl) && (prov) && (prov)->log) { \
            pubnub_log_entry_text_t entry_ = {0};                     \
            entry_.base.type               = PUBNUB_LOG_ENTRY_TEXT;   \
            entry_.base.level              = (lvl);                   \
            entry_.base.file               = __FILE__;                \
            entry_.base.line               = __LINE__;                \
            entry_.message                 = (msg);                   \
            (prov)->log((prov), (const pubnub_log_entry_t*)&entry_);  \
        }                                                             \
    } while (0)

/**
 * @brief Emit a structured object log entry.
 *
 * @param prov   Pointer to pubnub_logger_provider_t (may be NULL).
 * @param lvl    pubnub_log_level_t severity.
 * @param lbl    Human-readable label (NUL-terminated, may be NULL).
 * @param vdata  Pointer to pubnub_log_value_t data tree (borrowed).
 */
#define PUBNUB_LOG_OBJECT(prov, lvl, lbl, vdata)                        \
    do {                                                                \
        if (PUBNUB_LOG_LEVEL_ENABLED(lvl) && (prov) && (prov)->log) {   \
            pubnub_log_entry_object_t entry_ = {0};                     \
            entry_.base.type                 = PUBNUB_LOG_ENTRY_OBJECT; \
            entry_.base.level                = (lvl);                   \
            entry_.base.file                 = __FILE__;                \
            entry_.base.line                 = __LINE__;                \
            entry_.label                     = (lbl);                   \
            entry_.data                      = (vdata);                 \
            (prov)->log((prov), (const pubnub_log_entry_t*)&entry_);    \
        }                                                               \
    } while (0)

/**
 * @brief Emit an error log entry.
 *
 * @param prov     Pointer to pubnub_logger_provider_t (may be NULL).
 * @param lvl      pubnub_log_level_t severity (usually PUBNUB_LOG_LEVEL_ERROR).
 * @param code     Integer error code.
 * @param msg      Human-readable error message (borrowed).
 * @param details_arg  Pointer to pubnub_log_value_t details (may be NULL).
 */
#define PUBNUB_LOG_ERR(prov, lvl, code, msg, details_arg)             \
    do {                                                              \
        if (PUBNUB_LOG_LEVEL_ENABLED(lvl) && (prov) && (prov)->log) { \
            pubnub_log_entry_error_t entry_ = {0};                    \
            entry_.base.type                = PUBNUB_LOG_ENTRY_ERROR; \
            entry_.base.level               = (lvl);                  \
            entry_.base.file                = __FILE__;               \
            entry_.base.line                = __LINE__;               \
            entry_.error_code               = (code);                 \
            entry_.error_message            = (msg);                  \
            entry_.details                  = (details_arg);          \
            (prov)->log((prov), (const pubnub_log_entry_t*)&entry_);  \
        }                                                             \
    } while (0)

/*
 * The PUBNUB_LOG() printf-style macro and its bridge function require
 * <stdio.h> (vsnprintf). On bare-metal targets where stdio is
 * unavailable or heavyweight, set PUBNUB_CFG_MAX_LOG_MESSAGE_SIZE=0
 * at compile time to exclude this facility. Use PUBNUB_LOG_TEXT()
 * instead on those targets.
 *
 * Total stack cost per PUBNUB_LOG() call is approximately
 * PUBNUB_CFG_MAX_LOG_MESSAGE_SIZE + 56 bytes. On targets with
 * limited stack (<4KB), prefer PUBNUB_LOG_TEXT() or set
 * PUBNUB_CFG_MAX_LOG_MESSAGE_SIZE=0.
 */

#if PUBNUB_CFG_MAX_LOG_MESSAGE_SIZE > 0

/**
 * @brief Internal variadic-to-text-entry bridge (not public API).
 *
 * Declared here, defined in src/core/log_variadic.c.
 * Use PUBNUB_LOG() macro instead of calling directly.
 */
#if defined(__clang__) || defined(__GNUC__)
__attribute__((format(printf, 5, 6)))
#endif
void
// NOLINTNEXTLINE(readability-identifier-naming)
pn_log_variadic_(pubnub_logger_provider_t* prov,
                 pubnub_log_level_t        level,
                 const char*               file,
                 int                       line,
                 const char*               fmt,
                 ...);

/**
 * @brief Printf-style convenience macro (formats into stack buffer).
 *
 * Formats the message into a stack-allocated buffer of
 * PUBNUB_CFG_MAX_LOG_MESSAGE_SIZE bytes and emits a text entry.
 * If the formatted message is truncated, the entry is still emitted
 * with the truncated content.
 *
 * @param prov  Pointer to pubnub_logger_provider_t (may be NULL).
 * @param lvl   pubnub_log_level_t severity.
 * @param fmt   printf-style format string.
 * @param ...   Format arguments.
 */
/* ##__VA_ARGS__ is a GCC/Clang extension for zero-arg comma elision.
 * Suppress the pedantic warning so callers can pass fmt-only without
 * extra variadic args. When C23 __VA_OPT__ is baseline, replace. */
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wgnu-zero-variadic-macro-arguments"
#elif defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wvariadic-macros"
#endif

#define PUBNUB_LOG(prov, lvl, fmt, ...)                                   \
    do {                                                                  \
        if (PUBNUB_LOG_LEVEL_ENABLED(lvl) && (prov) && (prov)->log) {     \
            pn_log_variadic_(                                             \
                (prov), (lvl), __FILE__, __LINE__, (fmt), ##__VA_ARGS__); \
        }                                                                 \
    } while (0)

#if defined(__clang__)
#pragma clang diagnostic pop
#elif defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

#else /* PUBNUB_CFG_MAX_LOG_MESSAGE_SIZE > 0 */

#define PUBNUB_LOG(prov, lvl, fmt, ...) ((void)0)

#endif /* PUBNUB_CFG_MAX_LOG_MESSAGE_SIZE > 0 */

#ifdef __cplusplus
// clang-format off
}
// clang-format on
#endif

#endif /* PUBNUB_PROVIDER_LOGGER_H */
