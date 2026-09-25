/* Copyright (c) PubNub Inc. */
/* See LICENSE in the root directory of this source tree. */
/* Generated from config.h.in by CMake - do not edit. */

#ifndef PUBNUB_CONFIG_H
#define PUBNUB_CONFIG_H

#define PUBNUB_SDK_VERSION_MAJOR 0
#define PUBNUB_SDK_VERSION_MINOR 1
#define PUBNUB_SDK_VERSION_PATCH 0
#define PUBNUB_SDK_VERSION       "0.1.0"

/**
 * @brief Platform identifier injected at configure time.
 *
 * Defaults to CMAKE_SYSTEM_NAME (e.g. "Linux", "Darwin", "Windows").
 * Overridable via -DPUBNUB_SDK_PLATFORM=FreeRTOS for embedded targets.
 */
#define PUBNUB_SDK_PLATFORM      "Windows"

/**
 * @brief Canonical SDK identifier used in the pnsdk query parameter.
 *
 * Format: `<OS>-PubNub-C-core/MAJOR.MINOR.PATCH`. Assembled at
 * preprocessing time via string-literal concatenation, so it has
 * no runtime cost. Override via -DPUBNUB_SDK_IDENTIFIER="..." if needed.
 */
#ifndef PUBNUB_SDK_IDENTIFIER
#define PUBNUB_SDK_IDENTIFIER    PUBNUB_SDK_PLATFORM "-PubNub-C-core/" PUBNUB_SDK_VERSION
#endif

/* Wire feature toggles (PUBNUB_ENABLE_*).
   Gate user-facing protocol capabilities. Disabling excludes the
   corresponding feature module entirely. */
#define PUBNUB_ENABLE_PUBLISH 1
#define PUBNUB_ENABLE_SUBSCRIBE 1
#define PUBNUB_ENABLE_PRESENCE 1
#define PUBNUB_ENABLE_HISTORY 1
#define PUBNUB_ENABLE_MESSAGE_ACTIONS 1
#define PUBNUB_ENABLE_SIGNAL 1
#define PUBNUB_ENABLE_TIME 1
#define PUBNUB_ENABLE_PAM 1
#define PUBNUB_ENABLE_APP_CONTEXT 1
#define PUBNUB_ENABLE_FILES 1
#define PUBNUB_ENABLE_FILESYSTEM 1
#define PUBNUB_ENABLE_CHANNEL_GROUPS 1
#define PUBNUB_ENABLE_CRYPTO 1
#define PUBNUB_ENABLE_PUSH_NOTIFICATIONS 1
#define PUBNUB_ENABLE_RETRY 0
#define PUBNUB_ENABLE_SECURE_TRANSPORT 1
#define PUBNUB_ENABLE_CPP_WRAPPER 0

/* When 1, pubnub_create()/pubnub_destroy() are unavailable;
   only pubnub_init()/pubnub_deinit() with caller-provided memory work. */
#define PUBNUB_CFG_NO_HEAP 0

/*
 * Compile-time tunables (PUBNUB_CFG_*).
 *
 * When built via CMake, values are substituted by configure_file.
 * For non-CMake builds, override at compile time via -DPUBNUB_CFG_<NAME>=<value>.
 */

#ifndef PUBNUB_CFG_MAX_IN_FLIGHT_REQUESTS
#define PUBNUB_CFG_MAX_IN_FLIGHT_REQUESTS 4
#endif

#ifndef PUBNUB_CFG_MAX_PENDING_REQUESTS
#define PUBNUB_CFG_MAX_PENDING_REQUESTS 8
#endif

#ifndef PUBNUB_CFG_REQUEST_BUFFER_SIZE
#define PUBNUB_CFG_REQUEST_BUFFER_SIZE 4096
#endif

#ifndef PUBNUB_CFG_RESPONSE_BUFFER_SIZE
#define PUBNUB_CFG_RESPONSE_BUFFER_SIZE 32768
#endif

#ifndef PUBNUB_CFG_URL_BUFFER_SIZE
#define PUBNUB_CFG_URL_BUFFER_SIZE 2048
#endif

#define PUBNUB_CFG_MAX_SUBSCRIBE_CHANNELS 64

#ifndef PUBNUB_CFG_SUBSCRIBE_MAX_BATCH_SIZE
#define PUBNUB_CFG_SUBSCRIBE_MAX_BATCH_SIZE 100
#endif

#ifndef PUBNUB_CFG_MAX_SUBSCRIBE_LISTENERS
#define PUBNUB_CFG_MAX_SUBSCRIBE_LISTENERS 8
#endif

/**
 * @brief Maximum milliseconds to block in transport poll per iteration.
 *
 * Controls the blocking duration for both the background thread poll loop
 * and the cooperative @c pubnub_await fallback. On RTOS targets with a
 * hardware watchdog, keep this well below the watchdog timeout.
 */
#ifdef PUBNUB_CFG_BG_POLL_INTERVAL_MS
  #error "PUBNUB_CFG_BG_POLL_INTERVAL_MS was removed. Use PUBNUB_CFG_MAX_POLL_MS instead."
#endif
#ifdef PUBNUB_CFG_BG_IDLE_INTERVAL_MS
  #error "PUBNUB_CFG_BG_IDLE_INTERVAL_MS was removed. Use PUBNUB_CFG_MAX_POLL_MS instead."
#endif
#ifdef PUBNUB_CFG_BG_MAX_POLL_MS
  #error "PUBNUB_CFG_BG_MAX_POLL_MS was renamed to PUBNUB_CFG_MAX_POLL_MS."
#endif
#ifndef PUBNUB_CFG_MAX_POLL_MS
#define PUBNUB_CFG_MAX_POLL_MS 100
#endif

#ifndef PUBNUB_CFG_TRANSACTION_TIMEOUT_MS
#define PUBNUB_CFG_TRANSACTION_TIMEOUT_MS 10000
#endif

#ifndef PUBNUB_CFG_NON_TRANSACTION_TIMEOUT_MS
#define PUBNUB_CFG_NON_TRANSACTION_TIMEOUT_MS 310000
#endif

/* Retry tunables (only meaningful when PUBNUB_ENABLE_RETRY is set). */
#ifndef PUBNUB_CFG_RETRY_DELAY_MS
#define PUBNUB_CFG_RETRY_DELAY_MS 2000
#endif

#ifndef PUBNUB_CFG_RETRY_MAX_DELAY_MS
#define PUBNUB_CFG_RETRY_MAX_DELAY_MS 150000
#endif

#ifndef PUBNUB_CFG_LINEAR_MAX_RETRIES
#define PUBNUB_CFG_LINEAR_MAX_RETRIES 10
#endif

#ifndef PUBNUB_CFG_EXPONENTIAL_MAX_RETRIES
#define PUBNUB_CFG_EXPONENTIAL_MAX_RETRIES 6
#endif

/* Files feature tunables (only meaningful when PUBNUB_ENABLE_FILES is set). */
#ifndef PUBNUB_CFG_FILE_UPLOAD_TIMEOUT_MS
#define PUBNUB_CFG_FILE_UPLOAD_TIMEOUT_MS 300000
#endif

/* Files feature tunables (continued) */
#ifndef PUBNUB_CFG_FILES_MAX_DOWNLOAD_SIZE
#define PUBNUB_CFG_FILES_MAX_DOWNLOAD_SIZE 0
#endif

#define PUBNUB_CFG_ARENA_POOL_SIZE 65536
/* #undef PUBNUB_CFG_ARENA_ALLOC_BUDGET */
/* #undef PUBNUB_ARENA_MAX_ZONE_B_CELLS */

/**
 * @brief Enable verbose arena allocator debug output to stderr.
 *
 * When @c 1, @c arena_alloc prints a diagnostic line on every allocation
 * failure (OOM) and @c arena_buf_acquire prints the in-use bitmap on
 * backpressure. Requires stdio. Off by default; enable only for desktop
 * development builds where stderr is available.
 */
#define PUBNUB_CFG_ARENA_DEBUG 0

/**
 * @brief Warn about unreleased futures at context teardown.
 *
 * When @c 1, @c pn_request_pool_deinit logs a @c PUBNUB_LOG_LEVEL_WARN
 * message for every pool slot that is not idle when the context is
 * destroyed. Helps catch missing @c pubnub_future_release calls during
 * development. Off by default; enable only in debug builds.
 */
#define PUBNUB_CFG_ASSERT_POOL_CLEAN 0

/**
 * 1 when the SDK statically owns the arena backing pool (hosted profiles).
 * 0 when the application must provide the pool (embedded profile).
 *
 * When 0, pn_allocator_default() returns NULL and pubnub_config_t::allocator
 * must be set by the caller. PUBNUB_CFG_ARENA_POOL_SIZE remains defined as
 * the recommended minimum pool size regardless of this toggle.
 */
#define PUBNUB_CFG_ARENA_POOL_OWNER_SDK 1

/**
 * @brief Compile-time upper bound for static context storage.
 *
 * Use as the array size for a static buffer passed to
 * @c pubnub_init() on targets where heap allocation is unavailable.
 * A @c PUBNUB_STATIC_ASSERT in @c client.c guards against the formula
 * drifting below the real @c sizeof(struct pubnub_context).
 *
 * @code
 * static PUBNUB_ALIGNAS(max_align_t) uint8_t ctx_mem[PUBNUB_CONTEXT_SIZE];
 * pubnub_context_t* ctx = (pubnub_context_t*)ctx_mem;
 * pubnub_init(ctx, &cfg);
 * @endcode
 *
 * @note Not available in shared-library builds (@c PUBNUB_BUILD_SHARED=ON).
 *       Use @c pubnub_context_size() for runtime size queries instead.
 */
#if !defined(PUBNUB_SHARED_EXPORT) && !defined(PUBNUB_SHARED)
#define PUBNUB_CONTEXT_SIZE 1536
#endif

#define PUBNUB_CFG_MAX_LOG_MESSAGE_SIZE 512

#define PUBNUB_CFG_LOG_LEVEL_COMPILED 0x1E

#ifndef PUBNUB_CFG_MAX_LOGGERS
#define PUBNUB_CFG_MAX_LOGGERS 4
#endif

#ifndef PUBNUB_CFG_HTTP_MAX_PATH_SEGMENTS
#define PUBNUB_CFG_HTTP_MAX_PATH_SEGMENTS 10
#endif

#ifndef PUBNUB_CFG_HTTP_MAX_QUERY_PARAMS
#define PUBNUB_CFG_HTTP_MAX_QUERY_PARAMS 13
#endif

#ifndef PUBNUB_CFG_HTTP_MAX_HEADERS
#define PUBNUB_CFG_HTTP_MAX_HEADERS 8
#endif

#ifndef PUBNUB_CFG_HTTP_MAX_RESP_HEADERS
#define PUBNUB_CFG_HTTP_MAX_RESP_HEADERS 8
#endif

#ifndef PUBNUB_CFG_HTTP_SCRATCH_SIZE
#define PUBNUB_CFG_HTTP_SCRATCH_SIZE 32768
#endif

#ifndef PUBNUB_CFG_PUBLISH_META_BUF_SIZE
#define PUBNUB_CFG_PUBLISH_META_BUF_SIZE 4096
#endif

#ifndef PUBNUB_CFG_OBJECT_BUFFER_SIZE
#define PUBNUB_CFG_OBJECT_BUFFER_SIZE 33792
#endif

/**
 * @brief Maximum OBJ buffer (POST/PATCH body) size in bytes.
 *
 * Caps dynamic growth of the OBJ buffer during serialization.
 * Set to @c 0 to disable the SDK-level cap (the allocator's own
 * limits are then the only bound). Embedded profiles typically
 * set this to @c 0 because the arena allocator's NULL @c buf_grow
 * prevents any growth.
 *
 * Setting this smaller than @c PUBNUB_CFG_OBJECT_BUFFER_SIZE
 * effectively disables growth (the initial buffer already exceeds
 * the cap), which is functionally equivalent to embedded behavior.
 */
#ifndef PUBNUB_CFG_MAX_OBJ_BUFFER_SIZE
#define PUBNUB_CFG_MAX_OBJ_BUFFER_SIZE 8388608
#endif

/**
 * @brief Maximum RX buffer (response body) size in bytes.
 *
 * Caps dynamic growth of the response buffer, including
 * Content-Length pre-sizing and reactive doubling. Set to @c 0
 * to disable the SDK-level cap.
 *
 * Setting this smaller than @c PUBNUB_CFG_RESPONSE_BUFFER_SIZE
 * effectively disables growth for the response buffer.
 */
#ifndef PUBNUB_CFG_MAX_RESPONSE_BUFFER_SIZE
#define PUBNUB_CFG_MAX_RESPONSE_BUFFER_SIZE 16777216
#endif

#ifndef PUBNUB_CFG_SCRATCH_BUFFER_SIZE
#define PUBNUB_CFG_SCRATCH_BUFFER_SIZE 4096
#endif

#ifndef PUBNUB_CFG_ORIGIN
#define PUBNUB_CFG_ORIGIN "ps.pndsn.com"
#endif

/**
 * @brief Maximum number of middleware layers a single pipeline may own.
 *
 * Sized to fit the canonical chain ( @c pnsdk + @c userid + @c auth + @c signature)
 * plus headroom for additional middlewares ( @c retry, @c telemetry, @c gzip, etc.)
 * without touching this cap.
 *
 * @note The configure-time floor of @b 3 enforced below keeps @c pnsdk + @c userid + @c auth
 *       available even when signing is compiled out.
 */
#ifndef PUBNUB_CFG_PIPELINE_MAX_MIDDLEWARES
#define PUBNUB_CFG_PIPELINE_MAX_MIDDLEWARES 8
#endif
#define PUBNUB_CFG_MINIMAL_FORMATTER 0

/* Crypto module tunables. */
#ifndef PUBNUB_CFG_CRYPTO_MAX_FALLBACK_CRYPTORS
#define PUBNUB_CFG_CRYPTO_MAX_FALLBACK_CRYPTORS 4
#endif

/* Compile-time footprint / quality toggles ( @c PUBNUB_CFG_*).
   Tune how the implementation is shaped without changing wire surface. */

/* Per-context mutex for thread-safe same-context access. */
#define PUBNUB_CFG_THREAD_SAFETY 1

/* Stringifier for pubnub_res_t values. When @c 0, @c pubnub_res_str
   returns "" for every input - saves ~400-500 B @c .rodata on Cortex-M0. */
#define PUBNUB_CFG_RES_STR 1

/**
 * @brief Maximum JSON object/array nesting depth accepted by the
 *        @c serialization provider's parser and tolerated by the
 *        recursive destroy walker.
 *
 * Bounds the worst-case stack consumption of recursive parse and free
 * paths in JSON backends. Hostile or malformed inputs with depth
 * beyond this value cause @c parse to return @c NULL with no allocation
 * leaks; tree construction APIs that would push past the limit fail
 * the same way.
 *
 * @b Default: @c 16 (hosted profiles), reduced in the bare-metal profile.
 */
#ifndef PUBNUB_CFG_JSON_MAX_NESTING_DEPTH
#define PUBNUB_CFG_JSON_MAX_NESTING_DEPTH 16
#endif

/**
 * @brief Compile-time toggle for the optional JSON helper layer.
 *
 * When @c 1, the richer helpers in @c pubnub/json.h are compiled in
 * (varargs object builder, deep clone, debug-string serializer) and
 * the macros in @c pubnub/json_macros.h are usable. When @c 0, those
 * helpers and macros are excluded from translation entirely; the
 * always-on small wrappers ( @c pubnub_json_object_set_str,
 * @c pubnub_json_array_append, @c pubnub_json_destroy) remain
 * available regardless.
 *
 * @b Default: @c ON for hosted profiles (full, minimal) or @c OFF for the
 * embedded profile, where the varargs builder's stack pressure and
 * the clone walker's code size are not justified for the typical
 * tight-footprint targets.
 */
#define PUBNUB_CFG_JSON_HELPERS 1

/**
 * @brief Compile-time toggle for floating-point JSON values.
 *
 * When @c 1, the serialization provider exposes the @c value_create_double
 * constructor and @c value_as_double accessor; floating-point JSON
 * tokens parse into the @c PUBNUB_JSON_DOUBLE node type. When @c 0,
 * the corresponding @c vtable entries MUST be NULL and floating-point
 * literals encountered during parse fall back to backend-defined
 * handling (typically: best-effort int conversion or parse failure).
 *
 * @b Default: @c ON for hosted profiles ( @c full, @c minimal) and the
 * embedded profile (most RTOS targets have soft-float support). @c OFF
 * for the bare-metal profile (Cortex-M0/M0+ without an FPU), where
 * the libc double-formatting code is itself prohibitively large.
 */
#define PUBNUB_CFG_JSON_DOUBLE 1

/* Socket transport configuration */
#ifndef PUBNUB_CFG_SOCKET_HEADER_BUF_SIZE
#define PUBNUB_CFG_SOCKET_HEADER_BUF_SIZE 2048
#endif

#ifndef PUBNUB_CFG_SOCKET_CONNECT_TIMEOUT_MS
#define PUBNUB_CFG_SOCKET_CONNECT_TIMEOUT_MS 10000
#endif

#ifndef PUBNUB_CFG_SOCKET_KEEPALIVE_MAX_IDLE_MS
#define PUBNUB_CFG_SOCKET_KEEPALIVE_MAX_IDLE_MS 50000
#endif

#ifndef PUBNUB_CFG_SOCKET_KEEPALIVE_MAX_REQUESTS
#define PUBNUB_CFG_SOCKET_KEEPALIVE_MAX_REQUESTS 1000
#endif

#ifndef PUBNUB_CFG_SOCKET_DECOMP_MAX_BUFFER_SIZE
#define PUBNUB_CFG_SOCKET_DECOMP_MAX_BUFFER_SIZE 0
#endif

/**
 * Maximum HTTP redirect hops followed per request (0..8).
 *
 * Set to 0 to compile the redirect path out entirely. Following a
 * redirect requires building a second request descriptor, which costs
 * roughly 1 KB of stack on the hop, so constrained targets that do not
 * need redirects should keep this at 0. File download requires it
 * (PubNub returns 307 to object storage), so leave it non-zero whenever
 * PUBNUB_ENABLE_FILES is on.
 *
 * @b Default: @c 3 for hosted profiles ( @c full, @c minimal), @c 0 for
 * @c embedded.
 */
#ifndef PUBNUB_CFG_SOCKET_MAX_REDIRECTS
#define PUBNUB_CFG_SOCKET_MAX_REDIRECTS 3
#endif

#ifndef PUBNUB_CFG_MAX_DNS_RESULTS
#define PUBNUB_CFG_MAX_DNS_RESULTS 8
#endif

#ifndef PUBNUB_CFG_DNS_CACHE_SIZE
#define PUBNUB_CFG_DNS_CACHE_SIZE 8
#endif

#ifndef PUBNUB_CFG_MAX_DNS_SERVERS
#define PUBNUB_CFG_MAX_DNS_SERVERS 8
#endif

#ifndef PUBNUB_CFG_MAX_HOSTNAME_LEN
#define PUBNUB_CFG_MAX_HOSTNAME_LEN 256
#endif

#ifndef PUBNUB_CFG_DNS_PLATFORM_STATE_SIZE
#define PUBNUB_CFG_DNS_PLATFORM_STATE_SIZE 512
#endif

/**
 * @brief Cap for cached DNS entries (seconds).
 *
 * When using the built-in UDP resolver the actual server TTL
 * (PubNub: 300 s) is used; this cap only restricts entries where
 * the server returns a longer TTL or where the platform OS resolver
 * provides no TTL.
 */
#ifndef PUBNUB_CFG_DNS_MAX_TTL_SEC
#define PUBNUB_CFG_DNS_MAX_TTL_SEC 300
#endif

#define PUBNUB_ENABLE_CUSTOM_DNS 1
#define PUBNUB_ENABLE_COMPRESSION 1
#define PUBNUB_ENABLE_REQUEST_COMPRESSION 1
#define PUBNUB_ENABLE_PROXY 1
#define PUBNUB_ENABLE_IPV6 1

/* Compile-time validation. */

#if PUBNUB_CFG_MAX_PENDING_REQUESTS < 1
  #error "PUBNUB_CFG_MAX_PENDING_REQUESTS must be >= 1"
#endif

#if PUBNUB_CFG_REQUEST_BUFFER_SIZE < 256
  #error "PUBNUB_CFG_REQUEST_BUFFER_SIZE must be >= 256"
#endif

#if PUBNUB_CFG_RESPONSE_BUFFER_SIZE < 256
  #error "PUBNUB_CFG_RESPONSE_BUFFER_SIZE must be >= 256"
#endif

#if PUBNUB_CFG_URL_BUFFER_SIZE < 128
  #error "PUBNUB_CFG_URL_BUFFER_SIZE must be >= 128"
#endif

#ifdef PUBNUB_CFG_MAX_SUBSCRIBE_CHANNELS
  #if PUBNUB_CFG_MAX_SUBSCRIBE_CHANNELS < 1
    #error "PUBNUB_CFG_MAX_SUBSCRIBE_CHANNELS must be >= 1"
  #endif
#endif

#if PUBNUB_CFG_SUBSCRIBE_MAX_BATCH_SIZE < 1
  #error "PUBNUB_CFG_SUBSCRIBE_MAX_BATCH_SIZE must be >= 1"
#endif

#if PUBNUB_CFG_MAX_SUBSCRIBE_LISTENERS < 1
  #error "PUBNUB_CFG_MAX_SUBSCRIBE_LISTENERS must be >= 1"
#endif

#if PUBNUB_CFG_TRANSACTION_TIMEOUT_MS < 1000
  #error "PUBNUB_CFG_TRANSACTION_TIMEOUT_MS must be >= 1000"
#endif

#ifdef PUBNUB_CFG_NON_TRANSACTION_TIMEOUT_MS
  #if PUBNUB_CFG_NON_TRANSACTION_TIMEOUT_MS < 1000
    #error "PUBNUB_CFG_NON_TRANSACTION_TIMEOUT_MS must be >= 1000"
  #endif
#endif

#if PUBNUB_CFG_RETRY_DELAY_MS < 100
  #error "PUBNUB_CFG_RETRY_DELAY_MS must be >= 100"
#endif

#if PUBNUB_CFG_RETRY_MAX_DELAY_MS < PUBNUB_CFG_RETRY_DELAY_MS
  #error "PUBNUB_CFG_RETRY_MAX_DELAY_MS must be >= PUBNUB_CFG_RETRY_DELAY_MS"
#endif

#ifdef PUBNUB_CFG_ARENA_POOL_SIZE
  #if PUBNUB_CFG_ARENA_POOL_SIZE < 1024
    #error "PUBNUB_CFG_ARENA_POOL_SIZE must be >= 1024"
  #endif
#endif

#ifdef PUBNUB_CFG_MAX_LOG_MESSAGE_SIZE
  /* When 0, pubnub_log_text_formatted() is excluded from the build. */
  #if PUBNUB_CFG_MAX_LOG_MESSAGE_SIZE != 0 && PUBNUB_CFG_MAX_LOG_MESSAGE_SIZE < 64
    #error "PUBNUB_CFG_MAX_LOG_MESSAGE_SIZE must be 0 (disabled) or >= 64"
  #endif
#endif

#if PUBNUB_CFG_MAX_LOGGERS < 1
  #error "PUBNUB_CFG_MAX_LOGGERS must be >= 1"
#endif

#if PUBNUB_CFG_HTTP_MAX_PATH_SEGMENTS < 4
  #error "PUBNUB_CFG_HTTP_MAX_PATH_SEGMENTS must be >= 4"
#endif

#if PUBNUB_CFG_HTTP_MAX_QUERY_PARAMS < 4
  #error "PUBNUB_CFG_HTTP_MAX_QUERY_PARAMS must be >= 4"
#endif

#if PUBNUB_CFG_HTTP_MAX_HEADERS < 2
  #error "PUBNUB_CFG_HTTP_MAX_HEADERS must be >= 2"
#endif

#if PUBNUB_CFG_HTTP_MAX_RESP_HEADERS < 2
  #error "PUBNUB_CFG_HTTP_MAX_RESP_HEADERS must be >= 2"
#endif

#if PUBNUB_CFG_HTTP_SCRATCH_SIZE < 32
  #error "PUBNUB_CFG_HTTP_SCRATCH_SIZE must be >= 32"
#endif

#if PUBNUB_CFG_PIPELINE_MAX_MIDDLEWARES < (3 + PUBNUB_ENABLE_PAM + PUBNUB_ENABLE_RETRY + PUBNUB_ENABLE_REQUEST_COMPRESSION)
  #error "PUBNUB_CFG_PIPELINE_MAX_MIDDLEWARES is too small for the enabled middleware set"
#endif

#if PUBNUB_CFG_FILES_MAX_DOWNLOAD_SIZE < 0
  #error "PUBNUB_CFG_FILES_MAX_DOWNLOAD_SIZE must be >= 0 (0 = no limit)"
#endif

#if PUBNUB_CFG_MAX_POLL_MS < 1 || PUBNUB_CFG_MAX_POLL_MS > 5000
  #error "PUBNUB_CFG_MAX_POLL_MS must be in [1, 5000]"
#endif

#if PUBNUB_CFG_DNS_MAX_TTL_SEC < 1 || PUBNUB_CFG_DNS_MAX_TTL_SEC > 86400
  #error "PUBNUB_CFG_DNS_MAX_TTL_SEC must be between 1 and 86400 seconds"
#endif


#endif /* PUBNUB_CONFIG_H */
