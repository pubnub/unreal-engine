/* Copyright (c) PubNub Inc. */
/* See LICENSE in the root directory of this source tree. */

/**
 * @file pubnub/log.h
 * @brief User-facing logging API: emit log messages and manage logger
 *        registrations on a context.
 *
 * Provider implementors and sink authors should include
 * @c pubnub/providers/logger.h directly.
 */

#ifndef PUBNUB_LOG_H
#define PUBNUB_LOG_H

#include "pubnub/config.h"
#include "pubnub/types_fwd.h"
#include "pubnub/providers/logger.h"
#include "pubnub/error.h"

#ifdef __cplusplus
// clang-format off
extern "C" {
// clang-format on
#endif

/**
 * @brief Emit a plain-text log message through the context's configured
 *        logger.
 *
 * No-op when logging is compiled out (@c PUBNUB_CFG_LOG_LEVEL_COMPILED == 0),
 * when @p ctx is @c NULL, or when @p message is @c NULL.
 *
 * @param ctx     Context whose logger receives the entry. NULL-safe (no-op).
 * @param level   Severity level.
 * @param message NUL-terminated message string (borrowed). NULL-safe.
 */
PUBNUB_API void pubnub_log_text(pubnub_context_t*  ctx,
                                pubnub_log_level_t level,
                                const char*        message);

/**
 * @brief Emit a structured data log entry through the context's configured
 *        logger.
 *
 * No-op when logging is compiled out or @p ctx is @c NULL.
 *
 * @param ctx   Context whose logger receives the entry. NULL-safe (no-op).
 * @param level Severity level.
 * @param label Human-readable label string (borrowed, may be NULL).
 * @param value Structured data payload — borrowed, stack-allocated value tree
 *              (may be NULL). The tree must remain valid for the duration of
 *              this call; the logger does not retain pointers after returning.
 *
 * @note On stack-constrained embedded targets, guard the code that builds
 *       @c pubnub_log_value_t trees with
 *       @c if @c (PUBNUB_CFG_LOG_LEVEL_COMPILED) to avoid allocating stack
 *       nodes when logging is compiled out.
 */
PUBNUB_API void pubnub_log_object(pubnub_context_t*         ctx,
                                  pubnub_log_level_t        level,
                                  const char*               label,
                                  const pubnub_log_value_t* value);

/**
 * @brief Emit an error log entry at ERROR level through the context's
 *        configured logger.
 *
 * No-op when logging is compiled out or @p ctx is @c NULL.
 *
 * @param ctx        Context whose logger receives the entry. NULL-safe (no-op).
 * @param error_code SDK error code (@c pubnub_res_t cast to @c int) or system
 *                   errno value.
 * @param message    Human-readable error description (borrowed). NULL-safe.
 * @param details    Optional structured details — borrowed stack-allocated
 *                   value tree (may be NULL).
 */
PUBNUB_API void pubnub_log_error(pubnub_context_t*         ctx,
                                 int                       error_code,
                                 const char*               message,
                                 const pubnub_log_value_t* details);

#if PUBNUB_CFG_MAX_LOG_MESSAGE_SIZE > 0
/**
 * @brief Emit a printf-style formatted log message through the context's
 *        configured logger.
 *
 * The format string is expanded into a stack buffer of
 * @c PUBNUB_CFG_MAX_LOG_MESSAGE_SIZE bytes using @c pn_vsnprintf. Messages
 * longer than the buffer are silently truncated to fit.
 *
 * For large structured data, prefer @c pubnub_log_object() instead of
 * formatting into this buffer.
 *
 * Not available when @c PUBNUB_CFG_MAX_LOG_MESSAGE_SIZE == 0 (bare-metal
 * builds where stack size or the absence of @c stdio.h makes formatting
 * impractical).
 *
 * @param ctx   Context whose logger receives the entry. NULL-safe (no-op).
 * @param level Severity level.
 * @param fmt   printf-compatible format string. Must not be NULL.
 * @param ...   Format arguments.
 */
PUBNUB_API void pubnub_log_text_formatted(pubnub_context_t*  ctx,
                                          pubnub_log_level_t level,
                                          const char*        fmt,
                                          ...);
#endif /* PUBNUB_CFG_MAX_LOG_MESSAGE_SIZE > 0 */

/**
 * @brief Register an additional logger provider with a context.
 *
 * The provider is added alongside the built-in default logger (if any).
 * All registered providers receive every log entry. The provider is
 * borrowed — it must outlive the context or be removed before the context
 * is destroyed.
 *
 * @note Thread safety: acquires the per-context mutex. Safe to call from
 *       any thread. Do not call from within a log() or set_level()
 *       callback for the same context — deadlocks on
 *       PUBNUB_CFG_THREAD_SAFETY=1 builds.
 *
 * @param ctx    Initialized context. Must not be NULL.
 * @param logger Provider to add. Must not be NULL.
 * @return @c PUBNUB_OK on success; @c PUBNUB_ERR_INVALID_ARGUMENT for NULL
 *         or uninitialized inputs; @c PUBNUB_ERR_QUEUE_FULL when the
 *         per-context logger limit (@c PUBNUB_CFG_MAX_LOGGERS) is reached.
 */
PUBNUB_API pubnub_res_t pubnub_logger_add(pubnub_context_t*         ctx,
                                          pubnub_logger_provider_t* logger);

/**
 * @brief Remove a previously-added logger provider from a context.
 *
 * @note Thread safety: acquires the per-context mutex. Safe to call from
 *       any thread. Do not call from within a log() or set_level()
 *       callback for the same context — deadlocks on
 *       PUBNUB_CFG_THREAD_SAFETY=1 builds.
 *
 * @param ctx    Initialized context. Must not be NULL.
 * @param logger Provider to remove. Must not be NULL.
 * @return @c PUBNUB_OK on success; @c PUBNUB_ERR_INVALID_ARGUMENT if NULL,
 *         uninitialized, or the provider is not found.
 */
PUBNUB_API pubnub_res_t pubnub_logger_remove(pubnub_context_t*         ctx,
                                             pubnub_logger_provider_t* logger);

/**
 * @brief Remove all registered logger providers from a context.
 *
 * After this call the context has no active logger sinks. Use
 * @c pubnub_set_log_level() with @c PUBNUB_LOG_LEVEL_NONE to suppress output
 * temporarily without removing providers.
 *
 * @param ctx Context to clear. NULL-safe (no-op).
 */
PUBNUB_API void pubnub_logger_remove_all(pubnub_context_t* ctx);

/**
 * @brief Query the current minimum log level of a context.
 *
 * @param ctx Context to query. May be NULL.
 * @return Current minimum log level, or @c PUBNUB_LOG_LEVEL_NONE when @p ctx
 *         is NULL or uninitialized.
 * @see pubnub_set_log_level
 */
PUBNUB_API pubnub_log_level_t pubnub_logger_log_level(pubnub_context_t* ctx);

#ifdef __cplusplus
// clang-format off
}
// clang-format on
#endif

#endif /* PUBNUB_LOG_H */
