/* Copyright (c) PubNub Inc. */
/* See LICENSE in the root directory of this source tree. */

/**
 * @file providers/crypto.h
 * @brief Payload crypto provider interface.
 *
 * This provider handles **user-data payload encryption/decryption**
 * (e.g., AES-CBC for message content) and HMAC signing for PAM.
 *
 * It does NOT handle transport-level encryption (TLS). Transport
 * encryption is managed internally by the transport provider when
 * PUBNUB_ENABLE_SECURE_TRANSPORT is enabled.
 *
 * The vtable aligns with c-core's pubnub_cryptor_t pattern: each
 * crypto provider carries a 4-byte algorithm identifier and produces
 * structured encrypted data (ciphertext + algorithm-specific metadata
 * such as an IV).
 *
 * This is a **per-context** provider. The SDK calls init() during
 * context initialization and deinit() during context teardown.
 *
 * Implementation-specific state (cipher key, algorithm context, etc.)
 * should be stored in an extended struct with this vtable as the
 * first member.
 *
 * All callbacks are invoked from normal (non-ISR) context only.
 */

#ifndef PUBNUB_PROVIDER_CRYPTO_H
#define PUBNUB_PROVIDER_CRYPTO_H

#include "pubnub/config.h"
#include "pubnub/error.h"
#include "pubnub/providers/provider_deps.h"

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
// clang-format off
extern "C" {
// clang-format on
#endif

/**
 * @brief Structured encrypted data.
 *
 * Carries ciphertext together with algorithm-specific metadata
 * (e.g., initialization vector, nonce) for protocol compatibility
 * with c-core's encrypted message format.
 *
 * Pointers are borrowed; the caller owns the underlying buffers.
 */
typedef struct pubnub_encrypted_data {
    /** Encrypted ciphertext. */
    uint8_t* data;
    /** Ciphertext length in bytes. */
    size_t data_len;
    /** Algorithm-specific metadata (e.g., IV). May be @c NULL. */
    uint8_t* metadata;
    /** Metadata length in bytes. */
    size_t metadata_len;
} pubnub_encrypted_data_t;

/**
 * @brief Crypto provider function table.
 *
 * All callbacks are invoked from normal (non-ISR) context only.
 */
typedef struct pubnub_crypto_provider {
    /**
     * @brief 4-byte algorithm identifier.
     *
     * Matches the c-core cryptor identifier protocol. Used by the
     * SDK to select the correct decryptor when multiple algorithms
     * are registered. Example: "ACRH" for AES-CBC with random IV.
     * Set to all zeros for legacy/default algorithm.
     */
    uint8_t identifier[4];

    /**
     * @brief Query the required output buffer size for encryption.
     *
     * Allows the caller to pre-allocate output buffers before
     * calling encrypt. Embedded-friendly: no surprise allocations.
     *
     * @param self          Pointer to this provider instance.
     * @param plaintext_len Length of the plaintext to encrypt.
     * @return Required buffer size in bytes, or 0 on error.
     */
    size_t (*encrypt_size)(struct pubnub_crypto_provider* self,
                           size_t                         plaintext_len);

    /**
     * @brief Encrypt plaintext into structured encrypted data.
     *
     * The caller provides output buffers in @p output sized via
     * encrypt_size(). On success, output->data_len and
     * output->metadata_len are set to the actual bytes written.
     *
     * @param self      Pointer to this provider instance.
     * @param input     Plaintext input.
     * @param input_len Input length in bytes.
     * @param output    Output descriptor with pre-allocated buffers.
     * @return PUBNUB_OK on success.
     */
    pubnub_res_t (*encrypt)(struct pubnub_crypto_provider* self,
                            const uint8_t*                 input,
                            size_t                         input_len,
                            pubnub_encrypted_data_t*       output);

    /**
     * @brief Decrypt structured encrypted data into plaintext.
     *
     * @param self       Pointer to this provider instance.
     * @param input      Encrypted data descriptor (borrowed).
     * @param output     Output buffer for plaintext.
     * @param output_len On entry: output buffer size.
     *                   On exit: bytes written.
     * @return PUBNUB_OK on success.
     */
    pubnub_res_t (*decrypt)(struct pubnub_crypto_provider* self,
                            const pubnub_encrypted_data_t* input,
                            uint8_t*                       output,
                            size_t*                        output_len);

    /**
     * @brief Compute HMAC-SHA256 signature (for PAM signing).
     *
     * @param self       Pointer to this provider instance.
     * @param key        Signing key.
     * @param key_len    Key length in bytes.
     * @param data       Data to sign.
     * @param data_len   Data length in bytes.
     * @param output     Output buffer (at least 32 bytes).
     * @param output_len On entry: output buffer size.
     *                   On exit: bytes written (32).
     * @return PUBNUB_OK on success.
     */
    pubnub_res_t (*hmac_sha256)(struct pubnub_crypto_provider* self,
                                const uint8_t*                 key,
                                size_t                         key_len,
                                const uint8_t*                 data,
                                size_t                         data_len,
                                uint8_t*                       output,
                                size_t*                        output_len);

    /**
     * @brief Per-context initialization.
     *
     * Called by the SDK core after all providers are resolved.
     * The provider may allocate per-context resources using
     * deps->allocator.
     *
     * Optional: @c NULL = no per-context init needed.
     *
     * @param self Pointer to this provider instance.
     * @param deps Shared infrastructure providers.
     * @return 0 on success, non-zero on failure.
     */
    int (*init)(struct pubnub_crypto_provider* self,
                const pubnub_provider_deps_t*  deps);

    /**
     * @brief Per-context de-initialization.
     *
     * Called by the SDK core during pubnub_deinit(). Release
     * per-context resources allocated during init.
     *
     * Optional: @c NULL = no cleanup needed.
     *
     * @param self Pointer to this provider instance.
     */
    void (*deinit)(struct pubnub_crypto_provider* self);
} pubnub_crypto_provider_t;

#ifdef __cplusplus
// clang-format off
}
// clang-format on
#endif

#endif /* PUBNUB_PROVIDER_CRYPTO_H */
