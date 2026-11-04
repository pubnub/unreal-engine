# PubNub core (`Plugins/Pubnub`)

How to add or change a core API without breaking the threading and lifetime rules. Confirm details against the current peer. Chat rules are in [pubnub-chat.md](pubnub-chat.md). General comment and version rules are in [AGENTS.md](../../AGENTS.md).

## Surface

`UPubnubSubsystem` only creates, finds, and destroys clients (`CreatePubnubClient`, `GetPubnubClient`, `DestroyPubnubClient`). All PubNub work is on `UPubnubClient`. One client is one context: keys, user ID, crypto, and subscriptions. Several clients can exist at once.

Put a new operation on `UPubnubClient`. If that family already exists on an entity (`UPubnubChannelEntity` and the other entity types), add a thin wrapper there that passes `EntityID` into the client. Those wrappers stop at the client. `UPubnubSubscription` is the exception: its `.cpp` owns the C subscription handles. Change that lifecycle in `PubnubSubscription.cpp`, not in the channel entity.

`UPubnubSubscription::Subscribe` is the subscription API. `UPubnubClient::SubscribeToChannel` / `SubscribeToGroup` and `OnMessageReceived` are the older client-wide API. Extend the one the task is about. Do not add a third subscribe path.

Blueprint entry points are `UFUNCTION(BlueprintCallable)`. C++ also gets a native-delegate overload and, where the peer has it, an async overload with no callback. `meta = (AutoCreateRefTerm = "...")` on the Blueprint async function.

Settings, page, and result structs live in `PubnubStructLibrary.h`. Enums live in `PubnubEnumLibrary.h`. Do not reorder existing `UENUM` values. New identifiers use the `FPubnub` / `EPubnub` / `UPubnub` prefixes already in the file you are editing.

## Sync, async, and `_priv`

Blocking names have no suffix (`PublishMessage`). Non-blocking names end in `Async`.

The three public async overloads follow `UPubnubClient::PublishMessage`:

1. Blueprint dynamic delegate binds a lambda onto the native delegate and calls overload 2.
2. Native overload checks the client, queues work, calls `_priv`, then `UPubnubUtilities::CallPubnubDelegate`.
3. No-callback overload queues `_priv` and drops the result.

`_priv` is where validation and the C call live. Sync public methods must not grow a second copy of that logic.

`FPubnubFunctionThread` (`Public/Threads/PubnubFunctionThread.h`) is one queue per client, created in `InitWithConfig`. It sleeps 50 ms between batches. Do not spawn another thread for a single API.

`CallPubnubDelegate` posts onto the game thread. Subscribe listeners are not that path: copy the event in the C callback, then `AsyncTask` to the game thread before touching a `UObject` (`OnEntitySubscribeEvent`). Do not call Blueprint delegates on the C callback thread.

A sync call blocks the caller, including the game thread. From a queued lambda, call `_priv` (or the sync method, which calls `_priv`). Do not queue async-from-async.

## One REST call at a time

`PUBNUB_TRY_LOCK_MUTEX_RETURN_*` takes `PubnubOperationMutex` for the whole `_priv` call, including the wait and the result accessors. A second sync or async REST call on that client fails with "Another Pubnub operation is in progress." That failure is intentional.

The lock is held until `_priv` returns so `DeinitializeClient` can take the same mutex before freeing the context. Do not release it before `pubnub_future_release`. Do not put the macro inside a bare `if` without braces; it declares an RAII guard.

Subscribe registration uses `SubscriptionsMutex`, not this operation lock. Follow the subscribe peer you are editing.

## C call shape

`_priv` for a request/response API follows `PublishMessage_priv`:

1. `PUBNUB_LOG_FUNCTION_INPUTS_DEBUG`.
2. Empty-field and condition macros. They fill `FPubnubOperationResult` (`Error`, `ErrorMessage`, `Status`).
3. Operation mutex.
4. `FUTF8StringHolder` for every string the C call reads. The holder must stay in the function until the future is released. Use `GetOrNull()` for optional strings. A holder that dies before `pubnub_await` returns is a use-after-free.
5. `PUBNUB_*_OPTS_INIT`, then the C function that returns `pubnub_future_t operation_future`. The local must be named `operation_future`. The member pointer must be `InFlightFuture`.
6. `PUBNUB_RETURN_WRAPPER_IF_FUTURE_NOT_IN_PROGRESS` or the operation-result twin. That path releases the future and does not store it on `InFlightFuture`.
7. `PUBNUB_WRAPPER_AWAIT_FOR_FUTURE` / `PUBNUB_OPERATION_RESULT_AWAIT_FOR_FUTURE`.
8. Read result accessors only after a successful await.
9. `pubnub_future_release`, then `*InFlightFuture = MakeInvalidFuture()`, then `PUBNUB_LOG_OPERATION_RESULT`.

Non-JSON publish and signal text is passed through `UPubnubJsonUtilities::SerializeString`. JSON objects and arrays are sent unchanged.

`FPubnubOperationResult` is the status inside every `FPubnub…Result`. Success from a completed HTTP call sets `Status` to 200 and `Error` to false. A refused start sets `Error` and leaves `Status` at 0. An HTTP failure copies the response status and the response error text.

Operations that require a user ID use `PUBNUB_RETURN_*_IF_USER_ID_NOT_SET`. `FPubnubConfig::UserID` may be empty; the caller then has to `SetUserID` before those operations.

## Subscriptions

`Subscribe` / `SubscribeToChannel` succeeding means the subscription was registered locally. It does not mean the handshake finished. Connection and failure arrive later on `UPubnubClient::OnSubscriptionStatusChanged`. `FPubnubSubscriptionStatusData::Channels` and `ChannelGroups` are the active set.

`UPubnubSubscription` outlives the entity it was created from. Unsubscribe does not destroy the object. Cleanup is `BeginDestroy` plus the client's subscription registry. Do not free a C subscription pointer that another wrapper still has registered.

Listener user-data is retired and released with the context (`RetireEntityListenerUserData`). Do not `delete` it from the callback.

## Crypto, secrets, logging

`FPubnubConfig::CryptoModule` is fixed at `InitWithConfig`. Call `UPubnubCryptoModule::InitCryptoModule` first. The client borrows the module until the context is destroyed. `InitCryptoModule` refuses to run again while borrowed. Only `UPubnubAesCryptor` and `UPubnubLegacyCryptor` are accepted. At most four additional cryptors, and they are used for decrypt only.

`SecretKey` is applied only at creation and is server-only. Do not add a setter for it. Client apps use `SetAuthToken`.

Logging goes through `LoggerManager` and the `PUBNUB_LOG_*` macros in `PubnubInternalMacros.h`. Those macros require a `LoggerManager` member. `UE_LOG(PubnubLog, ...)` is for failures that happen before the logger exists (the ensure macros).

## Tests

Tests are Unreal automation tests in `PubnubLibraryTests`. They use the public API.

- Unit tests that do not need a client: `IMPLEMENT_SIMPLE_AUTOMATION_TEST`, names `Pubnub.aUnit.*`.
- Client tests: `IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST` on `FPubnubAutomationTestBase`, names `Pubnub.Integration.*`. Call `InitTest()` first and `CleanUp()` last. `InitTestWithPAM(true)` is for admin calls that need the secret key. Other PAM tests leave the secret unset and use a token.
- Network tests schedule `FDelayedFunctionLatentCommand` / `FWaitUntilLatentCommand` and return true from `RunTest`. Do not block `RunTest` on a live subscribe.
- A publish is not receivable until the handshake lists that channel or group. Use `FPubnubHandshakeTracker` and `FWaitForPubnubHandshakeCommand`. `PSS_Connected` and `PSS_SubscriptionChanged` both count.
- Keys: `PN_PUB_KEY`, `PN_SUB_KEY`, `PN_SEC_KEY`, and the `_PAM` variants. Unset keys fall back to `demo`. Channel names start with `UE_SDK_`.
- `FPubnubAutomationTestBase` suppresses log errors and warnings on purpose. Do not treat an expected SDK error log as a test failure.

`RecreateClient` exists because crypto cannot be changed on a live client.
