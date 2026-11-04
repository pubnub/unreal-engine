/* Copyright (c) PubNub Inc. */
/* See LICENSE in the root directory of this source tree. */

#ifndef PUBNUB_FEATURE_CRYPTO_H
#define PUBNUB_FEATURE_CRYPTO_H

#include "pubnub/config.h"

#if PUBNUB_ENABLE_CRYPTO

#include "pubnub/error.h"
#include "pubnub/types_fwd.h"
#include "pubnub/providers/allocator.h"
#include "pubnub/providers/crypto.h"

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
// clang-format off
extern "C" {
// clang-format on
#endif

/**
 * @brief Create a crypto module with AES-CBC (ACRH) as the default
 *        cryptor.
 *
 * Encrypts using AES-256-CBC with random IV (identifier "ACRH").
 * Decrypts data produced by both ACRH and legacy cryptors.
 *
 * The module owns both cryptor instances and destroys them when
 * @c pubnub_crypto_module_destroy is called.
 *
 * @param cipher_key    NUL-terminated cipher key string (borrowed).
 * @param use_random_iv Controls the legacy fallback cryptor's IV mode.
 *                      1 = expect random IV prepended to legacy
 *                      ciphertext. 0 = use static IV
 *                      "0123456789012345" for legacy decryption.
 * @param alloc         Allocator for internal state (@c NULL = compiled-in
 *                      default).
 * @return Module handle, or @c NULL on allocation/key-derivation failure.
 */
PUBNUB_API pubnub_crypto_module_t*
pubnub_crypto_module_aes_cbc(const char*                  cipher_key,
                             int                          use_random_iv,
                             pubnub_allocator_provider_t* alloc);

/**
 * @brief Create a crypto module with the legacy cryptor as default.
 *
 * Encrypts using legacy AES-256-CBC (identifier {0,0,0,0}).
 * Decrypts data produced by both legacy and ACRH cryptors.
 *
 * The module owns both cryptor instances and destroys them when
 * @c pubnub_crypto_module_destroy is called.
 *
 * @param cipher_key    NUL-terminated cipher key string (borrowed).
 * @param use_random_iv 1 = random IV per encrypt (prepended to
 *                      ciphertext), 0 = static IV "0123456789012345".
 * @param alloc         Allocator for internal state (@c NULL = compiled-in
 *                      default).
 * @return Module handle, or @c NULL on allocation/key-derivation failure.
 */
PUBNUB_API pubnub_crypto_module_t*
pubnub_crypto_module_legacy(const char*                  cipher_key,
                            int                          use_random_iv,
                            pubnub_allocator_provider_t* alloc);

/**
 * @brief Create a crypto module with a custom cryptor set.
 *
 * The @p default_cryptor handles all encryption. The @p others array
 * provides fallback cryptors for decryption (matched by identifier).
 *
 * The module does NOT take ownership of cryptor instances; the caller
 * manages their lifetime and must destroy them after destroying the
 * module.
 *
 * @param default_cryptor Cryptor for encryption (borrowed, non-NULL).
 * @param others          Array of fallback cryptors (borrowed, may be
 *                        @c NULL when @p others_count is 0).
 * @param others_count    Number of entries in @p others. Must not
 *                        exceed PUBNUB_CFG_CRYPTO_MAX_FALLBACK_CRYPTORS.
 * @param alloc           Allocator for the module struct (@c NULL =
 *                        compiled-in default).
 * @return Module handle, or @c NULL on allocation failure or
 *         @p others_count exceeds the compile-time limit.
 */
PUBNUB_API pubnub_crypto_module_t*
pubnub_crypto_module_create(pubnub_crypto_provider_t*    default_cryptor,
                            pubnub_crypto_provider_t**   others,
                            size_t                       others_count,
                            pubnub_allocator_provider_t* alloc);

/**
 * @brief Destroy a crypto module and release resources.
 *
 * For modules created via factory functions (@c
 * pubnub_crypto_module_aes_cbc, @c pubnub_crypto_module_legacy),
 * this also destroys the internally-owned cryptor instances and
 * securely zeros key material.
 *
 * For modules created via @c pubnub_crypto_module_create, only the
 * module struct is freed; caller-owned cryptors are not touched.
 *
 * Uses the allocator stored at creation time — no allocator parameter
 * needed.
 *
 * @param module Module to destroy (@b optional). @c NULL-safe.
 */
PUBNUB_API void pubnub_crypto_module_destroy(pubnub_crypto_module_t* module);

/**
 * @brief Query required output buffer size for encryption.
 *
 * Returns the worst-case byte count for encrypted output (PNED header
 * + metadata + ciphertext) given a plaintext of @p input_len bytes.
 * Use to pre-allocate a buffer before calling @c
 * pubnub_crypto_module_encrypt_buf.
 *
 * @param module    Initialized module (@b required).
 * @param input_len Plaintext length in bytes.
 * @return Required output buffer size, or 0 on error.
 */
PUBNUB_API size_t pubnub_crypto_module_encrypt_size(pubnub_crypto_module_t* module,
                                                    size_t input_len);

/**
 * @brief Query required base64 output size for encryption.
 *
 * Returns the worst-case byte count for base64-encoded encrypted
 * output including the NUL terminator.
 *
 * @param module    Initialized module (@b required).
 * @param input_len Plaintext length in bytes.
 * @return Required buffer size including NUL, or 0 on error.
 */
PUBNUB_API size_t pubnub_crypto_module_encrypted_base64_size(pubnub_crypto_module_t* module,
                                                             size_t input_len);

/**
 * @brief Encrypt plaintext into a caller-provided buffer.
 *
 * No allocation is performed. The buffer must be at least
 * @c pubnub_crypto_module_encrypt_size bytes.
 *
 * @param module     Initialized module (@b required, @b borrowed).
 * @param input      Plaintext bytes (@b required, @b borrowed).
 * @param input_len  Plaintext length in bytes.
 * @param output     Pre-allocated output buffer (@b required).
 * @param output_cap Buffer capacity in bytes.
 * @param output_len Receives actual bytes written on success (@b required).
 * @return PUBNUB_OK on success, PUBNUB_ERR_BUFFER_TOO_SMALL if
 *         capacity is insufficient, PUBNUB_ERR_CRYPTO on encrypt
 *         failure.
 */
PUBNUB_API pubnub_res_t pubnub_crypto_module_encrypt_buf(pubnub_crypto_module_t* module,
                                                         const uint8_t* input,
                                                         size_t   input_len,
                                                         uint8_t* output,
                                                         size_t   output_cap,
                                                         size_t*  output_len);

/**
 * @brief Decrypt a binary blob into a caller-provided buffer.
 *
 * No allocation is performed. The buffer MUST be at least
 * @p input_len bytes; returns @c PUBNUB_ERR_BUFFER_TOO_SMALL if not.
 *
 * @param module     Initialized module (@b required, @b borrowed).
 * @param input      Binary blob (@b required, @b borrowed — PNED header
 *                   + ciphertext, or raw legacy ciphertext).
 * @param input_len  Blob length in bytes.
 * @param output     Pre-allocated output buffer (@b required).
 * @param output_cap Buffer capacity in bytes (MUST be >= @p input_len).
 * @param output_len Receives actual plaintext length on success (@b required).
 * @return PUBNUB_OK on success, PUBNUB_ERR_BUFFER_TOO_SMALL if
 *         @p output_cap < @p input_len, PUBNUB_ERR_CRYPTO on failure
 *         (no matching cryptor, decryption error, truncated header).
 */
PUBNUB_API pubnub_res_t pubnub_crypto_module_decrypt_buf(pubnub_crypto_module_t* module,
                                                         const uint8_t* input,
                                                         size_t   input_len,
                                                         uint8_t* output,
                                                         size_t   output_cap,
                                                         size_t*  output_len);

/**
 * @brief Encrypt plaintext to an allocated binary blob.
 *
 * Free the output via @c pubnub_crypto_module_free.
 *
 * @param module     Initialized module (@b required, @b borrowed).
 * @param input      Plaintext bytes (@b required, @b borrowed).
 * @param input_len  Plaintext length in bytes.
 * @param output     Receives allocated output buffer pointer (@b required).
 * @param output_len Receives output length in bytes (@b required).
 * @return PUBNUB_OK on success, PUBNUB_ERR_OUT_OF_MEMORY or
 *         PUBNUB_ERR_CRYPTO on failure.
 */
PUBNUB_API pubnub_res_t pubnub_crypto_module_encrypt(pubnub_crypto_module_t* module,
                                                     const uint8_t* input,
                                                     size_t         input_len,
                                                     uint8_t**      output,
                                                     size_t*        output_len);

/**
 * @brief Decrypt a binary blob to allocated plaintext.
 *
 * Free the output via @c pubnub_crypto_module_free.
 *
 * @param module     Initialized module (@b required, @b borrowed).
 * @param input      Binary blob (@b required, @b borrowed — PNED header
 *                   + ciphertext).
 * @param input_len  Blob length in bytes.
 * @param output     Receives allocated plaintext buffer (@b required).
 * @param output_len Receives plaintext length in bytes (@b required).
 * @return PUBNUB_OK on success, PUBNUB_ERR_CRYPTO on failure.
 */
PUBNUB_API pubnub_res_t pubnub_crypto_module_decrypt(pubnub_crypto_module_t* module,
                                                     const uint8_t* input,
                                                     size_t         input_len,
                                                     uint8_t**      output,
                                                     size_t*        output_len);

/**
 * @brief Free a buffer returned by encrypt/decrypt functions.
 *
 * @param module Module that produced the buffer (@b required, @b borrowed).
 * @param ptr    Pointer to free (@b optional). @c NULL-safe.
 */
PUBNUB_API void pubnub_crypto_module_free(pubnub_crypto_module_t* module, void* ptr);

/**
 * @brief Encrypt and base64-encode in one step.
 *
 * @param module     Module handle (@b required, @b borrowed).
 * @param input      Plaintext bytes (@b required, @b borrowed).
 * @param input_len  Plaintext length in bytes.
 * @param out_base64 Receives allocator-owned base64 string (@b required).
 *                   Free via @c pubnub_crypto_module_free.
 * @param out_len    Receives base64 string length in bytes (@b required).
 * @return PUBNUB_OK on success.
 */
PUBNUB_API pubnub_res_t
pubnub_crypto_module_encrypt_to_base64(pubnub_crypto_module_t* module,
                                       const uint8_t*          input,
                                       size_t                  input_len,
                                       char**                  out_base64,
                                       size_t*                 out_len);

/**
 * @brief Decode base64 and decrypt in one step.
 *
 * @param module     Module handle (@b required, @b borrowed).
 * @param base64     Base64-encoded ciphertext (@b required, @b borrowed).
 * @param base64_len Length of @p base64 in bytes.
 * @param output     Receives allocator-owned plaintext (@b required).
 *                   Free via @c pubnub_crypto_module_free.
 * @param output_len Receives plaintext length in bytes (@b required).
 * @return PUBNUB_OK on success.
 */
PUBNUB_API pubnub_res_t
pubnub_crypto_module_decrypt_from_base64(pubnub_crypto_module_t* module,
                                         const char*             base64,
                                         size_t                  base64_len,
                                         uint8_t**               output,
                                         size_t*                 output_len);

/**
 * @brief Get the default cryptor from a module.
 *
 * The returned pointer is borrowed and valid for the module's
 * lifetime. Used internally by the SDK to access the provider vtable.
 *
 * @param module Module handle (@b required, @b borrowed).
 * @return Default cryptor provider pointer.
 */
PUBNUB_API pubnub_crypto_provider_t*
pubnub_crypto_module_default_cryptor(pubnub_crypto_module_t* module);

/**
 * @brief Create an AES-CBC cryptor with random IV (ACRH).
 *
 * Identifier: "ACRH" (0x41 0x43 0x52 0x48).
 * Key derivation: SHA-256 hash of @p cipher_key (raw 32 bytes).
 * IV: 16 random bytes per encrypt via backend PRNG.
 * Algorithm: AES-256-CBC with PKCS#7 padding.
 *
 * @param cipher_key NUL-terminated cipher key string.
 * @param alloc      Allocator for cryptor state.  Pass @c NULL to use
 *                   the compiled-in default; returns @c NULL if both
 *                   are unavailable.
 * @return Cryptor instance, or @c NULL on failure.
 */
PUBNUB_API pubnub_crypto_provider_t*
pubnub_cryptor_aes_cbc_create(const char*                  cipher_key,
                              pubnub_allocator_provider_t* alloc);

/**
 * @brief Create a legacy cryptor.
 *
 * Identifier: {0, 0, 0, 0}.
 * Key derivation: SHA-256 of @p cipher_key, hex-encoded, first 32
 * chars used as key bytes.
 * IV: random (prepended to ciphertext) or static "0123456789012345".
 * Algorithm: AES-256-CBC with PKCS#7 padding. No PNED header emitted.
 *
 * @param cipher_key    NUL-terminated cipher key string.
 * @param use_random_iv 1 = generate random IV per encrypt and prepend
 *                      to output. 0 = use static IV "0123456789012345".
 * @param alloc         Allocator for cryptor state.  Pass @c NULL to use
 *                      the compiled-in default; returns @c NULL if both
 *                      are unavailable.
 * @return Cryptor instance, or @c NULL on failure.
 */
PUBNUB_API pubnub_crypto_provider_t*
pubnub_cryptor_legacy_create(const char*                  cipher_key,
                             int                          use_random_iv,
                             pubnub_allocator_provider_t* alloc);

/**
 * @brief Destroy a cryptor instance.
 *
 * Securely zeros key material before freeing.
 *
 * @param cryptor Cryptor to destroy (@c NULL-safe).
 */
PUBNUB_API void pubnub_cryptor_destroy(pubnub_crypto_provider_t* cryptor);

#ifdef __cplusplus
// clang-format off
}
// clang-format on
#endif

#endif /* PUBNUB_ENABLE_CRYPTO */

#endif /* PUBNUB_FEATURE_CRYPTO_H */
