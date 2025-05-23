// Copyright 2024 PubNub Inc. All Rights Reserved.

#pragma once

#include "PubnubSubsystem.h"
#include "CoreMinimal.h"
#include "PubnubStructLibrary.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "PubnubUtilities.generated.h"


/**
 * 
 */
UCLASS()
class PUBNUBLIBRARY_API UPubnubUtilities : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:

	static FString AddQuotesToString(const FString InString, bool SkipIfHasQuotes = true);
	static FString RemoveOuterQuotesFromString(const FString InString);
	static FString PubnubCharMemBlockToString(const pubnub_char_mem_block PnChar);
	static FString MembershipIncludeToString(const FPubnubMembershipInclude& MembershipInclude);
	static FString MemberIncludeToString(const FPubnubMemberInclude& MemberInclude);
	static FString GetAllIncludeToString(const FPubnubGetAllInclude& GetAllInclude);
	static FString MembershipSortTypeToString(const EPubnubMembershipSortType SortType);
	static FString MemberSortTypeToString(const EPubnubMemberSortType SortType);
	static FString GetAllSortTypeToString(const EPubnubGetAllSortType SortType);
	static FString MembershipSortToString(const FPubnubMembershipSort& MemberInclude);
	static FString MemberSortToString(const FPubnubMemberSort& MemberInclude);
	static FString GetAllSortToString(const FPubnubGetAllSort& GetAllInclude);
	
	/* C-CORE EVENT ENGINE HELPERS */

	static pubnub_subscription_t* EEGetSubscriptionForChannel(pubnub_t* Context, FString Channel, FPubnubSubscribeSettings Options);
	static pubnub_subscription_t* EEGetSubscriptionForChannelGroup(pubnub_t* Context, FString ChannelGroup, FPubnubSubscribeSettings Options);
	static bool EEAddListenerAndSubscribe(pubnub_subscription_t* Subscription, pubnub_subscribe_message_callback_t Callback, UPubnubSubsystem* PubnubSubsystem);
	static bool EERemoveListenerAndUnsubscribe(pubnub_subscription_t** SubscriptionPtr, pubnub_subscribe_message_callback_t Callback, UPubnubSubsystem* PubnubSubsystem);
};
