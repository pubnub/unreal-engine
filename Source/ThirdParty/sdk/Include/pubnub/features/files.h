/* Copyright (c) PubNub Inc. */
/* See LICENSE in the root directory of this source tree. */

#ifndef PUBNUB_FEATURE_FILES_H
#define PUBNUB_FEATURE_FILES_H

#include "pubnub/config.h"

#if PUBNUB_ENABLE_FILES

#include "pubnub/error.h"
#include "pubnub/future.h"
#include "pubnub/types.h"

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
// clang-format off
extern "C" {
// clang-format on
#endif

/**
 * @brief Options for @c pubnub_send_file.
 *
 * Initialize with @c PUBNUB_SEND_FILE_OPTS_INIT before overriding
 * fields. The composed operation generates an upload URL, uploads to
 * S3, then publishes a file message to the channel.
 */
typedef struct pubnub_send_file_opts {
    /**
     * @brief Target channel name (@b required, @b borrowed,
     *        NUL-terminated).
     */
    const char* channel;

    /**
     * @brief Desired file name (@b required, @b borrowed,
     *        NUL-terminated).
     *
     * The server may assign a different name; the actual name is
     * available in @c pubnub_send_file_result_t::name.
     */
    const char* file_name;

    /**
     * @brief File content buffer (@b required, @b borrowed).
     *
     * Must remain valid for the lifetime of the send operation
     * (until @c pubnub_future_is_ready returns non-zero).
     *
     * @pre Non-NULL when @c data_len > 0.
     */
    const uint8_t* data;

    /**
     * @brief File content length in bytes (@b required).
     *
     * The server enforces a maximum file size (typically 5 MB).
     * Exceeding it results in @c PUBNUB_ERR_INVALID_ARGUMENT from the
     * S3 upload step (EntityTooLarge).
     */
    size_t data_len;

    /**
     * @brief MIME type of the file content (@b optional, @b borrowed,
     *        NUL-terminated).
     *
     * Pass @c NULL to default to @c "application/octet-stream".
     */
    const char* content_type;

    /**
     * @brief JSON string attached to the file message (@b optional,
     *        @b borrowed, NUL-terminated).
     *
     * Published alongside the file descriptor in the channel.
     * Must be valid JSON if non-NULL.
     */
    const char* message;

    /**
     * @brief Stream-filter metadata JSON (@b optional, @b borrowed,
     *        NUL-terminated).
     *
     * Attached to the publish step of the file message.
     */
    const char* meta;

    /**
     * @brief User-supplied message-type label (@b optional,
     *        @b borrowed, NUL-terminated).
     *
     * @pre 3-50 characters long.
     */
    const char* custom_message_type;

    /**
     * @brief Message persistence.
     *
     * @b Default: @c 1 (store). Set to @c 0 to skip persistence.
     */
    int store;

    /**
     * @brief Per-message time-to-live in minutes.
     *
     * @b Default: @c 0 (account default TTL).
     */
    unsigned int ttl;

    /**
     * @brief S3 upload timeout in milliseconds.
     *
     * @b Default: @c 0 (use SDK default of 300000 ms / 5 minutes).
     */
    uint32_t upload_timeout_ms;

    /**
     * @brief Per-step PubNub API timeout override in milliseconds
     *        (@b optional).
     *
     * Applies to the generate-upload-url and publish-file-message
     * steps.
     *
     * @note When non-zero, takes priority over the context-level
     *       @c pubnub_config_t::transaction_timeout_ms.
     */
    uint32_t timeout_ms;

    /**
     * @brief Filesystem path to load file content from (@b optional,
     *        @b borrowed, NUL-terminated).
     *
     * When non-NULL, the SDK loads the file content via the platform
     * provider's @c file_load method, populates @c data / @c data_len
     * from the result, and derives @c file_name from the path basename
     * if @c file_name is @c NULL. Returns @c PUBNUB_ERR_NOT_SUPPORTED if
     * the platform provider does not implement @c file_load.
     *
     * @b Default: @c NULL (use @c data / @c data_len directly).
     */
    const char* file_path;
} pubnub_send_file_opts_t;

/**
 * @brief Default initializer for @c pubnub_send_file_opts_t.
 *
 * Sets @c store to 1 (the REST API default is to store).
 * All other fields zero-initialize to documented defaults.
 *
 * Usage
 * @code
 * pubnub_send_file_opts_t opts = PUBNUB_SEND_FILE_OPTS_INIT;
 * opts.channel   = "my-channel";
 * opts.file_name = "report.pdf";
 * opts.data      = file_buf;
 * opts.data_len  = file_len;
 * @endcode
 *
 * @see pubnub_send_file
 */
#define PUBNUB_SEND_FILE_OPTS_INIT \
    {.channel             = NULL,  \
     .file_name           = NULL,  \
     .data                = NULL,  \
     .data_len            = 0,     \
     .content_type        = NULL,  \
     .message             = NULL,  \
     .meta                = NULL,  \
     .custom_message_type = NULL,  \
     .store               = 1,     \
     .ttl                 = 0,     \
     .upload_timeout_ms   = 0,     \
     .timeout_ms          = 0,     \
     .file_path           = NULL}

/**
 * @brief Options for @c pubnub_list_files.
 *
 * Initialize with @c PUBNUB_LIST_FILES_OPTS_INIT before overriding
 * fields.
 */
typedef struct pubnub_list_files_opts {
    /**
     * @brief Target channel name (@b required, @b borrowed,
     *        NUL-terminated).
     */
    const char* channel;

    /**
     * @brief Maximum files per page (1-100).
     *
     * @b Default: @c 0 (server default of 100).
     */
    int limit;

    /**
     * @brief Pagination token from a previous response (@b optional,
     *        @b borrowed, NUL-terminated).
     *
     * Pass @c NULL to fetch the first page.
     */
    const char* next;

    /**
     * @brief Per-request timeout override in milliseconds (@b optional).
     *
     * @note When non-zero, takes priority over the context-level
     *       @c pubnub_config_t::transaction_timeout_ms.
     */
    uint32_t timeout_ms;
} pubnub_list_files_opts_t;

/**
 * @brief Default initializer for @c pubnub_list_files_opts_t.
 *
 * All defaults are zero/@c NULL.
 */
#define PUBNUB_LIST_FILES_OPTS_INIT {0}

/**
 * @brief Options for @c pubnub_delete_file.
 *
 * Initialize with @c PUBNUB_DELETE_FILE_OPTS_INIT before overriding
 * fields.
 */
typedef struct pubnub_delete_file_opts {
    /**
     * @brief Target channel name (@b required, @b borrowed,
     *        NUL-terminated).
     */
    const char* channel;

    /**
     * @brief File identifier (@b required, @b borrowed,
     *        NUL-terminated).
     *
     * Obtained from @c pubnub_send_file_result_t::id or
     * @c pubnub_file_info_t::id.
     */
    const char* file_id;

    /**
     * @brief File name (@b required, @b borrowed, NUL-terminated).
     *
     * Obtained from @c pubnub_send_file_result_t::name or
     * @c pubnub_file_info_t::name.
     */
    const char* file_name;

    /**
     * @brief Per-request timeout override in milliseconds (@b optional).
     *
     * @note When non-zero, takes priority over the context-level
     *       @c pubnub_config_t::transaction_timeout_ms.
     */
    uint32_t timeout_ms;
} pubnub_delete_file_opts_t;

/**
 * @brief Default initializer for @c pubnub_delete_file_opts_t.
 *
 * All defaults are zero/@c NULL.
 */
#define PUBNUB_DELETE_FILE_OPTS_INIT {0}

/**
 * @brief Options for @c pubnub_download_file.
 *
 * Initialize with @c PUBNUB_DOWNLOAD_FILE_OPTS_INIT before
 * overriding fields.
 */
typedef struct pubnub_download_file_opts {
    /**
     * @brief Target channel name (@b required, @b borrowed,
     *        NUL-terminated).
     */
    const char* channel;

    /**
     * @brief File identifier (@b required, @b borrowed,
     *        NUL-terminated).
     */
    const char* file_id;

    /**
     * @brief File name (@b required, @b borrowed, NUL-terminated).
     */
    const char* file_name;

    /**
     * @brief Per-request timeout override in milliseconds (@b optional).
     *
     * @note When non-zero, takes priority over the context-level
     *       @c pubnub_config_t::transaction_timeout_ms.
     */
    uint32_t timeout_ms;
} pubnub_download_file_opts_t;

/**
 * @brief Default initializer for @c pubnub_download_file_opts_t.
 *
 * All defaults are zero/@c NULL.
 */
#define PUBNUB_DOWNLOAD_FILE_OPTS_INIT {0}

/**
 * @brief Options for @c pubnub_publish_file_message.
 *
 * Use this to manually (re-)publish a file message after
 * @c pubnub_send_file completed the upload but failed the publish
 * step. Obtain @c file_id and @c file_name from
 * @c pubnub_send_file_result_t.
 *
 * Initialize with @c PUBNUB_PUBLISH_FILE_MESSAGE_OPTS_INIT before
 * overriding fields.
 */
typedef struct pubnub_publish_file_message_opts {
    /**
     * @brief Target channel name (@b required, @b borrowed,
     *        NUL-terminated).
     */
    const char* channel;

    /**
     * @brief File identifier from a prior upload (@b required,
     *        @b borrowed, NUL-terminated).
     */
    const char* file_id;

    /**
     * @brief File name from a prior upload (@b required, @b borrowed,
     *        NUL-terminated).
     */
    const char* file_name;

    /**
     * @brief JSON string attached to the file message (@b optional,
     *        @b borrowed, NUL-terminated).
     */
    const char* message;

    /**
     * @brief Stream-filter metadata JSON (@b optional, @b borrowed,
     *        NUL-terminated).
     */
    const char* meta;

    /**
     * @brief User-supplied message-type label (@b optional,
     *        @b borrowed, NUL-terminated).
     *
     * @pre 3-50 characters long.
     */
    const char* custom_message_type;

    /**
     * @brief Store the file message in Message Persistence.
     *
     * @b Default: @c 1 (store). Set to @c 0 to skip persistence.
     */
    int store;

    /**
     * @brief Per-message time-to-live in minutes.
     *
     * @b Default: @c 0 (account default TTL).
     */
    unsigned int ttl;

    /**
     * @brief Per-request timeout override in milliseconds (@b optional).
     *
     * @note When non-zero, takes priority over the context-level
     *       @c pubnub_config_t::transaction_timeout_ms.
     */
    uint32_t timeout_ms;
} pubnub_publish_file_message_opts_t;

/**
 * @brief Default initializer for
 *        @c pubnub_publish_file_message_opts_t.
 *
 * Sets @c store to 1 (the REST API default is to store).
 */
#define PUBNUB_PUBLISH_FILE_MESSAGE_OPTS_INIT \
    {.channel             = NULL,             \
     .file_id             = NULL,             \
     .file_name           = NULL,             \
     .message             = NULL,             \
     .meta                = NULL,             \
     .custom_message_type = NULL,             \
     .store               = 1,                \
     .ttl                 = 0,                \
     .timeout_ms          = 0}

/**
 * @brief Options for @c pubnub_get_file_url.
 *
 * Initialize with @c PUBNUB_GET_FILE_URL_OPTS_INIT before overriding
 * fields.
 */
typedef struct pubnub_get_file_url_opts {
    /**
     * @brief Target channel name (@b required, @b borrowed,
     *        NUL-terminated).
     */
    const char* channel;

    /**
     * @brief File identifier (@b required, @b borrowed,
     *        NUL-terminated).
     */
    const char* file_id;

    /**
     * @brief File name (@b required, @b borrowed, NUL-terminated).
     */
    const char* file_name;
} pubnub_get_file_url_opts_t;

/**
 * @brief Default initializer for @c pubnub_get_file_url_opts_t.
 *
 * All defaults are zero/@c NULL.
 */
#define PUBNUB_GET_FILE_URL_OPTS_INIT {0}

/**
 * @brief Result of a completed @c pubnub_send_file operation.
 *
 * Fields @c id and @c name are populated for both complete success
 * and the partial-success case (upload OK, publish failed). The
 * @c timetoken is only valid on complete success.
 *
 * All views alias internal data and remain valid until
 * @c pubnub_future_release is called on the originating future.
 *
 * @note Check @c pubnub_future_status first:
 *       - @c PUBNUB_OK: all three steps completed; @c id, @c name,
 *         and @c timetoken are all valid.
 *       - Error with non-empty @c id / @c name: S3 upload succeeded
 *         but publishing the file message failed. The file exists
 *         under @c id / @c name and can be published manually via
 *         @c pubnub_publish_file_message.
 *       - Error with empty @c id: failure occurred before or during
 *         upload; nothing to recover.
 */
typedef struct pubnub_send_file_result {
    /** Server-assigned file identifier. */
    pubnub_string_view_t id;
    /** Final file name (may differ from the requested name). */
    pubnub_string_view_t name;
    /** Publish timetoken (valid only on complete success). */
    pubnub_timetoken_t timetoken;
} pubnub_send_file_result_t;

/**
 * @brief Result of a completed @c pubnub_list_files operation.
 *
 * Use @c count as the loop bound for
 * @c pubnub_list_files_result_file_at.
 */
typedef struct pubnub_list_files_result {
    /** Number of files in this page. */
    uint32_t count;
    /**
     * @brief Pagination token for the next page.
     *
     * Empty view (@c .len == 0) indicates the last page.
     */
    pubnub_string_view_t next;
} pubnub_list_files_result_t;

/**
 * @brief Metadata for a single file returned by
 *        @c pubnub_list_files.
 *
 * All views alias internal data and remain valid until
 * @c pubnub_future_release is called on the originating future.
 */
typedef struct pubnub_file_info {
    /** File identifier. */
    pubnub_string_view_t id;
    /** File name. */
    pubnub_string_view_t name;
    /** File size in bytes. */
    uint32_t size;
    /** ISO 8601 creation timestamp. */
    pubnub_string_view_t created;
} pubnub_file_info_t;

/**
 * @brief Result of a completed @c pubnub_download_file operation.
 *
 * The data pointer aliases internal buffers and remains valid until
 * @c pubnub_future_release is called on the originating future.
 */
typedef struct pubnub_download_file_result {
    /** File content (borrowed, valid until future release). */
    const uint8_t* data;
    /** Content length in bytes. */
    size_t data_len;
    /**
     * @brief Decryption status indicator.
     *
     * Set to 1 when the content was successfully decrypted via the
     * configured crypto module. 0 when the content is raw (no crypto
     * configured, decryption failed or not attempted).
     */
    uint8_t decrypted;
} pubnub_download_file_result_t;

/**
 * @brief Result of a completed @c pubnub_publish_file_message
 *        operation.
 */
typedef struct pubnub_publish_file_message_result {
    /** Publish timetoken of the file message. */
    pubnub_timetoken_t timetoken;
} pubnub_publish_file_message_result_t;

/**
 * @brief Send a file to a PubNub channel.
 *
 * This is a composed multi-step operation that:
 * 1. Requests a presigned upload URL from PubNub.
 * 2. Uploads the file directly to cloud storage (S3).
 * 3. Publishes a file message to notify channel subscribers.
 *
 * The returned future completes only after all steps succeed or a
 * non-recoverable failure occurs. On success, read the result via
 * @c pubnub_send_file_result.
 *
 * The @c opts.data buffer must remain valid for the duration of the
 * operation (until @c pubnub_future_is_ready returns non-zero).
 *
 * Example (cooperative polling)
 * @code
 * pubnub_send_file_opts_t opts = PUBNUB_SEND_FILE_OPTS_INIT;
 * opts.channel      = "my-channel";
 * opts.file_name    = "photo.jpg";
 * opts.data         = jpeg_buf;
 * opts.data_len     = jpeg_len;
 * opts.content_type = "image/jpeg";
 *
 * pubnub_future_t fut = pubnub_send_file(ctx, &opts);
 * while (!pubnub_future_is_ready(fut)) {
 *     pubnub_process(ctx);
 * }
 *
 * pubnub_send_file_result_t r = pubnub_send_file_result(fut);
 * if (PUBNUB_OK == pubnub_future_status(fut)) {
 *     printf("uploaded: id=%.*s name=%.*s\n",
 *            (int)r.id.len, r.id.ptr,
 *            (int)r.name.len, r.name.ptr);
 * } else if (NULL != r.id.ptr) {
 *     // File is on S3 but publish failed -- recover with
 *     // pubnub_publish_file_message using r.id and r.name.
 * }
 * pubnub_future_release(fut);
 * @endcode
 *
 * @note All sub-steps are driven by @c pubnub_process(ctx). On
 *       cooperative targets, continue calling pubnub_process in your
 *       event loop. Internally uses one request pool slot (steps
 *       execute sequentially within the slot).
 *
 * @note Required context configuration: @c subscribe_key and
 *       @c publish_key must be set in @c pubnub_config_t.
 *
 * @note When the S3 upload succeeds but the publish step fails,
 *       the future reports an error but @c pubnub_send_file_result
 *       still provides @c id and @c name. Use those values with
 *       @c pubnub_publish_file_message to complete delivery.
 *
 * @param ctx  Initialized context (@b borrowed).
 * @param opts Send-file options (@b required, @b borrowed).
 * @return Future handle (owned by caller). Release via
 *         @c pubnub_future_release after reading results.
 *
 * @see pubnub_send_file_result
 * @see pubnub_future_release
 * @see pubnub_send_file_opts_t
 * @see pubnub_context_t
 * @see pubnub_future_t
 */
PUBNUB_API pubnub_future_t pubnub_send_file(pubnub_context_t* ctx,
                                            const pubnub_send_file_opts_t* opts);

/**
 * @brief List files in a PubNub channel.
 *
 * Returns paginated file metadata. Use
 * @c pubnub_list_files_result to read the count and pagination
 * token, then iterate files with @c pubnub_list_files_result_file_at.
 *
 * Example (cooperative polling)
 * @code
 * pubnub_list_files_opts_t opts = PUBNUB_LIST_FILES_OPTS_INIT;
 * opts.channel = "my-channel";
 * opts.limit   = 25;
 *
 * pubnub_future_t fut = pubnub_list_files(ctx, &opts);
 * while (!pubnub_future_is_ready(fut)) {
 *     pubnub_process(ctx);
 * }
 *
 * if (PUBNUB_OK == pubnub_future_status(fut)) {
 *     pubnub_list_files_result_t r = pubnub_list_files_result(fut);
 *     for (uint32_t i = 0; i < r.count; ++i) {
 *         pubnub_file_info_t f =
 *             pubnub_list_files_result_file_at(fut, i);
 *         printf("%.*s (%u bytes)\n",
 *                (int)f.name.len, f.name.ptr, f.size);
 *     }
 * }
 * pubnub_future_release(fut);
 * @endcode
 *
 * @note Required context configuration: @c subscribe_key must be
 *       set in @c pubnub_config_t.
 *
 * @param ctx  Initialized context (@b borrowed).
 * @param opts List-files options (@b required, @b borrowed).
 * @return Future handle (owned by caller). Release via
 *         @c pubnub_future_release after reading results.
 *
 * @see pubnub_list_files_result
 * @see pubnub_future_release
 * @see pubnub_list_files_opts_t
 * @see pubnub_context_t
 * @see pubnub_future_t
 */
PUBNUB_API pubnub_future_t pubnub_list_files(pubnub_context_t* ctx,
                                             const pubnub_list_files_opts_t* opts);

/**
 * @brief Delete a file from a PubNub channel.
 *
 * On success (@c PUBNUB_OK), the file is removed from storage.
 * There is no feature-specific result struct; check
 * @c pubnub_future_status for the outcome.
 *
 * Example (cooperative polling)
 * @code
 * pubnub_delete_file_opts_t opts = PUBNUB_DELETE_FILE_OPTS_INIT;
 * opts.channel   = "my-channel";
 * opts.file_id   = "abc-123";
 * opts.file_name = "report.pdf";
 *
 * pubnub_future_t fut = pubnub_delete_file(ctx, &opts);
 * while (!pubnub_future_is_ready(fut)) {
 *     pubnub_process(ctx);
 * }
 *
 * if (PUBNUB_OK == pubnub_future_status(fut)) {
 *     printf("file deleted\n");
 * }
 * pubnub_future_release(fut);
 * @endcode
 *
 * @note Required context configuration: @c subscribe_key must be
 *       set in @c pubnub_config_t.
 *
 * @param ctx  Initialized context (@b borrowed).
 * @param opts Delete-file options (@b required, @b borrowed).
 * @return Future handle (owned by caller). Release via
 *         @c pubnub_future_release after reading results.
 *
 * @see pubnub_future_release
 * @see pubnub_delete_file_opts_t
 * @see pubnub_context_t
 * @see pubnub_future_t
 */
PUBNUB_API pubnub_future_t pubnub_delete_file(pubnub_context_t* ctx,
                                              const pubnub_delete_file_opts_t* opts);

/**
 * @brief Download a file from a PubNub channel.
 *
 * On success, read the raw bytes via
 * @c pubnub_download_file_result. If crypto is configured on the
 * context, the file is decrypted transparently.
 *
 * Example (cooperative polling)
 * @code
 * pubnub_download_file_opts_t opts = PUBNUB_DOWNLOAD_FILE_OPTS_INIT;
 * opts.channel   = "my-channel";
 * opts.file_id   = "abc-123";
 * opts.file_name = "report.pdf";
 *
 * pubnub_future_t fut = pubnub_download_file(ctx, &opts);
 * while (!pubnub_future_is_ready(fut)) {
 *     pubnub_process(ctx);
 * }
 *
 * if (PUBNUB_OK == pubnub_future_status(fut)) {
 *     pubnub_download_file_result_t r =
 *         pubnub_download_file_result(fut);
 *     fwrite(r.data, 1, r.data_len, output_file);
 * }
 * pubnub_future_release(fut);
 * @endcode
 *
 * @note The entire file content is buffered in memory until
 *       @c pubnub_future_release. On RAM-constrained targets,
 *       restrict downloads to files that fit in available memory.
 *
 * @note Required context configuration: @c subscribe_key must be
 *       set in @c pubnub_config_t.
 *
 * @param ctx  Initialized context (@b borrowed).
 * @param opts Download-file options (@b required, @b borrowed).
 * @return Future handle (owned by caller). Release via
 *         @c pubnub_future_release after reading results.
 *
 * @see pubnub_download_file_result
 * @see pubnub_future_release
 * @see pubnub_download_file_opts_t
 * @see pubnub_context_t
 * @see pubnub_future_t
 */
PUBNUB_API pubnub_future_t pubnub_download_file(pubnub_context_t* ctx,
                                                const pubnub_download_file_opts_t* opts);

/**
 * @brief Publish a file message to a PubNub channel.
 *
 * Use this to manually publish (or re-publish) a file message after a
 * prior @c pubnub_send_file upload succeeded but the publish step
 * failed. The @c file_id and @c file_name come from
 * @c pubnub_send_file_result_t.
 *
 * Example (cooperative polling)
 * @code
 * pubnub_publish_file_message_opts_t opts =
 *     PUBNUB_PUBLISH_FILE_MESSAGE_OPTS_INIT;
 * opts.channel   = "my-channel";
 * opts.file_id   = "abc-123";
 * opts.file_name = "report.pdf";
 * opts.message   = "{\"caption\": \"Q4 report\"}";
 *
 * pubnub_future_t fut = pubnub_publish_file_message(ctx, &opts);
 * while (!pubnub_future_is_ready(fut)) {
 *     pubnub_process(ctx);
 * }
 *
 * if (PUBNUB_OK == pubnub_future_status(fut)) {
 *     pubnub_publish_file_message_result_t r =
 *         pubnub_publish_file_message_result(fut);
 *     printf("published at %.*s\n",
 *            (int)r.timetoken.len, r.timetoken.ptr);
 * }
 * pubnub_future_release(fut);
 * @endcode
 *
 * @note Required context configuration: @c publish_key and
 *       @c subscribe_key must be set in @c pubnub_config_t.
 *
 * @param ctx  Initialized context (@b borrowed).
 * @param opts Publish-file-message options (@b required, @b borrowed).
 * @return Future handle (owned by caller). Release via
 *         @c pubnub_future_release after reading results.
 *
 * @see pubnub_publish_file_message_result
 * @see pubnub_future_release
 * @see pubnub_publish_file_message_opts_t
 * @see pubnub_context_t
 * @see pubnub_future_t
 */
PUBNUB_API pubnub_future_t
pubnub_publish_file_message(pubnub_context_t*                         ctx,
                            const pubnub_publish_file_message_opts_t* opts);

/**
 * @brief Construct the download URL for a file (no network call).
 *
 * Writes the URL into the caller-provided buffer. This is a pure
 * local computation using the context's configured origin and
 * subscribe key.
 *
 * The URL format is:
 * @c https://{origin}/v1/files/{sub_key}/channels/{channel}/files/{id}/{name}
 *
 * Example
 * @code
 *   char url_buf[512];
 *   size_t url_len = 0;
 *   pubnub_get_file_url_opts_t opts = PUBNUB_GET_FILE_URL_OPTS_INIT;
 *   opts.channel   = "my-channel";
 *   opts.file_id   = "abc-123";
 *   opts.file_name = "report.pdf";
 *
 *   pubnub_res_t rc = pubnub_get_file_url(
 *       ctx, &opts, url_buf, sizeof(url_buf), &url_len);
 *   if (PUBNUB_OK == rc) {
 *       printf("URL: %.*s\n", (int)url_len, url_buf);
 *   }
 * @endcode
 *
 * @param ctx      Initialized context (@b borrowed).
 * @param opts     Options identifying the file.
 * @param buf      Caller-owned buffer to receive the URL.
 * @param buf_size Capacity of @p buf in bytes.
 * @param out_len  Receives the number of bytes written (excluding
 *                 NUL terminator). Pass @c NULL if not needed.
 * @return @c PUBNUB_OK on success.
 * @retval PUBNUB_ERR_INVALID_ARGUMENT  @c NULL context, missing required
 *         fields, or @c NULL @p buf.
 * @retval PUBNUB_ERR_BUFFER_TOO_SMALL  @p buf_size is insufficient
 *         for the URL (check @p out_len for required size).
 */
PUBNUB_API pubnub_res_t pubnub_get_file_url(pubnub_context_t* ctx,
                                            const pubnub_get_file_url_opts_t* opts,
                                            char*   buf,
                                            size_t  buf_size,
                                            size_t* out_len);

/**
 * @brief Extract the result from a completed @c pubnub_send_file
 *        future.
 *
 * @param future Future returned from @c pubnub_send_file.
 * @return Result struct on success; zero-initialized if the future is
 *         not ready, carries an error, or the response did not parse.
 */
PUBNUB_API pubnub_send_file_result_t pubnub_send_file_result(pubnub_future_t future);

/**
 * @brief Extract the result from a completed @c pubnub_list_files
 *        future.
 *
 * @param future Future returned from @c pubnub_list_files.
 * @return Result struct with @c count and pagination @c next token.
 *         Zero-initialized if the future is not ready or carries an
 *         error.
 */
PUBNUB_API pubnub_list_files_result_t pubnub_list_files_result(pubnub_future_t future);

/**
 * @brief Retrieve file metadata at @p index from a completed
 *        @c pubnub_list_files future.
 *
 * @param future Future returned from @c pubnub_list_files.
 * @param index  Zero-based index (must be < @c result.count).
 * @return File info struct. Zero-initialized if @p index is out of
 *         range or the future is not ready.
 */
PUBNUB_API pubnub_file_info_t pubnub_list_files_result_file_at(pubnub_future_t future,
                                                               size_t index);

/**
 * @brief Extract the result from a completed
 *        @c pubnub_download_file future.
 *
 * @param future Future returned from @c pubnub_download_file.
 * @return Result struct with data pointer and length.
 *         Zero-initialized if the future is not ready or carries an
 *         error.
 */
PUBNUB_API pubnub_download_file_result_t
pubnub_download_file_result(pubnub_future_t future);

/**
 * @brief Extract the result from a completed
 *        @c pubnub_publish_file_message future.
 *
 * @param future Future returned from @c pubnub_publish_file_message.
 * @return Result struct with the publish timetoken. Zero-initialized
 *         if the future is not ready or carries an error.
 */
PUBNUB_API pubnub_publish_file_message_result_t
pubnub_publish_file_message_result(pubnub_future_t future);

#ifdef __cplusplus
// clang-format off
}
// clang-format on
#endif

#endif /* PUBNUB_ENABLE_FILES */

#endif /* PUBNUB_FEATURE_FILES_H */
