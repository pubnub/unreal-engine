# AGENTS.md — PubNub Unreal plugins

Rules for work in `Plugins/Pubnub` (core) and `Plugins/PubnubChat`. This file is not a substitute for the current code. When it conflicts with the code, the code wins; update this file if the code is the intended behavior.

`Source/SDKsTest` is an empty host used to open these plugins. Do not change it unless the task is about that host. `Plugins/PubnubGamingSDK` is a generated Fab package, not a source tree.

Feature-specific names and behavior come from the task. Follow the task and the nearest existing function, not a new design.

## How to work

- Match the nearest peer: same types, same macros, same sync/async split. Do not add a new thread, service object, result base, or abstraction for one call.
- Keep the change scoped. Do not refactor unrelated APIs, rename public types, or reorder `UENUM` values.
- Do not invent credentials. Tests already fall back to demo keys.
- Do not bump versions or changelog. `PubnubLibrary.uplugin` `VersionName`, `PubnubChat.uplugin` `VersionName`, `PubnubLibraryVersion.h`, `PubnubChatVersion.h`, and each plugin's `.pubnub.yml` are release-owned. They do not currently agree with each other. Do not reconcile them unless asked. Chat compile-depends on `PUBNUB_LIBRARY_VERSION` in `PubnubLibraryVersion.h`.
- Keep the engine's default C++ standard. Do not set `CppStandard` or `/std:c++20`. Do not use designated initializers.
- Do not edit `Plugins/Pubnub/Source/ThirdParty/sdk` sources unless the task is a C library drop. Shipped link flags live in `PubNubModule.Build.cs`.

## Where code lives

| Area | Path |
|---|---|
| Core runtime | `Plugins/Pubnub/Source/PubnubLibrary` |
| Core tests and samples | `Plugins/Pubnub/Source/PubnubLibraryTests` |
| Shipped C library and link module | `Plugins/Pubnub/Source/ThirdParty/sdk` |
| Chat runtime | `Plugins/PubnubChat/Source/PubnubChatSDK` |
| Chat tests and samples | `Plugins/PubnubChat/Source/PubnubChatSDKTests` |

Chat's `.uplugin` depends on `PubnubLibrary`. Chat calls `UPubnubClient` and the other public core types. It does not include C headers and must not start doing so.

`Public/` is the customer API (Blueprint and C++). `Private/` is implementation. A file under `Public/` whose name says Internal (`PubnubInternalUtilities.h`) is still not a customer API: do not add `UFUNCTION`s there, and do not include it from Chat.

Every plugin source file starts with `// Copyright 2026 PubNub Inc. All Rights Reserved.`

## Public comments

Public headers are read by customers in the editor and in generated docs.

- Say what the caller does and what they get back.
- Say whether the call blocks on the network (`Blocking:`) or only touches local state (`Local:`). Chat headers already use these words; keep using them.
- Name a parameter only when the type does not make it obvious, or when a default is surprising.
- Do not mention the C library, its types, or its functions. Customers use this plugin, not that library. Chat comments may name `UPubnubClient` and the core UE SDK, because that dependency is public.
- The synchronous method carries the behavior. Each async overload is one short note that it is non-blocking, plus the callback parameter. Do not paste the same paragraph onto all three overloads.
- Do not document obvious Blueprint specifiers.

```cpp
/**
 * Publishes a message to a channel.
 * Blocking: waits for the publish result on the calling thread.
 * A plain string is sent as a JSON string. A JSON object or array is sent as-is.
 *
 * @param Channel Channel ID.
 * @return Publish result, including the timetoken when the publish succeeds.
 */
```

Private comments can name the C library when that is what the next reader needs. Keep those short too.

## Implementation

- Core network work goes in the existing `*_priv` function on `UPubnubClient`. The public sync method checks initialization and calls `_priv`. Async methods only queue that call. See [`.ai/knowledge/pubnub-core.md`](.ai/knowledge/pubnub-core.md).
- Chat has no `_priv`. The sync method is the implementation and calls `UPubnubClient`. Async methods queue a call to that sync method. See [`.ai/knowledge/pubnub-chat.md`](.ai/knowledge/pubnub-chat.md).
- Copy the peer's macros for "not initialized", empty fields, and failed steps. Do not open-code those checks.
- Create `UObject`s with `UPubnubInternalUtilities::SafeNewObject`. `NewObject` from a background thread races garbage collection.
- Captures queued onto a function thread use `TWeakObjectPtr` and return when it is invalid.
- User-facing failures go through the existing result structs. Do not throw.

## Compiler and test output

`.cursor/hooks/` rewrites these shell commands before they run:

- `ue_compile_wrap.py` — Unreal Build Tool and MSBuild
- `ue_plugin_compile_wrap.py` — `RunUAT BuildPlugin`
- `ue_test_wrap.py` — Unreal automation, Gauntlet, and LowLevelTests

The shell result is the summary between the start and end banners. Use that summary as the result. Do not open `raw_log` unless a cited file or line is missing. Do not call the wrap scripts. Tests are run in the editor. Do not launch them from the agent.

## Before completing a coding task

1. Inspect the diff for unrelated edits.
2. Use the `ue-plugin-compile` skill when the task asks to compile a plugin. Do not launch automation tests.
3. Report the commands you actually ran. Do not claim a compile or a test passed if it was not run.

## Detailed knowledge

When changing the core plugin (client, entities, subscriptions, crypto, C-library calls, core tests), read [`.ai/knowledge/pubnub-core.md`](.ai/knowledge/pubnub-core.md).

When changing the chat plugin (chat, user, channel, message, membership, listeners, chat tests), read [`.ai/knowledge/pubnub-chat.md`](.ai/knowledge/pubnub-chat.md).

A change that crosses both (chat calling a new core method) needs both files.
