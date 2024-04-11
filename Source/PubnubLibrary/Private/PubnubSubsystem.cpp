// Copyright Epic Games, Inc. All Rights Reserved.

#include "PubnubSubsystem.h"
#include "Config/PubnubSettings.h"
#include "Threads/PubnubFunctionThread.h"
#include "Threads/PubnubLoopingThread.h"

DEFINE_LOG_CATEGORY_STATIC(PubnubLog, Log, All)

void UPubnubSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	//Create new threads - separate for subscribe and all other operations
	QuickActionThread = new FPubnubFunctionThread;
	LongpollThread = new FPubnubLoopingThread;

	//Load all settings from plugin config
	LoadPluginSettings();
	if(PubnubSettings->InitializeAutomatically)
	{
		InitPubnub();
	}
}

void UPubnubSubsystem::Deinitialize()
{
	DeinitPubnub();
	Super::Deinitialize();
	
	if(QuickActionThread)
	{
		QuickActionThread->Stop();
	}
	if(LongpollThread)
	{
		LongpollThread->Stop();
	}
}

void UPubnubSubsystem::InitPubnub()
{
	if(!CheckQuickActionThreadValidity())
	{return;}

	QuickActionThread->AddFunctionToQueue( [this]
	{
		InitPubnub_priv();
	});
}

void UPubnubSubsystem::DeinitPubnub()
{
	if(!CheckQuickActionThreadValidity())
	{return;}
	
	QuickActionThread->AddFunctionToQueue( [this]
	{
		DeinitPubnub_priv();
	});
}

void UPubnubSubsystem::SetUserID(FString UserID)
{
	if(!CheckQuickActionThreadValidity())
	{return;}
	
	QuickActionThread->AddFunctionToQueue( [this, UserID]
	{
		SetUserID_priv(UserID);
	});
}

void UPubnubSubsystem::SetSecretKey()
{
	if(!CheckQuickActionThreadValidity())
	{return;}
	
	QuickActionThread->AddFunctionToQueue( [this]
	{
		SetSecretKey_priv();
	});
}

void UPubnubSubsystem::PublishMessage(FString ChannelName, FString Message, FPubnubPublishSettings PublishSettings)
{
	if(!CheckQuickActionThreadValidity())
	{return;}
	
	QuickActionThread->AddFunctionToQueue( [this, ChannelName, Message, PublishSettings]
	{
		PublishMessage_priv(ChannelName, Message, PublishSettings);
	});
}

void UPubnubSubsystem::Signal(FString ChannelName, FString Message)
{
	if(!CheckQuickActionThreadValidity())
	{return;}

	QuickActionThread->AddFunctionToQueue( [this, ChannelName, Message]
	{
		PublishMessage_priv(ChannelName, Message);
	});
}

void UPubnubSubsystem::SubscribeToChannel(FString ChannelName)
{
	if(!CheckQuickActionThreadValidity())
	{return;}
	
	QuickActionThread->AddFunctionToQueue( [this, ChannelName]
	{
		SubscribeToChannel_priv(ChannelName);
	});
}

void UPubnubSubsystem::SubscribeToGroup(FString GroupName)
{
	if(!CheckQuickActionThreadValidity())
	{return;}
	
	QuickActionThread->AddFunctionToQueue( [this, GroupName]
	{
		SubscribeToGroup_priv(GroupName);
	});
}

void UPubnubSubsystem::UnsubscribeFromChannel(FString ChannelName)
{
	if(!CheckQuickActionThreadValidity())
	{return;}
	
	QuickActionThread->AddFunctionToQueue( [this, ChannelName]
	{
		UnsubscribeFromChannel_priv(ChannelName);
	});
}

void UPubnubSubsystem::UnsubscribeFromGroup(FString GroupName)
{
	if(!CheckQuickActionThreadValidity())
	{return;}
	
	QuickActionThread->AddFunctionToQueue( [this, GroupName]
	{
		UnsubscribeFromGroup_priv(GroupName);
	});
}

void UPubnubSubsystem::UnsubscribeFromAll()
{
	if(!CheckQuickActionThreadValidity())
	{return;}
	
	QuickActionThread->AddFunctionToQueue( [this]
	{
		UnsubscribeFromAll_priv();
	});
}

void UPubnubSubsystem::AddChannelToGroup(FString ChannelName, FString ChannelGroup)
{
	if(!CheckQuickActionThreadValidity())
	{return;}
	
	QuickActionThread->AddFunctionToQueue( [this, ChannelName, ChannelGroup]
	{
		AddChannelToGroup_priv(ChannelName, ChannelGroup);
	});
}

void UPubnubSubsystem::RemoveChannelFromGroup(FString ChannelName, FString ChannelGroup)
{
	if(!CheckQuickActionThreadValidity())
	{return;}
	
	QuickActionThread->AddFunctionToQueue( [this, ChannelName, ChannelGroup]
	{
		RemoveChannelFromGroup_priv(ChannelName, ChannelGroup);
	});
}

void UPubnubSubsystem::ListChannelsFromGroup(FString ChannelGroup, FOnListChannelsFromGroupResponse OnListChannelsResponse)
{
	if(!CheckQuickActionThreadValidity())
	{return;}
	
	QuickActionThread->AddFunctionToQueue( [this, ChannelGroup, OnListChannelsResponse]
	{
		ListChannelsFromGroup_priv(ChannelGroup, OnListChannelsResponse);
	});
}

void UPubnubSubsystem::RemoveChannelGroup(FString ChannelGroup)
{
	if(!CheckQuickActionThreadValidity())
	{return;}
	
	QuickActionThread->AddFunctionToQueue( [this, ChannelGroup]
	{
		RemoveChannelGroup_priv(ChannelGroup);
	});
}

void UPubnubSubsystem::HereNow(FString ChannelName, FOnHereNowResponse HereNowResponse, FPubnubHereNowSettings HereNowSettings)
{
	if(!CheckQuickActionThreadValidity())
	{return;}
	
	QuickActionThread->AddFunctionToQueue( [this, ChannelName, HereNowResponse, HereNowSettings]
	{
		HereNow_priv(ChannelName, HereNowResponse, HereNowSettings);
	});
}

void UPubnubSubsystem::WhereNow(FString UserID, FOnWhereNowResponse WhereNowResponse)
{
	if(!CheckQuickActionThreadValidity())
	{return;}
	
	QuickActionThread->AddFunctionToQueue( [this, UserID, WhereNowResponse]
	{
		WhereNow_priv(UserID, WhereNowResponse);
	});
}

void UPubnubSubsystem::SetState(FString ChannelName, FString StateJson, FPubnubSetStateSettings SetStateSettings)
{
	if(!CheckQuickActionThreadValidity())
	{return;}
	
	QuickActionThread->AddFunctionToQueue( [this, ChannelName, StateJson, SetStateSettings]
	{
		SetState_priv(ChannelName, StateJson, SetStateSettings);
	});
}

void UPubnubSubsystem::GetState(FString ChannelName, FString ChannelGroup, FString UserID, FOnGetStateResponse OnGetStateResponse)
{
	if(!CheckQuickActionThreadValidity())
	{return;}
	
	QuickActionThread->AddFunctionToQueue( [this, ChannelName, ChannelGroup, UserID, OnGetStateResponse]
	{
		GetState_priv(ChannelName, ChannelGroup, UserID, OnGetStateResponse);
	});
}

void UPubnubSubsystem::Heartbeat(FString ChannelName, FString ChannelGroup)
{
	if(!CheckQuickActionThreadValidity())
	{return;}
	
	QuickActionThread->AddFunctionToQueue( [this, ChannelName, ChannelGroup]
	{
		Heartbeat_priv(ChannelName, ChannelGroup);
	});
}

void UPubnubSubsystem::GrantToken(int TTLMinutes, FString AuthorizedUUID, FOnGrantTokenResponse OnGrantTokenResponse)
{
	if(!CheckQuickActionThreadValidity())
	{return;}
	
	QuickActionThread->AddFunctionToQueue( [this, TTLMinutes, AuthorizedUUID, OnGrantTokenResponse]
	{
		GrantToken_priv(TTLMinutes, AuthorizedUUID, OnGrantTokenResponse);
	});
}

void UPubnubSubsystem::RevokeToken(FString Token)
{
	if(!CheckQuickActionThreadValidity())
	{return;}
	
	QuickActionThread->AddFunctionToQueue( [this, Token]
	{
		RevokeToken_priv(Token);
	});
}

void UPubnubSubsystem::ParseToken(FString Token)
{
	if(!CheckQuickActionThreadValidity())
	{return;}
	
	QuickActionThread->AddFunctionToQueue( [this, Token]
	{
		ParseToken_priv(Token);
	});
}

void UPubnubSubsystem::SetAuthToken(FString Token)
{
	if(!CheckQuickActionThreadValidity())
	{return;}
	
	QuickActionThread->AddFunctionToQueue( [this, Token]
	{
		SetAuthToken_priv(Token);
	});
}

void UPubnubSubsystem::History(FString ChannelName, FOnHistoryResponse OnHistoryResponse, FPubnubHistorySettings HistorySettings)
{
	if(!CheckQuickActionThreadValidity())
	{return;}
	
	QuickActionThread->AddFunctionToQueue( [this, ChannelName, OnHistoryResponse, HistorySettings]
	{
		History_priv(ChannelName, OnHistoryResponse, HistorySettings);
	});
}

void UPubnubSubsystem::MessageCounts(FString ChannelName, FDateTime TimeStamp, FOnMessageCountsResponse OnMessageCountsResponse)
{
	if(!CheckQuickActionThreadValidity())
	{return;}
	
	QuickActionThread->AddFunctionToQueue( [this, ChannelName, TimeStamp, OnMessageCountsResponse]
	{
		MessageCounts_priv(ChannelName, TimeStamp, OnMessageCountsResponse);
	});
	
}

void UPubnubSubsystem::SystemPublish()
{
	if(SubscribedChannels.IsEmpty() && SubscribedGroups.IsEmpty())
	{return;}

	//TODO: this will not unlock context if user is subscribed only to groups, but not to any channels. This issue needs to be addressed.
	PublishMessage(SubscribedChannels[0], "{\"system\":\"subscribe unlock message\"}");
}

void UPubnubSubsystem::StartPubnubSubscribeLoop()
{
	if(!LongpollThread)
	{return;}

	LongpollThread->AddLoopingFunction([this]
	{
		if(SubscribedChannels.IsEmpty() && SubscribedGroups.IsEmpty())
		{return;}

		UE_LOG(PubnubLog, Warning, TEXT("Pubnub subscribe"));
		//Subscribe to channels - this is blocking function
		pubnub_subscribe(ctx_sub, TCHAR_TO_ANSI(*StringArrayToCommaSeparated(SubscribedChannels)), TCHAR_TO_ANSI(*StringArrayToCommaSeparated(SubscribedGroups)));

		//TODO: Make sure it works and maybe come up with more sophisticated check
		//If this happens after closing program just return
		if(!this)
		{return;}

		//Check for subscribe result
		pubnub_res SubscribeResult = pubnub_await(ctx_sub);
		if (SubscribeResult != PNR_OK)
		{
			FString ErrorCode(pubnub_res_2_string(SubscribeResult));
			UE_LOG(PubnubLog, Error, TEXT("Failed to subscribe, error: %s"), *ErrorCode);
			{return;}
		}

		//At this stage we received messages, so read them and get channel from where they were sent
		const char* MessageChar = pubnub_get(ctx_sub);
		const char* ChannelChar = pubnub_get_channel(ctx_sub);
		while(MessageChar != NULL)
		{
			FString Message(MessageChar);
			FString Channel(ChannelChar);

			//Broadcast callback with message content
			//Message needs to be called back on Game Thread
			AsyncTask(ENamedThreads::GameThread, [this, Message, Channel]()
			{
				OnMessageReceived.Broadcast(Message, Channel);
			});
				
			MessageChar = pubnub_get(ctx_sub);
			ChannelChar = pubnub_get_channel(ctx_sub);
		}
	});
}

FString UPubnubSubsystem::StringArrayToCommaSeparated(TArray<FString> StringArray)
{
	FString CommaSeparatedString;
	for(FString StringElement : SubscribedChannels)
	{
		if(CommaSeparatedString.IsEmpty())
		{
			CommaSeparatedString.Append(StringElement);
		}
		else
		{
			CommaSeparatedString.Append(",");
			CommaSeparatedString.Append(StringElement);
		}
	}
	return CommaSeparatedString;
}

FString UPubnubSubsystem::GetLastResponse(pubnub_t* context)
{
	FString Response;
	
	if(!context)
	{return Response;}
	
	pubnub_res PubnubResponse = pubnub_await(context);
	if (PNR_OK == PubnubResponse) {
		
		Response = pubnub_get(context);
	}
	else
	{
		UE_LOG(PubnubLog, Error, TEXT("Failed to get last response. Error code: %d"), PubnubResponse);
	}
	return Response;
}

FString UPubnubSubsystem::GetLastChannelResponse(pubnub_t* context)
{
	FString Response;
	
	if(!context)
	{return Response;}
	
	pubnub_res PubnubResponse = pubnub_await(context);
	if (PNR_OK == PubnubResponse) {
		
		Response = pubnub_get_channel(context);
	}
	else
	{
		UE_LOG(PubnubLog, Error, TEXT("Failed to get last channel response. Error code: %d"), PubnubResponse);
	}
	return Response;
}

void UPubnubSubsystem::LoadPluginSettings()
{
	//Save all settings
	PubnubSettings = GetMutableDefault<UPubnubSettings>();

	//TODO: Make something better for reading keys - at least check size instead of hard-coding it.
	//Copy memory for chars containing keys
	memcpy_s(PublishKey, 42, TCHAR_TO_ANSI(*PubnubSettings->PublishKey), 42);
	memcpy_s(SubscribeKey,42,  TCHAR_TO_ANSI(*PubnubSettings->SubscribeKey), 42);
	memcpy_s(SecretKey,54,  TCHAR_TO_ANSI(*PubnubSettings->SecretKey), 54);
	PublishKey[42] = '\0';
	SubscribeKey[42] = '\0';
	SecretKey[54] = '\0';
}

//This functions is a wrapper to IsInitialized bool, so it can print error if user is trying to do anything before initializing Pubnub correctly
bool UPubnubSubsystem::CheckIsPubnubInitialized()
{
	if(!IsInitialized)
	{
		UE_LOG(PubnubLog, Error, TEXT("Pubnub is not initialized. Aborting operation"));
	}
	
	return IsInitialized;
}

//This functions is a wrapper to IsUserIDSet bool, so it can print error if user is trying to do anything before setting user ID correctly
bool UPubnubSubsystem::CheckIsUserIDSet()
{
	if(!IsUserIDSet)
	{
		UE_LOG(PubnubLog, Error, TEXT("Pubnub user ID is not set. Aborting operation"));
	}
	
	return IsUserIDSet;
}

bool UPubnubSubsystem::CheckQuickActionThreadValidity()
{
	if(!QuickActionThread)
	{
		UE_LOG(PubnubLog, Error, TEXT("PublishThread is invalid. Aborting operation"));
		return false;
	}
	
	return true;
}


/* PRIV FUNCTIONS */

void UPubnubSubsystem::InitPubnub_priv()
{
	if(IsInitialized)
	{return;}

	//TODO::Not only check if keys are empty, but also make sure they are valid
	//Make sure that keys are filled
	if(std::strlen(PublishKey) == 0 )
	{
		UE_LOG(PubnubLog, Error, TEXT("Publish key is empty, can't initialize Pubnub"));
	}

	if(std::strlen(SubscribeKey) == 0 )
	{
		UE_LOG(PubnubLog, Error, TEXT("Subscribe key is empty, can't initialize Pubnub"));
	}
	
	ctx_pub = pubnub_alloc();
	ctx_sub = pubnub_alloc();

	pubnub_init(ctx_pub, PublishKey, SubscribeKey);
	pubnub_init(ctx_sub, PublishKey, SubscribeKey);

	if(PubnubSettings->SetSecretKetAutomatically)
	{
		SetSecretKey();
	}

	IsInitialized = true;
}

void UPubnubSubsystem::DeinitPubnub_priv()
{
	if(!IsInitialized)
	{return;}

	//Unsubscribe from all channels so this user will not be visible for others anymore
	UnsubscribeFromAll();
	
	if(ctx_pub)
	{
		pubnub_free(ctx_pub);
		ctx_pub = nullptr;
	}
	if(ctx_sub)
	{
		pubnub_free(ctx_sub);
		ctx_sub = nullptr;
	}
	IsInitialized = false;
}

void UPubnubSubsystem::SetUserID_priv(FString UserID)
{
	if(!CheckIsPubnubInitialized())
	{return;}

	pubnub_set_user_id(ctx_pub, TCHAR_TO_ANSI(*UserID));
	pubnub_set_user_id(ctx_sub, TCHAR_TO_ANSI(*UserID));

	IsUserIDSet = true;
}

void UPubnubSubsystem::SetSecretKey_priv()
{
	if(!CheckIsPubnubInitialized())
	{return;}

	if(std::strlen(SecretKey) == 0)
	{
		UE_LOG(PubnubLog, Warning, TEXT("Can't set Secret Key. Secret Key is empty."));
		return;
	}

	pubnub_set_secret_key(ctx_pub, SecretKey);
	pubnub_set_secret_key(ctx_sub, SecretKey);
}

void UPubnubSubsystem::PublishMessage_priv(FString ChannelName, FString Message, FPubnubPublishSettings PublishSettings)
{
	if(!CheckIsPubnubInitialized() || !CheckIsUserIDSet())
	{return;}

	if(ChannelName.IsEmpty() || Message.IsEmpty())
	{return;}

	//Set all options from PublishSettings
	pubnub_publish_options PublishOptions = pubnub_publish_defopts();
	PublishOptions.store = PublishSettings.StoreInHistory;
	PublishOptions.replicate = PublishSettings.Replicate;
	
	auto CharConverter = StringCast<ANSICHAR>(*PublishSettings.MetaData);
	const char* MetaDataChar = CharConverter.Get();
	
	if(!PublishSettings.MetaData.IsEmpty())
	{
		PublishOptions.meta = MetaDataChar;
	}

	//Convert UE enum to pubnub_method enum
	PublishOptions.method = (pubnub_method)(uint8)PublishSettings.PublishMethod;
	
	pubnub_publish_ex(ctx_pub, TCHAR_TO_ANSI(*ChannelName), TCHAR_TO_ANSI(*Message), PublishOptions);
	pubnub_res PublishResult = pubnub_await(ctx_pub);

	if(PublishResult != PNR_OK)
	{
		UE_LOG(PubnubLog, Error, TEXT("Publish failed. Error code: %d"), PublishResult);
	}
}

void UPubnubSubsystem::Signal_priv(FString ChannelName, FString Message)
{
	if(!CheckIsPubnubInitialized() || !CheckIsUserIDSet())
	{return;}

	if(ChannelName.IsEmpty() || Message.IsEmpty())
	{return;}

	pubnub_signal(ctx_pub, TCHAR_TO_ANSI(*ChannelName), TCHAR_TO_ANSI(*Message));
}

void UPubnubSubsystem::SubscribeToChannel_priv(FString ChannelName)
{
	if(!CheckIsPubnubInitialized() || !CheckIsUserIDSet())
	{return;}

	if(ChannelName.IsEmpty())
	{return;}

	if(SubscribedChannels.Contains(ChannelName))
	{return;}

	//Check if Pubnub was already subscribed to a channel or a group.
	bool WasCheckingMessages = !SubscribedChannels.IsEmpty() || !SubscribedGroups.IsEmpty();
	
	SubscribedChannels.Add(ChannelName);
	
	if(!WasCheckingMessages)
	{
		StartPubnubSubscribeLoop();
	}

	//System publish to unlock subscribe context and start listening for this new channel
	SystemPublish();
}

void UPubnubSubsystem::SubscribeToGroup_priv(FString GroupName)
{
	if(!CheckIsPubnubInitialized() || !CheckIsUserIDSet())
	{return;}

	if(GroupName.IsEmpty())
	{return;}

	if(SubscribedGroups.Contains(GroupName))
	{return;}

	//Check if Pubnub was already subscribed to a channel or a group.
	bool WasCheckingMessages = !SubscribedChannels.IsEmpty() || !SubscribedGroups.IsEmpty();
	
	SubscribedGroups.Add(GroupName);

	if(WasCheckingMessages)
	{
		StartPubnubSubscribeLoop();
	}

	//System publish to unlock subscribe context and start listening for this new group
	SystemPublish();
}

void UPubnubSubsystem::UnsubscribeFromChannel_priv(FString ChannelName)
{
	if(!CheckIsPubnubInitialized() || !CheckIsUserIDSet())
	{return;}
	
	//make sure user was subscribed to that channel
	if(SubscribedChannels.Remove(ChannelName) == 0)
	{return;}

	pubnub_leave(ctx_pub, TCHAR_TO_ANSI(*ChannelName), NULL);
}

void UPubnubSubsystem::UnsubscribeFromGroup_priv(FString GroupName)
{
	if(!CheckIsPubnubInitialized() || !CheckIsUserIDSet())
	{return;}
	
	//make sure user was subscribed to that channel
	if(SubscribedGroups.Remove(GroupName) == 0)
	{return;}

	pubnub_leave(ctx_pub, NULL, TCHAR_TO_ANSI(*GroupName));
}

void UPubnubSubsystem::UnsubscribeFromAll_priv()
{
	if(!CheckIsPubnubInitialized() || !CheckIsUserIDSet())
	{return;}

	//Cache and clear all groups and channels
	TArray<FString> SubscribedChannelsCached = SubscribedChannels;
	TArray<FString> SubscribedGroupsCached = SubscribedGroups;
	SubscribedChannels.Empty();
	SubscribedGroups.Empty();
	
	pubnub_cancel(ctx_pub);
	pubnub_await(ctx_pub);
	
	//TODO: Find out how to unsubscribe from all channels correctly
	for(FString Channel : SubscribedChannelsCached)
	{
		pubnub_leave(ctx_pub, TCHAR_TO_ANSI(*Channel), NULL);
		pubnub_await(ctx_pub);
	}

	for(FString Group : SubscribedGroupsCached)
	{
		pubnub_leave(ctx_pub, NULL, TCHAR_TO_ANSI(*Group));
		pubnub_await(ctx_pub);
	}
	
	LongpollThread->ClearLoopingFunctions();
}

void UPubnubSubsystem::AddChannelToGroup_priv(FString ChannelName, FString ChannelGroup)
{
	if(!CheckIsPubnubInitialized() || !CheckIsUserIDSet())
	{return;}

	if(ChannelName.IsEmpty() || ChannelGroup.IsEmpty())
	{return;}

	pubnub_add_channel_to_group(ctx_pub, TCHAR_TO_ANSI(*ChannelName), TCHAR_TO_ANSI(*ChannelGroup));
}

void UPubnubSubsystem::RemoveChannelFromGroup_priv(FString ChannelName, FString ChannelGroup)
{
	if(!CheckIsPubnubInitialized() || !CheckIsUserIDSet())
	{return;}

	if(ChannelName.IsEmpty() || ChannelGroup.IsEmpty())
	{return;}

	pubnub_remove_channel_from_group(ctx_pub, TCHAR_TO_ANSI(*ChannelName), TCHAR_TO_ANSI(*ChannelGroup));
}

void UPubnubSubsystem::ListChannelsFromGroup_priv(FString ChannelGroup,
	FOnListChannelsFromGroupResponse OnListChannelsResponse)
{
	if(!CheckIsPubnubInitialized() || !CheckIsUserIDSet())
	{return;}

	if(ChannelGroup.IsEmpty())
	{return;}

	pubnub_list_channel_group(ctx_pub, TCHAR_TO_ANSI(*ChannelGroup));
	FString JsonResponse = GetLastChannelResponse(ctx_pub);


	//Delegate needs to be executed back on Game Thread
	AsyncTask(ENamedThreads::GameThread, [this, OnListChannelsResponse, JsonResponse]()
	{
		//Broadcast bound delegate with JsonResponse
		OnListChannelsResponse.ExecuteIfBound(JsonResponse);
	});
}

void UPubnubSubsystem::RemoveChannelGroup_priv(FString ChannelGroup)
{
	if(!CheckIsPubnubInitialized() || !CheckIsUserIDSet())
	{return;}

	if(ChannelGroup.IsEmpty())
	{return;}

	pubnub_remove_channel_group(ctx_pub, TCHAR_TO_ANSI(*ChannelGroup));
}

void UPubnubSubsystem::HereNow_priv(FString ChannelName, FOnHereNowResponse HereNowResponse, FPubnubHereNowSettings HereNowSettings)
{
	if(!CheckIsPubnubInitialized() || !CheckIsUserIDSet())
	{return;}

	if(ChannelName.IsEmpty())
	{return;}

	//Set all options from HereNowSettings
	pubnub_here_now_options HereNowOptions = pubnub_here_now_defopts();
	HereNowOptions.disable_uuids = HereNowSettings.DisableUUID;
	HereNowOptions.state = HereNowSettings.State;

	auto CharConverter = StringCast<ANSICHAR>(*HereNowSettings.ChannelGroup);
	const char* ChannelNameChar = CharConverter.Get();
	if(!HereNowSettings.ChannelGroup.IsEmpty())
	{
		HereNowOptions.channel_group = ChannelNameChar;
	}
	
	pubnub_here_now_ex(ctx_pub, TCHAR_TO_ANSI(*ChannelName), HereNowOptions);
	FString JsonResponse = GetLastResponse(ctx_pub);

	//Delegate needs to be executed back on Game Thread
	AsyncTask(ENamedThreads::GameThread, [this, HereNowResponse, JsonResponse]()
	{
		//Broadcast bound delegate with JsonResponse
		HereNowResponse.ExecuteIfBound(JsonResponse);
	});
}

void UPubnubSubsystem::WhereNow_priv(FString UserID, FOnWhereNowResponse WhereNowResponse)
{
	if(!CheckIsPubnubInitialized() || !CheckIsUserIDSet())
	{return;}

	if(UserID.IsEmpty())
	{return;}

	pubnub_where_now(ctx_pub, TCHAR_TO_ANSI(*UserID));
	FString JsonResponse = GetLastResponse(ctx_pub);

	//Delegate needs to be executed back on Game Thread
	AsyncTask(ENamedThreads::GameThread, [this, WhereNowResponse, JsonResponse]()
	{
		//Broadcast bound delegate with JsonResponse
		WhereNowResponse.ExecuteIfBound(JsonResponse);
	});
}

void UPubnubSubsystem::SetState_priv(FString ChannelName, FString StateJson, FPubnubSetStateSettings SetStateSettings)
{
	if(!CheckIsPubnubInitialized() || !CheckIsUserIDSet())
	{return;}

	if(ChannelName.IsEmpty() || StateJson.IsEmpty())
	{return;}

	pubnub_set_state_options SetStateOptions = pubnub_set_state_defopts();

	//Set all options from SetStateSettings
	auto CharConverter = StringCast<ANSICHAR>(*SetStateSettings.ChannelGroup);
	const char* ChannelGroupChar = CharConverter.Get();
	if(!SetStateSettings.ChannelGroup.IsEmpty())
	{
		SetStateOptions.channel_group = ChannelGroupChar;
	}
	auto UserIDCharConverter = StringCast<ANSICHAR>(*SetStateSettings.UserID);
	const char* UserIDChar = UserIDCharConverter.Get();
	if(!SetStateSettings.UserID.IsEmpty())
	{
		SetStateOptions.user_id = UserIDChar;
	}

	SetStateOptions.heartbeat = SetStateSettings.HeartBeat;

	pubnub_set_state_ex(ctx_pub, TCHAR_TO_ANSI(*ChannelName), TCHAR_TO_ANSI(*StateJson), SetStateOptions);

	//TODO: waiting for response takes time. Maybe it would be useful to expose a bool like "WaitForResponse". So if set to false we would skip further part of this function
	pubnub_res PubnubResponse = pubnub_await(ctx_pub);
	if (PNR_OK != PubnubResponse) {
		UE_LOG(PubnubLog, Error, TEXT("Failed to set state. Error code: %d"), PubnubResponse);
	}
}

void UPubnubSubsystem::GetState_priv(FString ChannelName, FString ChannelGroup, FString UserID, FOnGetStateResponse OnGetStateResponse)
{
	if(!CheckIsPubnubInitialized() || !CheckIsUserIDSet())
	{return;}

	if(ChannelName.IsEmpty() && ChannelGroup.IsEmpty())
	{return;}

	pubnub_state_get(ctx_pub, TCHAR_TO_ANSI(*ChannelName), TCHAR_TO_ANSI(*ChannelGroup), TCHAR_TO_ANSI(*UserID));
	FString JsonResponse = GetLastResponse(ctx_pub);

	//Delegate needs to be executed back on Game Thread
	AsyncTask(ENamedThreads::GameThread, [this, OnGetStateResponse, JsonResponse]()
	{
		//Broadcast bound delegate with JsonResponse
		OnGetStateResponse.ExecuteIfBound(JsonResponse);
	});
}

void UPubnubSubsystem::Heartbeat_priv(FString ChannelName, FString ChannelGroup)
{
	if(!CheckIsPubnubInitialized() || !CheckIsUserIDSet())
	{return;}

	if(ChannelName.IsEmpty() && ChannelGroup.IsEmpty())
	{return;}

	pubnub_heartbeat(ctx_pub, TCHAR_TO_ANSI(*ChannelName), TCHAR_TO_ANSI(*ChannelGroup));
}

void UPubnubSubsystem::GrantToken_priv(int TTLMinutes, FString AuthorizedUUID, FOnGrantTokenResponse OnGrantTokenResponse)
{
	if(!CheckIsPubnubInitialized() || !CheckIsUserIDSet())
	{return;}
	
	//Format all data for Grant Token
	struct pam_permission ChPerm;
	ChPerm.read = true;
	int PermMyChannel = pubnub_get_grant_bit_mask_value(ChPerm);
	char PermObj[2000];
	char* AuthorUuid = "my_authorized_uuid";
	sprintf_s(PermObj,"{\"ttl\":%d, \"uuid\":\"%s\", \"permissions\":{\"resources\":{\"channels\":{ \"my_channel\":%d }, \"groups\":{}, \"users\":{ }, \"spaces\":{}}, \"patterns\":{\"channels\":{}, \"groups\":{}, \"users\":{}, \"spaces\":{}},\"meta\":{}}}", TTLMinutes, TCHAR_TO_ANSI(*AuthorizedUUID), PermMyChannel);

	pubnub_grant_token(ctx_pub, PermObj);

	pubnub_res PubnubResponse = pubnub_await(ctx_pub);
	if(PubnubResponse != PNR_OK)
	{
		UE_LOG(PubnubLog, Error, TEXT("Failed to Grant Token. Error code: %d"), PubnubResponse);
		return;
	}

	pubnub_chamebl_t grant_token_resp = pubnub_get_grant_token(ctx_pub);
	FString JsonResponse(grant_token_resp.ptr);
	
	//Delegate needs to be executed back on Game Thread
	AsyncTask(ENamedThreads::GameThread, [this, OnGrantTokenResponse, JsonResponse]()
	{
		//Broadcast bound delegate with JsonResponse
		OnGrantTokenResponse.ExecuteIfBound(JsonResponse);
	});
	
}

void UPubnubSubsystem::RevokeToken_priv(FString Token)
{
	if(!CheckIsPubnubInitialized() || !CheckIsUserIDSet())
	{return;}

	if(Token.IsEmpty())
	{return;}
	
	pubnub_revoke_token(ctx_pub, TCHAR_TO_ANSI(*Token));

	pubnub_res PubnubResponse = pubnub_await(ctx_pub);
	if(PubnubResponse != PNR_OK)
	{
		UE_LOG(PubnubLog, Error, TEXT("Failed to Revoke Token. Error code: %d"), PubnubResponse);
	}
}

void UPubnubSubsystem::ParseToken_priv(FString Token)
{
	if(!CheckIsPubnubInitialized() || !CheckIsUserIDSet())
	{return;}

	if(Token.IsEmpty())
	{return;}
	
	pubnub_parse_token(ctx_pub, TCHAR_TO_ANSI(*Token));

	pubnub_res PubnubResponse = pubnub_await(ctx_pub);
	if(PubnubResponse != PNR_OK)
	{
		UE_LOG(PubnubLog, Error, TEXT("Failed to Parse Token. Error code: %d"), PubnubResponse);
	}
}

void UPubnubSubsystem::SetAuthToken_priv(FString Token)
{
	if(!CheckIsPubnubInitialized() || !CheckIsUserIDSet())
	{return;}

	if(Token.IsEmpty())
	{return;}
	
	pubnub_set_auth_token(ctx_pub, TCHAR_TO_ANSI(*Token));

	pubnub_res PubnubResponse = pubnub_await(ctx_pub);
	if(PubnubResponse != PNR_OK)
	{
		UE_LOG(PubnubLog, Error, TEXT("Failed to Set Auth Token. Error code: %d"), PubnubResponse);
	}
}

void UPubnubSubsystem::History_priv(FString ChannelName, FOnHistoryResponse OnHistoryResponse, FPubnubHistorySettings HistorySettings)
{
	if(!CheckIsPubnubInitialized() || !CheckIsUserIDSet())
	{return;}

	if(ChannelName.IsEmpty())
	{return;}

	//Set all options from HistorySettings
	pubnub_history_options HistoryOptions = pubnub_history_defopts();
	HistoryOptions.string_token = HistorySettings.StringToken;
	HistoryOptions.count = HistorySettings.Count;
	HistoryOptions.reverse = HistorySettings.Reverse;
	HistoryOptions.include_token = HistorySettings.IncludeToken;
	HistoryOptions.include_meta = HistorySettings.IncludeMeta;

	auto StartCharConverter = StringCast<ANSICHAR>(*HistorySettings.Start);
	const char* StartChar = StartCharConverter.Get();
	if(!HistorySettings.Start.IsEmpty())
	{
		HistoryOptions.start = StartChar;
	}

	auto EndCharConverter = StringCast<ANSICHAR>(*HistorySettings.End);
	const char* EndChar = EndCharConverter.Get();
	if(!HistorySettings.End.IsEmpty())
	{
		HistoryOptions.end = EndChar;
	}
	
	pubnub_history_ex(ctx_pub, TCHAR_TO_ANSI(*ChannelName), HistoryOptions);

	FString JsonResponse = GetLastResponse(ctx_pub);

	//Delegate needs to be executed back on Game Thread
	AsyncTask(ENamedThreads::GameThread, [this, OnHistoryResponse, JsonResponse]()
	{
		//Broadcast bound delegate with JsonResponse
		OnHistoryResponse.ExecuteIfBound(JsonResponse);
	});
}

void UPubnubSubsystem::MessageCounts_priv(FString ChannelName, FDateTime TimeStamp, FOnMessageCountsResponse OnMessageCountsResponse)
{
	if(!CheckIsPubnubInitialized() || !CheckIsUserIDSet())
	{return;}

	if(ChannelName.IsEmpty())
	{return;}
	FString UnixTimeStamp = FString::FromInt(TimeStamp.ToUnixTimestamp());
	
	pubnub_message_counts(ctx_pub, TCHAR_TO_ANSI(*ChannelName), TCHAR_TO_ANSI(*UnixTimeStamp));
	FString JsonResponse = GetLastResponse(ctx_pub);

	//Delegate needs to be executed back on Game Thread
	AsyncTask(ENamedThreads::GameThread, [this, OnMessageCountsResponse, JsonResponse]()
	{
		//Broadcast bound delegate with JsonResponse
		OnMessageCountsResponse.ExecuteIfBound(JsonResponse);
	});
}

