# PubNub Chat (`Plugins/PubnubChat`)

How chat objects call the core plugin and stay in sync. Core threading and `_priv` rules are in [pubnub-core.md](pubnub-core.md). Comment and version rules are in [AGENTS.md](../../AGENTS.md).

## Surface

`UPubnubChatSubsystem` owns chats in a map keyed by user ID. `InitChat` builds a `UPubnubClient` and a `UPubnubChat`. `InitChatWithPubnubClient` uses a client the caller already has. A second init for the same user ID returns an error and the existing chat. `GetChat` is local.

`UPubnubChat` is the factory and the session: users, channels, memberships, presence, unread counts. The objects are `UPubnubChatUser`, `UPubnubChatChannel`, `UPubnubChatMessage`, `UPubnubChatMembership`. `UPubnubChatThreadChannel` extends the channel. `UPubnubChatThreadMessage` extends the message. `UPubnubChatMessageDraft` is an in-memory draft owned by a channel until `Send`.

`UPubnubChatAccessManager` reads the current auth token locally (`CanI`, `ParseToken`, `SetAuthToken`). It does not grant tokens. Granting stays on `UPubnubClient`.

Public structs are split across `Public/StructLibraries/`. Shared constants that are not customer-facing (`Pubnub_Chat_*`) live in `Private/PubnubChatConst.h`.

Chat does not include C headers. A new chat feature calls a public `UPubnubClient` method (or another chat object). If the core method does not exist, add it on the core plugin first, using [pubnub-core.md](pubnub-core.md).

## Sync and async

Blocking names have no suffix. Non-blocking names end in `Async`. There is no `_priv`.

Follow `UPubnubChat::CreateUser`:

1. Sync method: `PUBNUB_CHAT_RETURN_WRAPPER_IF_NOT_INITIALIZED`, then field/object/condition macros, then one or more `UPubnubClient` calls, then build the chat object.
2. Blueprint async overload binds the dynamic delegate to the native delegate.
3. Native async overload checks init (the `RETURN_WITH_DELEGATE_*` macro, which also requires `AsyncFunctionsThread`), queues onto that thread, calls the sync method, then `UPubnubUtilities::CallPubnubDelegate`.

The lambda captures `TWeakObjectPtr` and returns if it is invalid. Move input structs into the lambda (`UserData = MoveTemp(UserData)`).

`AsyncFunctionsThread` is a `FPubnubFunctionThread` owned by the chat. Queued work calls the sync chat method, which calls the sync core method. Do not call `UPubnubClient::*Async` from that queue. Core async would queue again onto the client thread and fight the operation mutex described in [pubnub-core.md](pubnub-core.md).

Object methods (`UPubnubChatUser::Update` and peers) use the `PUBNUB_CHAT_OBJECT_*` macros and queue on `Chat->AsyncFunctionsThread`, not a thread of their own.

`InitChat` sets `bOwnsPubnubClient` only when the subsystem created the client. `DestroyChat` must not destroy a client the caller owns.

## Operation results

`FPubnubChatOperationResult` is the chat-level result: `Error`, `ErrorMessage`, and `StepResults`. Each step stores the core `FPubnubOperationResult` under the core method name (`"SetUserMetadata"`, `"GetUserMetadata"`).

- Local failure: `FPubnubChatOperationResult::CreateError`.
- After a core call: `PUBNUB_CHAT_ADD_PUBNUB_RESULT_AND_RETURN_WRAPPER_IF_ERROR` (wrapper) or `PUBNUB_CHAT_ADD_PUBNUB_RESULT_AND_RETURN_OPR_RESULT_IF_ERROR`.
- After another chat call: `PUBNUB_CHAT_MERGE_CHAT_RESULT_AND_RETURN_*`.

Those macros return on `Error`. Do not continue a multi-step call after a failed step unless the peer you are copying does.

`CreateUser` treats "user already exists" as an error and tells the caller to use `GetUser`. `CreatePublicConversation` overwrites `ChannelData.Type`. Keep those method-specific checks next to the peer, not in a new validator type.

## Repository

`UPubnubChatObjectsRepository` is internal. Every live wrapper with the same ID shares one stored record. Getters copy under a lock (`TryGetUserData` and the channel, message, and membership twins). Updates go back through `UpdateUserData` and the matching twins so the other wrappers see them. Register on create, unregister on destroy. The record is removed when the count hits zero.

IDs:

- User and channel: the PubNub id.
- Message: `[ChannelID].[Timetoken]`.
- Membership: `[UserID].[ChannelID]`.

Do not keep a long-lived pointer into a repository struct. Copy, edit, write back.

## Channels, threads, and drafts

`Type` on channel metadata is the chat kind, not a C enum:

- `CreatePublicConversation` forces `"public"` and requires a channel ID.
- `CreateGroupConversation` forces `"group"`. An empty channel ID becomes a new GUID. It then creates the host membership and invites the users.
- `CreateDirectConversation` forces `"direct"`. An empty channel ID becomes `direct.` plus a hash of the two user IDs sorted and joined with `&`.

Those three reject a thread ID. Threads use the `PUBNUB_INTERNAL_THREAD` prefix and `CreateThread`. Moderation channels use the `PUBNUB_INTERNAL_MODERATION` prefix. Chat-private JSON fields use the `PN_PRIV.` prefix. Do not invent a second prefix.

`FPubnubChatConfig::EmitReadReceiptEvents` is keyed by `"public"`, `"group"`, and `"direct"`. The C++ default is public off, group on, direct on. Blueprint `Make Struct` does not run that constructor. Callers that want the defaults use `UPubnubChatSubsystem::GetDefaultChatConfig`.

`UPubnubChatMessageDraft` does not hit the network until `Send`. Mentions are markdown links, `pn-user://` and `pn-channel://`. Draft unit tests do not need a client.

Listen APIs return `UPubnubChatCallbackStop`. `Stop()` runs the stored unlisten function. `BeginDestroy` stops too. Do not add a second unlisten handle type.

## Connection status

Chat listens to `UPubnubClient::OnSubscriptionStatusChanged` and rebroadcasts `OnConnectionStatusChanged`. It drops `EPubnubSubscriptionStatus::PSS_SubscriptionChanged`. A successful chat subscribe still only means the subscription was registered. Handshake completion is the later status event. Tests that need the active channel list listen on the client, not on the chat delegate.

## Tests

Tests are Unreal automation tests in `PubnubChatSDKTests`. The test module adds the chat SDK directory to its include path so it can reach private helpers. Do not move a helper to `Public/` only to test it.

- No client: `IMPLEMENT_SIMPLE_AUTOMATION_TEST`, names `PubnubChat.Unit.*` (message draft).
- Session tests: `IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST` on `FPubnubChatAutomationTestBase`, names `PubnubChat.Integration.*`. `InitTest()` first, `CleanUp()` last, `CleanUpCurrentChatUser` when the test created the chat user.
- Network tests use latent commands and return true from `RunTest`.
- Wait for the core handshake with `FChatHandshakeTracker` and `FWaitForChatHandshakeCommand` before expecting a message. `PSS_Connected` and `PSS_SubscriptionChanged` count, and both are observed on `UPubnubClient`.
- Keys: `PUBNUB_PUBLISH_KEY` and `PUBNUB_SUBSCRIBE_KEY`. Unset keys fall back to `demo`. Ids start with `UE_ChatSDK_`.

Core and chat tests share the host editor but not keys, prefixes, or base classes. Do not point a chat test at `PN_PUB_KEY`.
