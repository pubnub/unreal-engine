/* Copyright (c) PubNub Inc. */
/* See LICENSE in the root directory of this source tree. */

/**
 * @file types_fwd.h
 * @brief Forward-declaration typedefs for opaque SDK handle types.
 */

#ifndef PUBNUB_TYPES_FWD_H
#define PUBNUB_TYPES_FWD_H

#ifdef __cplusplus
// clang-format off
extern "C" {
// clang-format on
#endif

/** Opaque PubNub client context. */
typedef struct pubnub_context pubnub_context_t;

/** Opaque crypto module for payload encryption/decryption. */
typedef struct pubnub_crypto_module pubnub_crypto_module_t;

#ifdef __cplusplus
// clang-format off
}
// clang-format on
#endif

#endif /* PUBNUB_TYPES_FWD_H */
