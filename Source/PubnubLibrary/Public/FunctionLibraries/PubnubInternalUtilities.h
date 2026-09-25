// Copyright 2026 PubNub Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
//#include "PubNub.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "UObject/GarbageCollection.h"
#include "PubnubStructLibrary.h"
#include "PubnubInternalUtilities.generated.h"

struct pubnub_publish_opts;
typedef struct pubnub_publish_opts pubnub_publish_opts_t;

struct pubnub_string_view;
typedef struct pubnub_string_view pubnub_string_view_t;

struct pubnub_context;
typedef struct pubnub_context pubnub_context_t;
struct pubnub_subscription;
typedef struct pubnub_subscription* pubnub_subscription_t;
struct pubnub_subscription_set;
typedef struct pubnub_subscription_set* pubnub_subscription_set_t;
struct pubnub_subscribe_event;
typedef struct pubnub_subscribe_event pubnub_subscribe_event_t;
struct pubnub_subscribe_listener;
typedef struct pubnub_subscribe_listener pubnub_subscribe_listener_t;
typedef uint16_t pubnub_listener_handle_t;

struct pubnub_json_value;
typedef struct pubnub_json_value pubnub_json_value_t;
struct pubnub_serialization_provider;
typedef struct pubnub_serialization_provider pubnub_serialization_provider_t;
struct pubnub_future;
typedef struct pubnub_future pubnub_future_t;

struct pubnub_uuid_metadata;
typedef struct pubnub_uuid_metadata pubnub_uuid_metadata_t;
struct pubnub_channel_metadata;
typedef struct pubnub_channel_metadata pubnub_channel_metadata_t;
struct pubnub_app_context_page;
typedef struct pubnub_app_context_page pubnub_app_context_page_t;
struct pubnub_membership;
typedef struct pubnub_membership pubnub_membership_t;
struct pubnub_member;
typedef struct pubnub_member pubnub_member_t;

struct FParsedRelationInput;
struct FParsedRelationList;
struct FMembershipInputBatch;
struct FMemberInputBatch;

class UPubnubClient;
class UPubnubSubscriptionBase;
class FJsonObject;
struct FMetadataStringField;
struct FParsedUserMetadataObject;
struct FParsedChannelMetadataObject;
struct FUTF8StringHolder;

/**
 * 
 */
UCLASS()
class PUBNUBLIBRARY_API UPubnubInternalUtilities : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:

	/* STRUCT CONVERTERS */

	
	static void PublishUESettingsToPubnubPublishOptions(const FPubnubPublishSettings &PublishSettings, pubnub_publish_opts_t &PubnubPublishOptions);
	/*
	static void HereNowUESettingsToPubnubHereNowOptions(const FPubnubListUsersFromChannelSettings &HereNowSettings, pubnub_here_now_options &PubnubHereNowOptions);
	static void SetStateUESettingsToPubnubSetStateOptions(const FPubnubSetStateSettings &SetStateSettings, pubnub_set_state_options &PubnubSetStateOptions);
	static void FetchHistoryUESettingsToPbFetchHistoryOptions(const FPubnubFetchHistorySettings &FetchHistorySettings, pubnub_fetch_history_options &PubnubFetchHistoryOptions);
	*/

	/** e.g. "Windows" / "Android" / "iOS" / "MacOS" / "Linux" for the current build target platform. */
	static FString GetOperatingSystemString();

	/** e.g. "-Pubnub-C-core/7.1.3/Unreal/2.0.2" for runtime PNSDK suffix; uses PUBNUB_C_CORE_VERSION and PUBNUB_LIBRARY_VERSION_*. */
	static FString GetPubnubSdkVersionSuffix();
	
	static FString PubnubStringViewToString(pubnub_string_view_t StringView);

	/** Converts a C-Core JSON value to FString. String nodes are copied decoded; other types are serialized. */
	static FString PubnubJsonValueToFString(pubnub_serialization_provider_t* Serial, const pubnub_json_value_t* Value);

	/** Same as PubnubJsonValueToFString, except a missing value or JSON null stays empty. */
	static FString PubnubJsonFieldToString(pubnub_serialization_provider_t* Serial, const pubnub_json_value_t* Value);

	/** Reads one string field from a JSON object. Empty when the field is missing or not a string. */
	static FString PubnubJsonObjectStringField(pubnub_serialization_provider_t* Serial, const pubnub_json_value_t* Object, const char* Key);

	/** Clamps a C-Core count into the int range used by UE result structs. */
	static int MessageCountToInt(uint32 Count);

	/**
	 * Appends history message actions from { type: { value: [ { uuid, actionTimetoken } ] } }.
	 * MessageTimetoken is copied onto each action.
	 */
	static void AppendHistoryMessageActions(pubnub_serialization_provider_t* Serial, const pubnub_json_value_t* Actions, const FString& MessageTimetoken, TArray<FPubnubMessageActionData>& OutActions);

	/** Maps a C-Core pubnub_subscribe_message_type_t value to the UE message type enum. */
	static EPubnubMessageType MessageTypeFromPubnubSubscribeEvent(int EventType);

	/** Maps a C-Core pubnub_subscribe_status_t value to the UE subscription status enum. */
	static EPubnubSubscriptionStatus SubscriptionStatusFromPubnubSubscribeStatus(int Status);

	/**
	 * Creates a C-Core subscription for an entity, then destroys the entity handle.
	 * The returned subscription holds its own registry ref and must be destroyed
	 * with pubnub_subscription_destroy when no longer needed.
	 */
	static pubnub_subscription_t CreateCCoreSubscription(pubnub_context_t* Context, const FString& EntityID, EPubnubEntityType EntityType, FPubnubSubscribeSettings Options);

	/** Creates an empty subscription set. Destroy it with pubnub_subscription_set_destroy. */
	static pubnub_subscription_set_t CreateCCoreSubscriptionSet(pubnub_context_t* Context);

	/**
	 * Creates a subscription for the entity, adds it to the set, then destroys the temporary handle.
	 * The set keeps its own registry reference.
	 */
	static bool AddEntityToCCoreSubscriptionSet(pubnub_context_t* Context, pubnub_subscription_set_t SubscriptionSet, const FString& EntityID, EPubnubEntityType EntityType, FPubnubSubscribeSettings Options);

	/**
	 * Activates a subscription. A non-empty cursor timetoken is stored with pubnub_subscribe_restore
	 * after subscribe succeeds. Cursor.Region is ignored; the new C-Core cursor is a timetoken only.
	 * On restore failure the subscription is unsubscribed again.
	 */
	static int32 ActivateCCoreSubscription(pubnub_context_t* Context, pubnub_subscription_t Subscription, const FPubnubSubscriptionCursor& Cursor);

	/** Same as ActivateCCoreSubscription for a subscription set. */
	static int32 ActivateCCoreSubscriptionSet(pubnub_context_t* Context, pubnub_subscription_set_t SubscriptionSet, const FPubnubSubscriptionCursor& Cursor);

	/** Registers data-event callbacks that forward into UPubnubSubscriptionBase::DeliverSubscribeEvent. */
	static pubnub_listener_handle_t AddEntitySubscriptionListener(pubnub_subscription_t Subscription, void* UserData);

	/** Same as AddEntitySubscriptionListener for a subscription set. */
	static pubnub_listener_handle_t AddEntitySubscriptionSetListener(pubnub_subscription_set_t SubscriptionSet, void* UserData);

	static void RemoveEntitySubscriptionListener(pubnub_subscription_t Subscription, pubnub_listener_handle_t Handle);

	static void RemoveEntitySubscriptionSetListener(pubnub_subscription_set_t SubscriptionSet, pubnub_listener_handle_t Handle);

	/** Fills Out with subscribed handles. Out is empty when the query fails. */
	static void ListActiveCCoreSubscriptions(pubnub_context_t* Context, TArray<pubnub_subscription_t>& Out);

	static void ListActiveCCoreSubscriptionSets(pubnub_context_t* Context, TArray<pubnub_subscription_set_t>& Out);

	static void ListCCoreSetSubscriptions(pubnub_subscription_set_t SubscriptionSet, TArray<pubnub_subscription_t>& Out);

	/** Copies a subscribe event into UE message data. Views are only valid during the C-Core callback. */
	static FPubnubMessageData UEMessageFromSubscribeEvent(pubnub_context_t* Context, const pubnub_subscribe_event_t* Event);

	/** C-Core data callback. Copies the event, then delivers it on the game thread. */
	static void OnEntitySubscribeEvent(const pubnub_subscribe_event_t* Event, void* UserData);

	/** Fills a listener that forwards every data event to OnEntitySubscribeEvent. */
	static void FillEntityDataListener(pubnub_subscribe_listener_t* Listener, void* UserData);

	/** Stores a resume timetoken with pubnub_subscribe_restore. */
	static int32 RestoreSubscribeTimetoken(pubnub_context_t* Context, const FString& Timetoken);

	/**
	 * Runs a C-Core list query into Out.
	 * Query is pubnub_res_t(HandleType* Buffer, size_t MaxCount, size_t* OutCount).
	 */
	template<typename HandleType, typename QueryType>
	static void ListCCoreHandles(QueryType Query, TArray<HandleType>& Out);

	/** Heap user_data for an entity subscription listener. Destroy with DestroyEntityListenerUserData. */
	static void* CreateEntityListenerUserData(UPubnubSubscriptionBase* Subscription, pubnub_context_t* Context);

	static void DestroyEntityListenerUserData(void* UserData);

	static void ClearSubscriptionDelegates(UPubnubSubscriptionBase* Subscription);

	/**
	 * Fills TotalOccupancy, TotalChannels, and Channels from a completed list-users future.
	 * String views are copied immediately. Call before pubnub_future_release.
	 * Does not modify OutResult.Result.
	 */
	static void ListUsersFromChannelFromFuture(pubnub_future_t Future, FPubnubListUsersFromChannelResult& OutResult);

	/**
	 * Fills Channels from a completed list-channels future.
	 * String views are copied immediately. Call before pubnub_future_release.
	 * Does not modify OutResult.Result.
	 */
	static void ListChannelsFromGroupFromFuture(pubnub_future_t Future, FPubnubListChannelsFromGroupResult& OutResult);

	/**
	 * Fills Channels from a completed list-user-subscribed-channels future.
	 * String views are copied immediately. Call before pubnub_future_release.
	 * Does not modify OutResult.Result.
	 */
	static void ListUserSubscribedChannelsFromFuture(pubnub_future_t Future, FPubnubListUsersSubscribedChannelsResult& OutResult);

	/**
	 * Fills States from a completed get-state future.
	 * Channel names and state JSON are copied immediately. Call before pubnub_future_release.
	 * Does not modify OutResult.Result.
	 */
	static void GetStateFromFuture(pubnub_future_t Future, FPubnubGetStateResult& OutResult);

	/**
	 * Fills Messages from a completed fetch-history future.
	 * Payload, meta, and message actions are copied immediately. Call before pubnub_future_release.
	 * Does not modify OutResult.Result.
	 */
	static void FetchHistoryFromFuture(pubnub_future_t Future, FPubnubFetchHistoryResult& OutResult);

	/**
	 * Fills MessageCounts from a completed message-counts future for one channel.
	 * Call before pubnub_future_release. Does not modify OutResult.Result.
	 */
	static void MessageCountsFromFuture(pubnub_future_t Future, const FString& Channel, FPubnubMessageCountsResult& OutResult);

	/**
	 * Fills MessageCountsPerChannel from a completed message-counts future.
	 * Requested channels are preset to 0, then overwritten by server counts.
	 * Call before pubnub_future_release. Does not modify OutResult.Result.
	 */
	static void MessageCountsMultipleFromFuture(pubnub_future_t Future, const TArray<FString>& Channels, FPubnubMessageCountsMultipleResult& OutResult);

	/**
	 * Fills MessageActionData from a completed add-message-action future.
	 * String views are copied immediately. Call before pubnub_future_release.
	 * Does not modify OutResult.Result.
	 */
	static void AddMessageActionFromFuture(pubnub_future_t Future, FPubnubAddMessageActionResult& OutResult);

	/**
	 * Fills MessageActions from a completed get-message-actions future.
	 * String views are copied immediately. Call before pubnub_future_release.
	 * Does not modify OutResult.Result.
	 * Pagination cursors from the new C-Core are not copied; FPubnubGetMessageActionsResult has no page fields.
	 */
	static void GetMessageActionsFromFuture(pubnub_future_t Future, FPubnubGetMessageActionsResult& OutResult);

	/**
	 * Maps a comma-separated App Context include list onto PUBNUB_APP_CONTEXT_INCLUDE_* bits.
	 * Total count is not part of this string; callers add PUBNUB_APP_CONTEXT_INCLUDE_TOTAL_COUNT themselves.
	 * Tokens the new C-Core does not know are copied into OutUnknownTokens and are not set in the mask.
	 */
	static uint32 AppContextIncludeMaskFromString(const FString& Include, FString& OutUnknownTokens);

	/**
	 * Fills UsersData, Page, and TotalCount from a completed get-all-uuid-metadata future.
	 * String views and custom JSON are copied immediately. Call before pubnub_future_release.
	 * Does not modify OutResult.Result.
	 */
	static void GetAllUserMetadataFromFuture(pubnub_future_t Future, FPubnubGetAllUserMetadataResult& OutResult);

	/**
	 * Fills ChannelsData, Page, and TotalCount from a completed get-all-channel-metadata future.
	 * String views and custom JSON are copied immediately. Call before pubnub_future_release.
	 * Does not modify OutResult.Result.
	 */
	static void GetAllChannelMetadataFromFuture(pubnub_future_t Future, FPubnubGetAllChannelMetadataResult& OutResult);

	/** Copies one get-uuid-metadata result. Call before pubnub_future_release. */
	static FPubnubUserData UserDataFromGetUuidMetadataFuture(pubnub_future_t Future);

	/** Copies one set-uuid-metadata result. Call before pubnub_future_release. */
	static FPubnubUserData UserDataFromSetUuidMetadataFuture(pubnub_future_t Future);

	/** Copies one get-channel-metadata result. Call before pubnub_future_release. */
	static FPubnubChannelData ChannelDataFromGetChannelMetadataFuture(pubnub_future_t Future);

	/** Copies one set-channel-metadata result. Call before pubnub_future_release. */
	static FPubnubChannelData ChannelDataFromSetChannelMetadataFuture(pubnub_future_t Future);

	/** Include mask for a list call. PT_True adds PUBNUB_APP_CONTEXT_INCLUDE_TOTAL_COUNT. */
	static uint32 MetadataListIncludeMask(const FString& Include, EPubnubTribool Count, FString& OutUnknownTokens);

	/**
	 * Splits a set-user JSON object into C-Core fields.
	 * Returns false and sets Out.Error when the JSON is not an object or a field has the wrong type.
	 */
	static bool ParseUserMetadataObject(const FString& Json, FParsedUserMetadataObject& Out);

	/** Splits a set-channel JSON object into C-Core fields. Same failure rules as ParseUserMetadataObject. */
	static bool ParseChannelMetadataObject(const FString& Json, FParsedChannelMetadataObject& Out);

	/** Error text for string fields that were JSON null. The new C-Core set API cannot send those. */
	static FString MetadataNullFieldError(const TArray<FString>& Fields);

	/** Holder pointer when the field was present, otherwise nullptr so C-Core omits it. */
	static const char* OptionalMetadataString(const FMetadataStringField& Field, const FUTF8StringHolder& Holder);

	/**
	 * Fills MembershipsData, Page, and TotalCount from a completed get-memberships future.
	 * String views and custom JSON are copied immediately. Call before pubnub_future_release.
	 * Does not modify OutResult.Result.
	 */
	static void GetMembershipsFromFuture(pubnub_future_t Future, FPubnubMembershipsResult& OutResult);

	/** Same as GetMembershipsFromFuture for a completed set-memberships future. */
	static void SetMembershipsFromFuture(pubnub_future_t Future, FPubnubMembershipsResult& OutResult);

	/**
	 * Fills MembersData, Page, and TotalCount from a completed get-channel-members future.
	 * String views and custom JSON are copied immediately. Call before pubnub_future_release.
	 * Does not modify OutResult.Result.
	 */
	static void GetChannelMembersFromFuture(pubnub_future_t Future, FPubnubChannelMembersResult& OutResult);

	/** Same as GetChannelMembersFromFuture for a completed set-channel-members future. */
	static void SetChannelMembersFromFuture(pubnub_future_t Future, FPubnubChannelMembersResult& OutResult);

	/**
	 * Parses a SetMemberships JSON array into C-Core membership inputs.
	 * Returns false and sets Out.Error when the JSON is not an array or an item has the wrong type.
	 */
	static bool ParseMembershipSetList(const FString& Json, FParsedRelationList& Out);

	/** Parses a RemoveMemberships JSON array. Only channel.id is kept. */
	static bool ParseMembershipRemoveList(const FString& Json, FParsedRelationList& Out);

	/**
	 * Parses a SetChannelMembers JSON array into C-Core member inputs.
	 * Returns false and sets Out.Error when the JSON is not an array or an item has the wrong type.
	 */
	static bool ParseMemberSetList(const FString& Json, FParsedRelationList& Out);

	/** Parses a RemoveChannelMembers JSON array. Only uuid.id is kept. */
	static bool ParseMemberRemoveList(const FString& Json, FParsedRelationList& Out);

	/** Copies parsed membership items into borrowed C-Core inputs. Out owns the UTF-8 bytes. */
	static void BuildMembershipInputs(const FParsedRelationList& Parsed, FMembershipInputBatch& Out);

	/** Copies parsed member items into borrowed C-Core inputs. Out owns the UTF-8 bytes. */
	static void BuildMemberInputs(const FParsedRelationList& Parsed, FMemberInputBatch& Out);

	/* TEMPLATES */

	/**
	 * Thread-safe wrapper around NewObject<T>.
	 *
	 * FGCScopeGuard takes Unreal's GC async lock, which prevents GC from starting while the
	 * guard is alive (and waits for an in-progress GC to finish before returning from its
	 * constructor). Holding it across the NewObject call.
	 */
	template<typename ObjectType, typename OuterType>
	static ObjectType* SafeNewObject(OuterType* Outer)
	{
		FGCScopeGuard GCGuard;
		return NewObject<ObjectType>(Outer);
	}

private:

	static bool ReadOptionalMetadataString(const TSharedPtr<FJsonObject>& Object, const TCHAR* Key, FMetadataStringField& Out, TArray<FString>& NullFields, FString& Error);
	static bool ReadMetadataCustom(const TSharedPtr<FJsonObject>& Object, FString& OutJson, bool& bHasCustom, FString& Error);
	static void CollectUnknownMetadataFields(const TSharedPtr<FJsonObject>& Object, const TArray<FString>& KnownKeys, TArray<FString>& OutUnknown);
	static FString AppContextJsonObjectToString(pubnub_serialization_provider_t* Serial, const pubnub_json_value_t* Value);
	static FPubnubUserData UserDataFromUuidMetadata(pubnub_serialization_provider_t* Serial, const pubnub_uuid_metadata_t& Metadata);
	static FPubnubChannelData ChannelDataFromChannelMetadata(pubnub_serialization_provider_t* Serial, const pubnub_channel_metadata_t& Metadata);
	static void CopyAppContextPage(const pubnub_app_context_page_t& Page, FPubnubPage& OutPage, int& OutTotalCount);
	static FPubnubMembershipData MembershipDataFromMembership(pubnub_serialization_provider_t* Serial, const pubnub_membership_t& Membership);
	static FPubnubChannelMemberData ChannelMemberDataFromMember(pubnub_serialization_provider_t* Serial, const pubnub_member_t& Member);
	static bool ParseRelationSetList(const FString& Json, const TCHAR* IdObjectKey, const TCHAR* JsonName, FParsedRelationList& Out);
	static bool ParseRelationRemoveList(const FString& Json, const TCHAR* IdObjectKey, const TCHAR* JsonName, FParsedRelationList& Out);
};
