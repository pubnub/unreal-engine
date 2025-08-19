// Copyright 2024 PubNub Inc. All Rights Reserved.

#pragma once

#include "PubnubSubsystem.h"
#include "CoreMinimal.h"
#include "Async/Async.h"
#include "PubnubStructLibrary.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "PubnubUtilities.generated.h"


class UPubnubSettings;

/** This struct is an utility for more convenient converting FString into const char* while keeping char memory alive
 * Will not break UTF8 characters.
 * NOTE:: if the struct goes out of scope, the memory pointed by const char* from Get() will do as well.
 */
struct FUTF8StringHolder
{
	FTCHARToUTF8 Converter;

	FUTF8StringHolder(const FString& Input)
		: Converter(*Input)
	{
	}

	const char* Get() const
	{
		return Converter.Get();
	}
};

/**
 * 
 */
UCLASS()
class PUBNUBLIBRARY_API UPubnubUtilities : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:

	static FPubnubConfig PubnubConfigFromPluginSettings(UPubnubSettings* PubnubSettings);
	static FString PubnubGetLastServerHttpResponse(pubnub_t* Context);

	//Sets limit to be between 0 and PUBNUB_MAX_LIMIT. Prevents C-Core crash on providing incorrect limit.
	static int RoundLimitForPubnubFunctions(int ProvidedLimit);
	
	/* STRING OPERATIONS */
	
	static FString AddQuotesToString(const FString InString, bool SkipIfHasQuotes = true);
	static FString RemoveOuterQuotesFromString(const FString InString);
	static FString PubnubCharMemBlockToString(const pubnub_char_mem_block PnChar);

	/**
	 * Safely copies an FString to a char buffer with proper bounds checking and null termination.
	 * @param Destination The destination char buffer
	 * @param DestSize The size of the destination buffer (including space for null terminator)
	 * @param Source The source FString to copy
	 * @param KeyName Optional name for logging purposes (defaults to "String")
	 * @return true if copy was successful, false if conversion failed
	 */
	static bool SafeCopyFStringToCharBuffer(char* Destination, int DestSize, const FString& Source, const TCHAR* KeyName = TEXT("String"));

	//This is to remove class name and "_priv" from __FUNCTION__ macro output
	static FString GetNameFromFunctionMacro(FString FunctionName);

	/* CONVERTING INCLUDES */
	
	static FString MembershipIncludeToString(const FPubnubMembershipInclude& MembershipInclude);
	static FString MemberIncludeToString(const FPubnubMemberInclude& MemberInclude);
	static FString GetAllIncludeToString(const FPubnubGetAllInclude& GetAllInclude);
	static FString GetMetadataIncludeToString(const FPubnubGetMetadataInclude& GetMetadataInclude);

	/* CONVERTING SORTS */
	
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


	/* TEMPLATES TO CALL PUBNUB DELEGATES */

	//Template to call any Delegate in case of providing incorrect parameters. Provide error message and FPubnubOperationResult will be made out of it
	template<typename DelegateType, typename... Args>
	static void CallPubnubDelegateWithInvalidArgumentResult(const DelegateType& Delegate, const FString &ErrorMessage,  Args&&... args)
	{
		CallPubnubDelegate(Delegate, FPubnubOperationResult{0, false, ErrorMessage}, std::forward<Args>(args)...);
	}

	//Main template to execute Pubnub delegate as an AsyncTask on the GameThread. Make sure to provide all required parameters for the delegate.
	template<typename DelegateType, typename... Args>
	static void CallPubnubDelegate(const DelegateType& Delegate, Args&&... args)
	{
		if (Delegate.IsBound())
		{
			auto CopiedDelegate = Delegate;
			auto ArgsTuple = std::make_tuple(std::forward<Args>(args)...);

			// Launch the async task, all such delegates should be called on GameThread to work well with widgets and other systems
			AsyncTask(ENamedThreads::GameThread, [CopiedDelegate, ArgsTuple = std::move(ArgsTuple)]() mutable
			{
				CallDelegateWithTuple(CopiedDelegate, std::move(ArgsTuple),
									  std::make_index_sequence<sizeof...(Args)>{});
			});
		}
	}

	// Helper to unpack tuple and call Delegate.Execute(...)
	template<typename DelegateType, typename TupleType, size_t... Indices>
	static void CallDelegateWithTuple(DelegateType& Delegate, TupleType&& Tuple, std::index_sequence<Indices...>)
	{
		Delegate.Execute(std::get<Indices>(std::forward<TupleType>(Tuple))...);
	}
};
