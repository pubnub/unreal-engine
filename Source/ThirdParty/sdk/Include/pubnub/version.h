/* Copyright (c) PubNub Inc. */
/* See LICENSE in the root directory of this source tree. */

/**
 * @file version.h
 * @brief SDK version information.
 */

#ifndef PUBNUB_VERSION_H
#define PUBNUB_VERSION_H

#include "pubnub/config.h"
#include "pubnub/pubnub_compat.h"

#ifdef __cplusplus
// clang-format off
extern "C" {
// clang-format on
#endif

/**
 * @brief Return the SDK version string (e.g. "0.1.0").
 *
 * The returned pointer is to a static string literal.
 */
PUBNUB_API const char* pubnub_sdk_version(void);

#ifdef __cplusplus
// clang-format off
}
// clang-format on
#endif

#endif /* PUBNUB_VERSION_H */
