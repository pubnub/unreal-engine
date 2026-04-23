// Copyright 2026 PubNub Inc. All Rights Reserved.

#pragma once

/** C PubNub SDK (ThirdParty) version string, e.g. 7.1.3. Keep in sync with PUBNUB_SDK_VERSION in core/pubnub_version_internal.h. */
#define PUBNUB_C_CORE_VERSION "7.1.3"

/** Keep in sync with VersionName in PubnubLibrary.uplugin. Used by dependent plugins (e.g. PubnubChat) for version checks. Two digits per component (max 99 each). */
#define PUBNUB_LIBRARY_VERSION_MAJOR 2
#define PUBNUB_LIBRARY_VERSION_MINOR 0
#define PUBNUB_LIBRARY_VERSION_PATCH 2
#define PUBNUB_LIBRARY_VERSION ((PUBNUB_LIBRARY_VERSION_MAJOR * 10000) + (PUBNUB_LIBRARY_VERSION_MINOR * 100) + PUBNUB_LIBRARY_VERSION_PATCH)
