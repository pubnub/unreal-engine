/* Copyright (c) PubNub Inc. */
/* See LICENSE in the root directory of this source tree. */

/**
 * @file providers/logger_types.h
 * @brief Structured log value and log entry types for the logger provider.
 *
 * All types are designed for stack allocation with borrowed pointers.
 * No heap allocation is required in the logging hot path.
 *
 * Log values use a tagged-union pattern with linked-list containers
 * for map and array types, allowing arbitrary nesting while remaining
 * fully stack-allocated.
 *
 * Log entries use an inheritance-by-embedding pattern: the base
 * pubnub_log_entry_t appears as the first member of each concrete
 * entry struct, enabling safe casting via the type discriminator.
 */

#ifndef PUBNUB_PROVIDER_LOGGER_TYPES_H
#define PUBNUB_PROVIDER_LOGGER_TYPES_H

#include "pubnub/error.h"
#include "pubnub/pubnub_compat.h"

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
// clang-format off
extern "C" {
// clang-format on
#endif

/**
 * @brief Integer compile-time constant for TRACE level.
 * Use with PUBNUB_LOG_ENABLED() in preprocessor #if guards.
 */
#define PUBNUB_LOG_LEVEL_TRACE_VALUE 0x01
/** @brief Integer compile-time constant for DEBUG level. */
#define PUBNUB_LOG_LEVEL_DEBUG_VALUE 0x02
/** @brief Integer compile-time constant for INFO level. */
#define PUBNUB_LOG_LEVEL_INFO_VALUE 0x04
/** @brief Integer compile-time constant for WARNING level. */
#define PUBNUB_LOG_LEVEL_WARNING_VALUE 0x08
/** @brief Integer compile-time constant for ERROR level. */
#define PUBNUB_LOG_LEVEL_ERROR_VALUE 0x10

/** Bitmask covering all active log levels. */
#define PUBNUB_LOG_LEVEL_ALL 0x1F

/**
 * @brief Bitmask of log levels compiled into the binary.
 *
 * Override via -DPUBNUB_CFG_LOG_LEVEL_COMPILED=<mask> at compile time.
 * Default includes all levels. Set to 0 to strip all logging code.
 *
 * Examples:
 *   0x1F  -- all levels (default)
 *   0x1C  -- INFO, WARNING, ERROR (strip TRACE+DEBUG)
 *   0x18  -- WARNING, ERROR only
 *   0x00  -- no logging compiled in
 */
#ifndef PUBNUB_CFG_LOG_LEVEL_COMPILED
#define PUBNUB_CFG_LOG_LEVEL_COMPILED PUBNUB_LOG_LEVEL_ALL
#endif

/**
 * @brief Evaluates to non-zero at preprocessing time if @p LEVEL is
 * compiled into this build (i.e. not stripped by
 * PUBNUB_CFG_LOG_LEVEL_COMPILED).
 *
 * Unlike PUBNUB_LOG_LEVEL_ENABLED() which takes an enum value,
 * this macro accepts the SHORT name (TRACE, DEBUG, INFO, WARNING, ERROR)
 * and is usable in #if directives.
 *
 * Example:
 * // clang-format off
 * @code
 * #if PUBNUB_LOG_ENABLED(DEBUG)
 *     pubnub_log_value_t map_ = PUBNUB_LOG_VALUE_NULL_INIT();
 *     map_.type = PUBNUB_LOG_VALUE_MAP;
 *     PUBNUB_LOG_MAP_SET_STRING(map_, opts->channel, channel);
 *     PUBNUB_LOG_OBJECT(logger, PUBNUB_LOG_LEVEL_DEBUG, "params", &map_);
 * #endif
 * @endcode
 * // clang-format on
 */
#define PUBNUB_LOG_ENABLED(LEVEL) \
    ((PUBNUB_LOG_LEVEL_##LEVEL##_VALUE & PUBNUB_CFG_LOG_LEVEL_COMPILED) != 0)

/**
 * @brief Log severity levels.
 *
 * Values are powers of two so they can be combined into a bitmask for
 * compile-time level stripping via PUBNUB_CFG_LOG_LEVEL_COMPILED.
 */
typedef enum pubnub_log_level {
    /** Verbose trace events. */
    PUBNUB_LOG_LEVEL_TRACE = 0x01,
    /** Debugging messages. */
    PUBNUB_LOG_LEVEL_DEBUG = 0x02,
    /** Informational messages. */
    PUBNUB_LOG_LEVEL_INFO = 0x04,
    /** Recoverable warnings. */
    PUBNUB_LOG_LEVEL_WARNING = 0x08,
    /** Error conditions. */
    PUBNUB_LOG_LEVEL_ERROR = 0x10,
    /** Disable all logging. */
    PUBNUB_LOG_LEVEL_NONE = 0x00
} pubnub_log_level_t;

PUBNUB_STATIC_ASSERT(
    PUBNUB_LOG_LEVEL_TRACE < PUBNUB_LOG_LEVEL_DEBUG
        && PUBNUB_LOG_LEVEL_DEBUG < PUBNUB_LOG_LEVEL_INFO
        && PUBNUB_LOG_LEVEL_INFO < PUBNUB_LOG_LEVEL_WARNING
        && PUBNUB_LOG_LEVEL_WARNING < PUBNUB_LOG_LEVEL_ERROR,
    "Log levels must be monotonically increasing for threshold comparison");

/** Discriminator for pubnub_log_value_t tagged union. */
typedef enum pubnub_log_value_type {
    /** Null / absent value. */
    PUBNUB_LOG_VALUE_NULL = 0,
    /** Boolean (`int` payload). */
    PUBNUB_LOG_VALUE_BOOL = 1,
    /** int64_t (no FPU required). */
    PUBNUB_LOG_VALUE_NUMBER = 2,
    /** Borrowed string view. */
    PUBNUB_LOG_VALUE_STRING = 3,
    /** Linked list of value nodes. */
    PUBNUB_LOG_VALUE_ARRAY = 4,
    /** Linked list of key/value nodes. */
    PUBNUB_LOG_VALUE_MAP = 5
} pubnub_log_value_type_t;

/**
 * @brief Structured log value (tagged union, zero-copy).
 *
 * All instances are intended to be stack-allocated. String pointers
 * are borrowed and must remain valid for the duration of the log call.
 *
 * Arrays and maps are represented as singly-linked lists of
 * pubnub_log_value_t nodes via the `next` pointer, enabling
 * stack-allocated containers without heap allocation.
 *
 * Usage pattern (all on stack):
 * @code
 * pubnub_log_value_t v_name = PUBNUB_LOG_VALUE_STR("alice");
 * pubnub_log_value_t v_age  = PUBNUB_LOG_VALUE_NUM(30);
 * pubnub_log_value_t m_age  = PUBNUB_LOG_MAP_ENTRY("age", &v_age, NULL);
 * pubnub_log_value_t m_name = PUBNUB_LOG_MAP_ENTRY("name", &v_name, &m_age);
 * // m_name is the head of a 2-entry map
 * @endcode
 */
typedef struct pubnub_log_value {
    /** Type discriminator. */
    pubnub_log_value_type_t type;

    /** Tagged union payload. */
    union {
        /** PUBNUB_LOG_VALUE_BOOL */
        int bool_val;

        /** PUBNUB_LOG_VALUE_NUMBER (integer, no FPU dependency). */
        int64_t number_val;

        /**
         * PUBNUB_LOG_VALUE_STRING (borrowed).
         * len > 0: explicit length (not necessarily NUL-terminated).
         * len == 0 with ptr != NULL: NUL-terminated; provider should use
         * strlen(). len == 0 with ptr == NULL: absent/empty string.
         */
        struct {
            const char* ptr;
            size_t      len;
        } string_val;

        /**
         * PUBNUB_LOG_VALUE_ARRAY
         * Head of a linked list of child values.
         */
        struct {
            struct pubnub_log_value* head;
        } array_val;

        /**
         * PUBNUB_LOG_VALUE_MAP
         * Each node carries a key (NUL-terminated) and a value pointer.
         * The map is a linked list of these nodes via `next`.
         */
        struct {
            const char*              key;
            struct pubnub_log_value* value;
        } map_val;
    } data;

    /** Next sibling in an array or map list (@c NULL = end of list). */
    struct pubnub_log_value* next;
} pubnub_log_value_t;

/** Initialize a null log value. */
#define PUBNUB_LOG_VALUE_NULL_INIT() {PUBNUB_LOG_VALUE_NULL, {0}, NULL}

/** Initialize a boolean log value. */
#define PUBNUB_LOG_VALUE_BOOL_INIT(b) \
    {PUBNUB_LOG_VALUE_BOOL, {.bool_val = (b)}, NULL}

/** Initialize an integer number log value (int64_t). */
#define PUBNUB_LOG_VALUE_NUM_INIT(n) \
    {PUBNUB_LOG_VALUE_NUMBER, {.number_val = (int64_t)(n)}, NULL}

/**
 * Initialize a string log value from a NUL-terminated C string.
 * The string is borrowed, not copied.
 * Sets len=0 to signal the provider should use strlen(ptr).
 */
#define PUBNUB_LOG_VALUE_STR_INIT(s) \
    {PUBNUB_LOG_VALUE_STRING, {.string_val = {(s), 0}}, NULL}

/**
 * Initialize a string log value with explicit length (non-NUL-terminated OK).
 * The string is borrowed, not copied.
 */
#define PUBNUB_LOG_VALUE_STRN_INIT(s, l) \
    {PUBNUB_LOG_VALUE_STRING, {.string_val = {(s), (l)}}, NULL}

/**
 * Initialize an array head node.
 * @param head_ptr  Pointer to the first element (pubnub_log_value_t*), or @c NULL.
 */
#define PUBNUB_LOG_VALUE_ARRAY_INIT(head_ptr) \
    {PUBNUB_LOG_VALUE_ARRAY, {.array_val = {(head_ptr)}}, NULL}

/**
 * Initialize a map entry node.
 * @param k         NUL-terminated key string (borrowed).
 * @param v         Pointer to value node (pubnub_log_value_t*).
 * @param next_ptr  Pointer to next map entry, or @c NULL.
 */
#define PUBNUB_LOG_MAP_ENTRY(k, v, next_ptr) \
    {PUBNUB_LOG_VALUE_MAP, {.map_val = {(k), (v)}}, (next_ptr)}

/**
 * @brief Append a value node to the end of an array's linked list.
 *
 * @param arr   Pointer to the array head value (type must be ARRAY).
 * @param node  Pointer to the value node to append. node->next is set to @c NULL.
 */
static inline void pubnub_log_array_append(pubnub_log_value_t* arr,
                                           pubnub_log_value_t* node)
{
    pubnub_log_value_t** tail;
    node->next = NULL;
    tail       = &arr->data.array_val.head;
    while (*tail) {
        tail = &(*tail)->next;
    }
    *tail = node;
}

/**
 * @brief Prepend a map entry node to a map's linked list.
 *
 * @param map_head  Pointer to pointer to the current map head.
 *                  Updated to point to the new entry.
 * @param entry     The map entry node to prepend.
 */
static inline void pubnub_log_map_prepend(pubnub_log_value_t** map_head,
                                          pubnub_log_value_t*  entry)
{
    entry->next = *map_head;
    *map_head   = entry;
}

/**
 * @brief Create a null log value.
 *
 * @return Stack-allocated null log value.
 */
static inline pubnub_log_value_t pubnub_log_value_null(void)
{
    pubnub_log_value_t v = (pubnub_log_value_t)PUBNUB_LOG_VALUE_NULL_INIT();
    return v;
}

/**
 * @brief Create a boolean log value.
 *
 * @param b  Boolean value (0 = false, non-zero = true).
 * @return Stack-allocated boolean log value.
 */
static inline pubnub_log_value_t pubnub_log_value_bool(int b)
{
    pubnub_log_value_t v = (pubnub_log_value_t)PUBNUB_LOG_VALUE_BOOL_INIT(b);
    return v;
}

/**
 * @brief Create an integer log value.
 *
 * Uses int64_t to avoid FPU dependency. Cast wider integers explicitly.
 *
 * @param n  Integer value.
 * @return Stack-allocated number log value.
 */
static inline pubnub_log_value_t pubnub_log_value_number(int64_t n)
{
    pubnub_log_value_t v = (pubnub_log_value_t)PUBNUB_LOG_VALUE_NUM_INIT(n);
    return v;
}

/**
 * @brief Create a string log value from a NUL-terminated string.
 *
 * The string is borrowed — it must remain valid for the log call's duration.
 *
 * @param s  NUL-terminated string to borrow.
 * @return Stack-allocated string log value.
 */
static inline pubnub_log_value_t pubnub_log_value_string(const char* s)
{
    pubnub_log_value_t v = (pubnub_log_value_t)PUBNUB_LOG_VALUE_STR_INIT(s);
    return v;
}

/**
 * @brief Create a string log value with explicit length.
 *
 * The string is borrowed and need not be NUL-terminated.
 *
 * @param s    String pointer to borrow.
 * @param len  Byte length of the string.
 * @return Stack-allocated string log value.
 */
static inline pubnub_log_value_t pubnub_log_value_string_n(const char* s, size_t len)
{
    pubnub_log_value_t v = (pubnub_log_value_t)PUBNUB_LOG_VALUE_STRN_INIT(s, len);
    return v;
}

/**
 * @brief Create an empty array container value.
 *
 * @return Stack-allocated array log value with no elements.
 */
static inline pubnub_log_value_t pubnub_log_value_array_init(void)
{
    pubnub_log_value_t v = (pubnub_log_value_t)PUBNUB_LOG_VALUE_ARRAY_INIT(NULL);
    return v;
}

/**
 * @brief Initialize a map head pointer to empty (NULL).
 *
 * Maps are linked lists of entry nodes. The head starts NULL.
 * Use pubnub_log_value_map_set_entry() or PUBNUB_LOG_MAP_SET_* to add entries.
 *
 * Expands to a typed NULL pointer cast for use as a map head initializer.
 */
/* Intentional lowercase: mirrors the function-call style of the other
 * factory functions. Suppressed via NOLINTNEXTLINE below. */
// NOLINTNEXTLINE(readability-identifier-naming)
#define pubnub_log_value_map_init() ((pubnub_log_value_t*)NULL)

/**
 * @brief Add a pre-initialized entry node to a map's linked list.
 *
 * The entry node and its value must remain valid (stack-alive) for the
 * duration of any log call that uses this map.
 *
 * @param map_head  Pointer to the map head pointer (updated to new head).
 * @param entry     Map entry node to prepend (PUBNUB_LOG_VALUE_MAP type).
 */
static inline void pubnub_log_value_map_set_entry(pubnub_log_value_t** map_head,
                                                  pubnub_log_value_t*  entry)
{
    pubnub_log_map_prepend(map_head, entry);
}

/**
 * @brief Append a pre-initialized value node to an array's linked list.
 *
 * The node must remain valid (stack-alive) for the duration of any log
 * call that uses this array.
 *
 * @param arr   Pointer to the array container value (type ARRAY).
 * @param node  Value node to append.
 */
static inline void pubnub_log_value_array_append_node(pubnub_log_value_t* arr,
                                                      pubnub_log_value_t* node)
{
    pubnub_log_array_append(arr, node);
}

/**
 * @brief Return the type discriminator of a log value.
 *
 * @param v  Log value to inspect. NULL returns PUBNUB_LOG_VALUE_NULL.
 * @return Type enum value, or PUBNUB_LOG_VALUE_NULL when @p v is NULL.
 */
static inline pubnub_log_value_type_t pubnub_log_value_type(const pubnub_log_value_t* v)
{
    if (NULL == v) {
        return PUBNUB_LOG_VALUE_NULL;
    }
    return v->type;
}

/**
 * @brief Extract a boolean from a log value.
 *
 * @param v  Log value with type PUBNUB_LOG_VALUE_BOOL.
 * @return Boolean as int (0 or non-zero). Returns 0 on type mismatch or NULL.
 */
static inline int pubnub_log_value_get_bool(const pubnub_log_value_t* v)
{
    if (NULL == v || PUBNUB_LOG_VALUE_BOOL != v->type) {
        return 0;
    }
    return v->data.bool_val;
}

/**
 * @brief Extract a number from a log value.
 *
 * @param v  Log value with type PUBNUB_LOG_VALUE_NUMBER.
 * @return int64_t number. Returns 0 on type mismatch or NULL.
 */
static inline int64_t pubnub_log_value_get_number(const pubnub_log_value_t* v)
{
    if (NULL == v || PUBNUB_LOG_VALUE_NUMBER != v->type) {
        return 0;
    }
    return v->data.number_val;
}

/**
 * @brief Extract a string pointer from a log value.
 *
 * @param v        Log value with type PUBNUB_LOG_VALUE_STRING.
 * @param out_len  Receives the explicit length (0 = NUL-terminated, use
 *                 strlen). May be NULL if the length is not needed.
 * @return Borrowed string pointer, or NULL on type mismatch or NULL input.
 */
static inline const char* pubnub_log_value_get_string(const pubnub_log_value_t* v,
                                                      size_t* out_len)
{
    if (NULL == v || PUBNUB_LOG_VALUE_STRING != v->type) {
        if (NULL != out_len) {
            *out_len = 0;
        }
        return NULL;
    }
    if (NULL != out_len) {
        *out_len = v->data.string_val.len;
    }
    return v->data.string_val.ptr;
}

/**
 * @brief Return the first element of an array container value.
 *
 * @param container  Array value (PUBNUB_LOG_VALUE_ARRAY type).
 * @return First element node, or NULL if empty or wrong type.
 */
static inline const pubnub_log_value_t*
pubnub_log_value_first(const pubnub_log_value_t* container)
{
    if (NULL == container || PUBNUB_LOG_VALUE_ARRAY != container->type) {
        return NULL;
    }
    return container->data.array_val.head;
}

/**
 * @brief Return the key of a map entry node.
 *
 * @param entry  A map entry node (PUBNUB_LOG_VALUE_MAP type).
 * @return NUL-terminated key string, or NULL if not a map entry or NULL.
 */
static inline const char* pubnub_log_value_key(const pubnub_log_value_t* entry)
{
    if (NULL == entry || PUBNUB_LOG_VALUE_MAP != entry->type) {
        return NULL;
    }
    return entry->data.map_val.key;
}

/**
 * @brief Return the next sibling in an array element or map entry list.
 *
 * @param entry  Any log value node.
 * @return Next node, or NULL at end of list or on NULL input.
 */
static inline const pubnub_log_value_t*
pubnub_log_value_next(const pubnub_log_value_t* entry)
{
    if (NULL == entry) {
        return NULL;
    }
    return entry->next;
}

/** Discriminator for pubnub_log_entry_t subtypes. */
typedef enum pubnub_log_entry_type {
    /** Plain text message (pubnub_log_entry_text_t). */
    PUBNUB_LOG_ENTRY_TEXT = 0,
    /** Structured data (pubnub_log_entry_object_t). */
    PUBNUB_LOG_ENTRY_OBJECT = 1,
    /** Error with code and details (pubnub_log_entry_error_t). */
    PUBNUB_LOG_ENTRY_ERROR = 2,
    /** Outgoing HTTP request (pubnub_log_entry_net_request_t). */
    PUBNUB_LOG_ENTRY_NET_REQ = 3,
    /** Incoming HTTP response (pubnub_log_entry_net_response_t). */
    PUBNUB_LOG_ENTRY_NET_RESP = 4
} pubnub_log_entry_type_t;

/**
 * @brief Base log entry header.
 *
 * This struct appears as the first member of every concrete log entry
 * type, enabling safe down-casting via the `type` discriminator.
 *
 * The mux logger stamps context_id and timestamp_ms before dispatch;
 * direct provider usage leaves them zero/NULL.
 */
typedef struct pubnub_log_entry {
    /** Entry subtype discriminator. */
    pubnub_log_entry_type_t type;
    /** Severity level. */
    pubnub_log_level_t level;
    /** Source file name (borrowed, may be @c NULL). */
    const char* file;
    /** Source line number. */
    int line;
    /**
     * @brief Identifier of the context that emitted this entry.
     *
     * 8-hex FNV-1a hash of the context UUID, stamped by the mux before
     * dispatch. NULL when emitted outside a mux (e.g. direct provider use).
     */
    const char* context_id;
    /**
     * @brief Unix epoch timestamp in milliseconds.
     *
     * Stamped by the mux via platform->monotonic_ms(). Zero when no
     * platform provider is available.
     */
    uint64_t timestamp_ms;
    /**
     * @brief Minimum log level threshold active when this entry was emitted.
     *
     * Used by the stdout logger to decide whether to show full detail
     * (headers, body) for network entries.
     */
    pubnub_log_level_t minimum_level;
} pubnub_log_entry_t;

/**
 * @brief Plain text log entry.
 *
 * For simple printf-style messages. The `message` field is a
 * pre-formatted NUL-terminated string (borrowed pointer).
 */
typedef struct pubnub_log_entry_text {
    /** Base entry (must be first member). */
    pubnub_log_entry_t base;
    /** Pre-formatted text message (borrowed, NUL-terminated). */
    const char* message;
} pubnub_log_entry_text_t;

/**
 * @brief Structured data log entry.
 *
 * Carries a human-readable message label and a structured data
 * payload as a pubnub_log_value_t tree.
 */
typedef struct pubnub_log_entry_object {
    /** Base entry (must be first member). */
    pubnub_log_entry_t base;
    /** Human-readable label (borrowed, NUL-terminated, may be @c NULL). */
    const char* label;
    /** Structured data payload (borrowed, stack-allocated tree). */
    const pubnub_log_value_t* data;
} pubnub_log_entry_object_t;

/**
 * @brief Error log entry.
 *
 * Carries an error code, human-readable message, and optional
 * structured details (e.g., context, parameters that caused the error).
 */
typedef struct pubnub_log_entry_error {
    /** Base entry (must be first member). */
    pubnub_log_entry_t base;
    /** SDK error code. */
    int error_code;
    /** Human-readable error message (borrowed, NUL-terminated). */
    const char* error_message;
    /** Optional structured details (borrowed, may be @c NULL). */
    const pubnub_log_value_t* details;
} pubnub_log_entry_error_t;

/**
 * @brief Network request log entry.
 *
 * Logged when the SDK sends an HTTP request. All string fields are
 * borrowed pointers valid for the duration of the log call.
 */
typedef struct pubnub_log_entry_net_request {
    /** Base entry (must be first member). */
    pubnub_log_entry_t base;
    /** HTTP method string (e.g., "GET", "POST"; borrowed). */
    const char* method;
    /** Request URL (borrowed, NUL-terminated). */
    const char* url;
    /** Request headers as a map value (borrowed, may be @c NULL). */
    const pubnub_log_value_t* headers;
    /** Request body (borrowed, may be @c NULL). */
    const uint8_t* body;
    /** Request body length in bytes. */
    size_t body_len;
    /** Non-zero if the request was cancelled before completion. */
    int canceled;
    /** Non-zero if the request failed at the transport level. */
    int failed;
    /** SDK-level result code when failed (PUBNUB_OK when not failed). */
    pubnub_res_t result;
    /** Pool slot index that owns this request (for queue diagnostics). */
    uint16_t slot_id;
} pubnub_log_entry_net_request_t;

/**
 * @brief Network response log entry.
 *
 * Logged when the SDK receives an HTTP response.
 */
typedef struct pubnub_log_entry_net_response {
    /** Base entry (must be first member). */
    pubnub_log_entry_t base;
    /** Request URL that produced this response (borrowed). */
    const char* url;
    /** HTTP status code (e.g. 200, 403, 500). */
    int status_code;
    /** Response headers as a map value (borrowed, may be @c NULL). */
    const pubnub_log_value_t* headers;
    /** Response body (borrowed, may be @c NULL). */
    const uint8_t* body;
    /** Response body length in bytes. */
    size_t body_len;
} pubnub_log_entry_net_response_t;

/* Overload dispatch: select 2-arg or 3-arg variant by argument count. */
#define PUBNUB_LOG_IMPL_PICK(A1, A2, A3, N, ...) N
#define PUBNUB_LOG_IMPL_NARG2(...)               PUBNUB_LOG_IMPL_PICK(__VA_ARGS__, 3, 2, 1)
#define PUBNUB_LOG_IMPL_CAT2(A, B)               A##B
#define PUBNUB_LOG_IMPL_CAT(A, B)                PUBNUB_LOG_IMPL_CAT2(A, B)
#define PUBNUB_LOG_IMPL_DISPATCH(base, ...) \
    PUBNUB_LOG_IMPL_CAT(base, PUBNUB_LOG_IMPL_NARG2(__VA_ARGS__))(__VA_ARGS__)

#define PUBNUB_LOG_IMPL_MAP_SET_STRING2(map, name)                               \
    pubnub_log_value_t name##_val_;                                              \
    pubnub_log_value_t name##_entry_;                                            \
    if ((name) && (name)[0] != '\0') {                                           \
        name##_val_ = (pubnub_log_value_t)PUBNUB_LOG_VALUE_STR_INIT(name);       \
        name##_entry_ =                                                          \
            (pubnub_log_value_t)PUBNUB_LOG_MAP_ENTRY(#name, &name##_val_, NULL); \
        pubnub_log_map_prepend(&(map), &name##_entry_);                          \
    }

#define PUBNUB_LOG_IMPL_MAP_SET_STRING3(map, value, key)                       \
    pubnub_log_value_t key##_val_;                                             \
    pubnub_log_value_t key##_entry_;                                           \
    if ((value) && (value)[0] != '\0') {                                       \
        key##_val_ = (pubnub_log_value_t)PUBNUB_LOG_VALUE_STR_INIT(value);     \
        key##_entry_ =                                                         \
            (pubnub_log_value_t)PUBNUB_LOG_MAP_ENTRY(#key, &key##_val_, NULL); \
        pubnub_log_map_prepend(&(map), &key##_entry_);                         \
    }

/** @brief Add a string entry to a map value (2-arg or 3-arg). */
#define PUBNUB_LOG_MAP_SET_STRING(...) \
    PUBNUB_LOG_IMPL_DISPATCH(PUBNUB_LOG_IMPL_MAP_SET_STRING, __VA_ARGS__)

#define PUBNUB_LOG_IMPL_MAP_SET_NUMBER2(map, name)                           \
    pubnub_log_value_t name##_val_ =                                         \
        (pubnub_log_value_t)PUBNUB_LOG_VALUE_NUM_INIT(name);                 \
    pubnub_log_value_t name##_entry_ =                                       \
        (pubnub_log_value_t)PUBNUB_LOG_MAP_ENTRY(#name, &name##_val_, NULL); \
    pubnub_log_map_prepend(&(map), &name##_entry_);

#define PUBNUB_LOG_IMPL_MAP_SET_NUMBER3(map, value, key)                   \
    pubnub_log_value_t key##_val_ =                                        \
        (pubnub_log_value_t)PUBNUB_LOG_VALUE_NUM_INIT(value);              \
    pubnub_log_value_t key##_entry_ =                                      \
        (pubnub_log_value_t)PUBNUB_LOG_MAP_ENTRY(#key, &key##_val_, NULL); \
    pubnub_log_map_prepend(&(map), &key##_entry_);

/** @brief Add a numeric (int64_t) entry to a map value (2-arg or 3-arg). */
#define PUBNUB_LOG_MAP_SET_NUMBER(...) \
    PUBNUB_LOG_IMPL_DISPATCH(PUBNUB_LOG_IMPL_MAP_SET_NUMBER, __VA_ARGS__)

#define PUBNUB_LOG_IMPL_MAP_SET_BOOL2(map, name)                             \
    pubnub_log_value_t name##_val_ =                                         \
        (pubnub_log_value_t)PUBNUB_LOG_VALUE_BOOL_INIT(name);                \
    pubnub_log_value_t name##_entry_ =                                       \
        (pubnub_log_value_t)PUBNUB_LOG_MAP_ENTRY(#name, &name##_val_, NULL); \
    pubnub_log_map_prepend(&(map), &name##_entry_);

#define PUBNUB_LOG_IMPL_MAP_SET_BOOL3(map, value, key)                     \
    pubnub_log_value_t key##_val_ =                                        \
        (pubnub_log_value_t)PUBNUB_LOG_VALUE_BOOL_INIT(value);             \
    pubnub_log_value_t key##_entry_ =                                      \
        (pubnub_log_value_t)PUBNUB_LOG_MAP_ENTRY(#key, &key##_val_, NULL); \
    pubnub_log_map_prepend(&(map), &key##_entry_);

/** @brief Add a boolean entry to a map value (2-arg or 3-arg). */
#define PUBNUB_LOG_MAP_SET_BOOL(...) \
    PUBNUB_LOG_IMPL_DISPATCH(PUBNUB_LOG_IMPL_MAP_SET_BOOL, __VA_ARGS__)

#define PUBNUB_LOG_IMPL_ARRAY_APPEND_STRING2(arr, name)                    \
    pubnub_log_value_t name##_elm_;                                        \
    if ((name) && (name)[0] != '\0') {                                     \
        name##_elm_ = (pubnub_log_value_t)PUBNUB_LOG_VALUE_STR_INIT(name); \
        pubnub_log_array_append((arr), &name##_elm_);                      \
    }

#define PUBNUB_LOG_IMPL_ARRAY_APPEND_STRING3(arr, value, name)              \
    pubnub_log_value_t name##_elm_;                                         \
    if ((value) && (value)[0] != '\0') {                                    \
        name##_elm_ = (pubnub_log_value_t)PUBNUB_LOG_VALUE_STR_INIT(value); \
        pubnub_log_array_append((arr), &name##_elm_);                       \
    }

/** @brief Append a string element to an array value (2-arg or 3-arg). */
#define PUBNUB_LOG_ARRAY_APPEND_STRING(...) \
    PUBNUB_LOG_IMPL_DISPATCH(PUBNUB_LOG_IMPL_ARRAY_APPEND_STRING, __VA_ARGS__)

#define PUBNUB_LOG_IMPL_ARRAY_APPEND_NUMBER2(arr, name)      \
    pubnub_log_value_t name##_elm_ =                         \
        (pubnub_log_value_t)PUBNUB_LOG_VALUE_NUM_INIT(name); \
    pubnub_log_array_append((arr), &name##_elm_);

#define PUBNUB_LOG_IMPL_ARRAY_APPEND_NUMBER3(arr, value, name) \
    pubnub_log_value_t name##_elm_ =                           \
        (pubnub_log_value_t)PUBNUB_LOG_VALUE_NUM_INIT(value);  \
    pubnub_log_array_append((arr), &name##_elm_);

/** @brief Append a numeric element to an array value (2-arg or 3-arg). */
#define PUBNUB_LOG_ARRAY_APPEND_NUMBER(...) \
    PUBNUB_LOG_IMPL_DISPATCH(PUBNUB_LOG_IMPL_ARRAY_APPEND_NUMBER, __VA_ARGS__)

#define PUBNUB_LOG_IMPL_ARRAY_APPEND_BOOL2(arr, name)         \
    pubnub_log_value_t name##_elm_ =                          \
        (pubnub_log_value_t)PUBNUB_LOG_VALUE_BOOL_INIT(name); \
    pubnub_log_array_append((arr), &name##_elm_);

#define PUBNUB_LOG_IMPL_ARRAY_APPEND_BOOL3(arr, value, name)   \
    pubnub_log_value_t name##_elm_ =                           \
        (pubnub_log_value_t)PUBNUB_LOG_VALUE_BOOL_INIT(value); \
    pubnub_log_array_append((arr), &name##_elm_);

/** @brief Append a boolean element to an array value (2-arg or 3-arg). */
#define PUBNUB_LOG_ARRAY_APPEND_BOOL(...) \
    PUBNUB_LOG_IMPL_DISPATCH(PUBNUB_LOG_IMPL_ARRAY_APPEND_BOOL, __VA_ARGS__)

#ifdef __cplusplus
// clang-format off
}
// clang-format on
#endif

#endif /* PUBNUB_PROVIDER_LOGGER_TYPES_H */
