// Copyright 2024 PubNub Inc. All Rights Reserved.

#include "PubnubSubsystem.h"
#include "Json.h"

#include "Config/PubnubSettings.h"
#include "FunctionLibraries/PubnubJsonUtilities.h"
#include "FunctionLibraries/PubnubUtilities.h"
#include "Threads/PubnubFunctionThread.h"
#include "Threads/PubnubLoopingThread.h"

DEFINE_LOG_CATEGORY(PubnubLog)

void UPubnubSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

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

	//Give some time for C-Core to clean up correctly
	FPlatformProcess::Sleep(0.5);
	
	if(QuickActionThread)
	{
		QuickActionThread->Stop();
	}
	if(LongpollThread)
	{
		LongpollThread->Stop();
	}

	Super::Deinitialize();
	
}

void UPubnubSubsystem::InitPubnub()
{
	if(IsInitialized)
	{
		PubnubError("Pubnub is already initialized", EPubnubErrorType::PET_Warning);
		return;
	}

	InitPubnub_priv();

	//If initialized correctly, create required thread.
	if(IsInitialized)
	{
		//Create new threads - separate for subscribe and all other operations
		QuickActionThread = new FPubnubFunctionThread;
		LongpollThread = new FPubnubLoopingThread;
	}
	
}

void UPubnubSubsystem::DeinitPubnub()
{
	if(!QuickActionThread)
	{
		DeinitPubnub_priv();
		return;
	}
	
	QuickActionThread->AddFunctionToQueue( [this]
	{
		DeinitPubnub_priv();
	});
}

void UPubnubSubsystem::SetUserID(FString UserID)
{
	if(!CheckIsPubnubInitialized() || !CheckQuickActionThreadValidity())
	{return;}
	
	QuickActionThread->AddFunctionToQueue( [this, UserID]
	{
		SetUserID_priv(UserID);
	});
}

FString UPubnubSubsystem::GetUserID()
{
	return GetUserIDInternal();
}

void UPubnubSubsystem::SetSecretKey()
{
	if(!CheckIsPubnubInitialized() || !CheckQuickActionThreadValidity())
	{return;}
	
	QuickActionThread->AddFunctionToQueue( [this]
	{
		SetSecretKey_priv();
	});
}

void UPubnubSubsystem::PublishMessage(FString ChannelName, FString Message, FPubnubPublishSettings PublishSettings)
{
	if(!CheckIsPubnubInitialized() || !CheckQuickActionThreadValidity())
	{return;}
	
	QuickActionThread->AddFunctionToQueue( [this, ChannelName, Message, PublishSettings]
	{
		PublishMessage_priv(ChannelName, Message, PublishSettings);
	});
}

void UPubnubSubsystem::Signal(FString ChannelName, FString Message)
{
	if(!CheckIsPubnubInitialized() || !CheckQuickActionThreadValidity())
	{return;}

	QuickActionThread->AddFunctionToQueue( [this, ChannelName, Message]
	{
		PublishMessage_priv(ChannelName, Message);
	});
}

void UPubnubSubsystem::SubscribeToChannel(FString ChannelName)
{
	if(!CheckIsPubnubInitialized() || !CheckQuickActionThreadValidity())
	{return;}
	
	QuickActionThread->AddFunctionToQueue( [this, ChannelName]
	{
		SubscribeToChannel_priv(ChannelName);
	});
}

void UPubnubSubsystem::SubscribeToGroup(FString GroupName)
{
	if(!CheckIsPubnubInitialized() || !CheckQuickActionThreadValidity())
	{return;}
	
	QuickActionThread->AddFunctionToQueue( [this, GroupName]
	{
		SubscribeToGroup_priv(GroupName);
	});
}

void UPubnubSubsystem::UnsubscribeFromChannel(FString ChannelName)
{
	if(!CheckIsPubnubInitialized() || !CheckQuickActionThreadValidity())
	{return;}
	
	QuickActionThread->AddFunctionToQueue( [this, ChannelName]
	{
		UnsubscribeFromChannel_priv(ChannelName);
	});
}

void UPubnubSubsystem::UnsubscribeFromGroup(FString GroupName)
{
	if(!CheckIsPubnubInitialized() || !CheckQuickActionThreadValidity())
	{return;}
	
	QuickActionThread->AddFunctionToQueue( [this, GroupName]
	{
		UnsubscribeFromGroup_priv(GroupName);
	});
}

void UPubnubSubsystem::UnsubscribeFromAll()
{
	if(!CheckIsPubnubInitialized() || !CheckQuickActionThreadValidity())
	{return;}
	
	QuickActionThread->AddFunctionToQueue( [this]
	{
		UnsubscribeFromAll_priv();
	});
}

void UPubnubSubsystem::AddChannelToGroup(FString ChannelName, FString ChannelGroup)
{
	if(!CheckIsPubnubInitialized() || !CheckQuickActionThreadValidity())
	{return;}
	
	QuickActionThread->AddFunctionToQueue( [this, ChannelName, ChannelGroup]
	{
		AddChannelToGroup_priv(ChannelName, ChannelGroup);
	});
}

void UPubnubSubsystem::RemoveChannelFromGroup(FString ChannelName, FString ChannelGroup)
{
	if(!CheckIsPubnubInitialized() || !CheckQuickActionThreadValidity())
	{return;}
	
	QuickActionThread->AddFunctionToQueue( [this, ChannelName, ChannelGroup]
	{
		RemoveChannelFromGroup_priv(ChannelName, ChannelGroup);
	});
}

void UPubnubSubsystem::ListChannelsFromGroup(FString ChannelGroup, FOnListChannelsFromGroupResponse OnListChannelsResponse)
{
	if(!CheckIsPubnubInitialized() || !CheckQuickActionThreadValidity())
	{return;}
	
	QuickActionThread->AddFunctionToQueue( [this, ChannelGroup, OnListChannelsResponse]
	{
		ListChannelsFromGroup_DATA_priv(ChannelGroup, OnListChannelsResponse);
	});
}

void UPubnubSubsystem::ListChannelsFromGroup_JSON(FString ChannelGroup, FOnPubnubResponse OnListChannelsResponse)
{
	if(!CheckIsPubnubInitialized() || !CheckQuickActionThreadValidity())
	{return;}
	
	QuickActionThread->AddFunctionToQueue( [this, ChannelGroup, OnListChannelsResponse]
	{
		ListChannelsFromGroup_JSON_priv(ChannelGroup, OnListChannelsResponse);
	});
}

void UPubnubSubsystem::RemoveChannelGroup(FString ChannelGroup)
{
	if(!CheckIsPubnubInitialized() || !CheckQuickActionThreadValidity())
	{return;}
	
	QuickActionThread->AddFunctionToQueue( [this, ChannelGroup]
	{
		RemoveChannelGroup_priv(ChannelGroup);
	});
}

void UPubnubSubsystem::ListUsersFromChannel(FString ChannelName, FOnListUsersFromChannelResponse ListUsersFromChannelResponse, FPubnubListUsersFromChannelSettings ListUsersFromChannelSettings)
{
	if(!CheckIsPubnubInitialized() || !CheckQuickActionThreadValidity())
	{return;}
	
	QuickActionThread->AddFunctionToQueue( [this, ChannelName, ListUsersFromChannelResponse, ListUsersFromChannelSettings]
	{
		ListUsersFromChannel_DATA_priv(ChannelName, ListUsersFromChannelResponse, ListUsersFromChannelSettings);
	});
}

void UPubnubSubsystem::ListUsersFromChannel_JSON(FString ChannelName, FOnPubnubResponse ListUsersFromChannelResponse, FPubnubListUsersFromChannelSettings ListUsersFromChannelSettings)
{
	if(!CheckIsPubnubInitialized() || !CheckQuickActionThreadValidity())
	{return;}
	
	QuickActionThread->AddFunctionToQueue( [this, ChannelName, ListUsersFromChannelResponse, ListUsersFromChannelSettings]
	{
		ListUsersFromChannel_JSON_priv(ChannelName, ListUsersFromChannelResponse, ListUsersFromChannelSettings);
	});
}

void UPubnubSubsystem::ListUserSubscribedChannels(FString UserID, FOnListUsersSubscribedChannelsResponse ListUserSubscribedChannelsResponse)
{
	if(!CheckIsPubnubInitialized() || !CheckQuickActionThreadValidity())
	{return;}
	
	QuickActionThread->AddFunctionToQueue( [this, UserID, ListUserSubscribedChannelsResponse]
	{
		ListUserSubscribedChannels_DATA_priv(UserID, ListUserSubscribedChannelsResponse);
	});
}

void UPubnubSubsystem::ListUserSubscribedChannels_JSON(FString UserID, FOnPubnubResponse ListUserSubscribedChannelsResponse)
{
	if(!CheckIsPubnubInitialized() || !CheckQuickActionThreadValidity())
	{return;}
	
	QuickActionThread->AddFunctionToQueue( [this, UserID, ListUserSubscribedChannelsResponse]
	{
		ListUserSubscribedChannels_JSON_priv(UserID, ListUserSubscribedChannelsResponse);
	});
}

void UPubnubSubsystem::SetState(FString ChannelName, FString StateJson, FPubnubSetStateSettings SetStateSettings)
{
	if(!CheckIsPubnubInitialized() || !CheckQuickActionThreadValidity())
	{return;}
	
	QuickActionThread->AddFunctionToQueue( [this, ChannelName, StateJson, SetStateSettings]
	{
		SetState_priv(ChannelName, StateJson, SetStateSettings);
	});
}

void UPubnubSubsystem::GetState(FString ChannelName, FString ChannelGroup, FString UserID, FOnPubnubResponse OnGetStateResponse)
{
	if(!CheckIsPubnubInitialized() || !CheckQuickActionThreadValidity())
	{return;}
	
	QuickActionThread->AddFunctionToQueue( [this, ChannelName, ChannelGroup, UserID, OnGetStateResponse]
	{
		GetState_priv(ChannelName, ChannelGroup, UserID, OnGetStateResponse);
	});
}

void UPubnubSubsystem::Heartbeat(FString ChannelName, FString ChannelGroup)
{
	if(!CheckIsPubnubInitialized() || !CheckQuickActionThreadValidity())
	{return;}
	
	QuickActionThread->AddFunctionToQueue( [this, ChannelName, ChannelGroup]
	{
		Heartbeat_priv(ChannelName, ChannelGroup);
	});
}

void UPubnubSubsystem::GrantToken(FString PermissionObject, FOnPubnubResponse OnGrantTokenResponse)
{
	if(!CheckIsPubnubInitialized() || !CheckQuickActionThreadValidity())
	{return;}
	
	QuickActionThread->AddFunctionToQueue( [this, PermissionObject, OnGrantTokenResponse]
	{
		GrantToken_priv(PermissionObject, OnGrantTokenResponse);
	});
}

void UPubnubSubsystem::RevokeToken(FString Token)
{
	if(!CheckIsPubnubInitialized() || !CheckQuickActionThreadValidity())
	{return;}
	
	QuickActionThread->AddFunctionToQueue( [this, Token]
	{
		RevokeToken_priv(Token);
	});
}

void UPubnubSubsystem::ParseToken(FString Token, FOnPubnubResponse OnParseTokenResponse)
{
	if(!CheckIsPubnubInitialized() || !CheckQuickActionThreadValidity())
	{return;}
	
	QuickActionThread->AddFunctionToQueue( [this, Token, OnParseTokenResponse]
	{
		ParseToken_priv(Token, OnParseTokenResponse);
	});
}

void UPubnubSubsystem::SetAuthToken(FString Token)
{
	if(!CheckIsPubnubInitialized() || !CheckQuickActionThreadValidity())
	{return;}
	
	QuickActionThread->AddFunctionToQueue( [this, Token]
	{
		SetAuthToken_priv(Token);
	});
}

void UPubnubSubsystem::FetchHistory(FString ChannelName, FOnFetchHistoryResponse OnFetchHistoryResponse, FPubnubFetchHistorySettings FetchHistorySettings)
{
	if(!CheckIsPubnubInitialized() || !CheckQuickActionThreadValidity())
	{return;}
	
	QuickActionThread->AddFunctionToQueue( [this, ChannelName, OnFetchHistoryResponse, FetchHistorySettings]
	{
		FetchHistory_DATA_priv(ChannelName, OnFetchHistoryResponse, FetchHistorySettings);
	});
}

void UPubnubSubsystem::FetchHistory_JSON(FString ChannelName, FOnPubnubResponse OnFetchHistoryResponse, FPubnubFetchHistorySettings FetchHistorySettings)
{
	if(!CheckIsPubnubInitialized() || !CheckQuickActionThreadValidity())
	{return;}
	
	QuickActionThread->AddFunctionToQueue( [this, ChannelName, OnFetchHistoryResponse, FetchHistorySettings]
	{
		FetchHistory_JSON_priv(ChannelName, OnFetchHistoryResponse, FetchHistorySettings);
	});
}

void UPubnubSubsystem::MessageCounts(FString ChannelName, FString Timetoken, FOnPubnubIntResponse OnMessageCountsResponse)
{
	if(!CheckIsPubnubInitialized() || !CheckQuickActionThreadValidity())
	{return;}
	
	QuickActionThread->AddFunctionToQueue( [this, ChannelName, Timetoken, OnMessageCountsResponse]
	{
		MessageCounts_priv(ChannelName, Timetoken, OnMessageCountsResponse);
	});
}

void UPubnubSubsystem::GetAllUserMetadata(FOnGetAllUserMetadataResponse OnGetAllUserMetadataResponse, FString Include, int Limit, FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count)
{
	if(!CheckIsPubnubInitialized() || !CheckQuickActionThreadValidity())
	{return;}
	
	QuickActionThread->AddFunctionToQueue( [this, OnGetAllUserMetadataResponse, Include, Limit, Filter, Sort, PageNext, PagePrev,  Count]
	{
		GetAllUserMetadata_DATA_priv(OnGetAllUserMetadataResponse, Include, Limit, Filter, Sort, PageNext, PagePrev,  Count);
	});
}

void UPubnubSubsystem::GetAllUserMetadata_JSON(FOnPubnubResponse OnGetAllUserMetadataResponse, FString Include, int Limit, FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count)
{
	if(!CheckIsPubnubInitialized() || !CheckQuickActionThreadValidity())
	{return;}
	
	QuickActionThread->AddFunctionToQueue( [this, OnGetAllUserMetadataResponse, Include, Limit, Filter, Sort, PageNext, PagePrev,  Count]
	{
		GetAllUserMetadata_JSON_priv(OnGetAllUserMetadataResponse, Include, Limit, Filter, Sort, PageNext, PagePrev,  Count);
	});
}

void UPubnubSubsystem::SetUserMetadata(FString User, FString UserMetadataObj, FString Include)
{
	if(!CheckIsPubnubInitialized() || !CheckQuickActionThreadValidity())
	{return;}
	
	QuickActionThread->AddFunctionToQueue( [this, User, UserMetadataObj, Include]
	{
		SetUserMetadata_priv(User, UserMetadataObj, Include);
	});
}

void UPubnubSubsystem::GetUserMetadata(FString User, FOnGetUserMetadataResponse OnGetUserMetadataResponse, FString Include)
{
	if(!CheckIsPubnubInitialized() || !CheckQuickActionThreadValidity())
	{return;}
	
	QuickActionThread->AddFunctionToQueue( [this, User, OnGetUserMetadataResponse, Include]
	{
		GetUserMetadata_DATA_priv(User, OnGetUserMetadataResponse, Include);
	});
}

void UPubnubSubsystem::GetUserMetadata_JSON(FString User, FOnPubnubResponse OnGetUserMetadataResponse, FString Include)
{
	if(!CheckIsPubnubInitialized() || !CheckQuickActionThreadValidity())
	{return;}
	
	QuickActionThread->AddFunctionToQueue( [this, User, OnGetUserMetadataResponse, Include]
	{
		GetUserMetadata_JSON_priv(User, OnGetUserMetadataResponse, Include);
	});
}

void UPubnubSubsystem::RemoveUserMetadata(FString User)
{
	if(!CheckIsPubnubInitialized() || !CheckQuickActionThreadValidity())
	{return;}
	
	QuickActionThread->AddFunctionToQueue( [this, User]
	{
		RemoveUserMetadata_priv(User);
	});
}

void UPubnubSubsystem::GetAllChannelMetadata(FOnGetAllChannelMetadataResponse OnGetAllChannelMetadataResponse, FString Include, int Limit, FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count)
{
	if(!CheckIsPubnubInitialized() || !CheckQuickActionThreadValidity())
	{return;}
	
	QuickActionThread->AddFunctionToQueue( [this, OnGetAllChannelMetadataResponse, Include, Limit, Filter, Sort, PageNext, PagePrev,  Count]
	{
		GetAllChannelMetadata_DATA_priv(OnGetAllChannelMetadataResponse, Include, Limit, Filter, Sort, PageNext, PagePrev,  Count);
	});
}

void UPubnubSubsystem::GetAllChannelMetadata_JSON(FOnPubnubResponse OnGetAllChannelMetadataResponse, FString Include, int Limit, FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count)
{
	if(!CheckIsPubnubInitialized() || !CheckQuickActionThreadValidity())
	{return;}
	
	QuickActionThread->AddFunctionToQueue( [this, OnGetAllChannelMetadataResponse, Include, Limit, Filter, Sort, PageNext, PagePrev,  Count]
	{
		GetAllChannelMetadata_JSON_priv(OnGetAllChannelMetadataResponse, Include, Limit, Filter, Sort, PageNext, PagePrev,  Count);
	});
}

void UPubnubSubsystem::SetChannelMetadata(FString ChannelMetadataID, FString ChannelMetadataObj, FString Include)
{
	if(!CheckIsPubnubInitialized() || !CheckQuickActionThreadValidity())
	{return;}
	
	QuickActionThread->AddFunctionToQueue( [this, ChannelMetadataID, ChannelMetadataObj, Include]
	{
		SetChannelMetadata_priv(ChannelMetadataID, ChannelMetadataObj, Include);
	});
}

void UPubnubSubsystem::GetChannelMetadata(FString ChannelMetadataID, FOnGetChannelMetadataResponse OnGetChannelMetadataResponse, FString Include)
{
	if(!CheckIsPubnubInitialized() || !CheckQuickActionThreadValidity())
	{return;}
	
	QuickActionThread->AddFunctionToQueue( [this, ChannelMetadataID, OnGetChannelMetadataResponse, Include]
	{
		GetChannelMetadata_DATA_priv(ChannelMetadataID, OnGetChannelMetadataResponse, Include);
	});
}

void UPubnubSubsystem::GetChannelMetadata_JSON(FString ChannelMetadataID, FOnPubnubResponse OnGetChannelMetadataResponse, FString Include)
{
	if(!CheckIsPubnubInitialized() || !CheckQuickActionThreadValidity())
	{return;}
	
	QuickActionThread->AddFunctionToQueue( [this, ChannelMetadataID, OnGetChannelMetadataResponse, Include]
	{
		GetChannelMetadata_JSON_priv(ChannelMetadataID, OnGetChannelMetadataResponse, Include);
	});
}

void UPubnubSubsystem::RemoveChannelMetadata(FString ChannelMetadataID)
{
	if(!CheckIsPubnubInitialized() || !CheckQuickActionThreadValidity())
	{return;}
	
	QuickActionThread->AddFunctionToQueue( [this, ChannelMetadataID]
	{
		RemoveChannelMetadata_priv(ChannelMetadataID);
	});
}

void UPubnubSubsystem::GetMemberships(FString User, FOnGetMembershipsResponse OnGetMembershipResponse, FString Include, int Limit, FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count)
{
	if(!CheckIsPubnubInitialized() || !CheckQuickActionThreadValidity())
	{return;}
	
	QuickActionThread->AddFunctionToQueue( [this, User, OnGetMembershipResponse, Include, Limit, Filter, Sort, PageNext, PagePrev,  Count]
	{
		GetMemberships_DATA_priv(User, OnGetMembershipResponse, Include, Limit, Filter, Sort, PageNext, PagePrev,  Count);
	});
}

void UPubnubSubsystem::GetMemberships_JSON(FString User, FOnPubnubResponse OnGetMembershipResponse, FString Include, int Limit, FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count)
{
	if(!CheckIsPubnubInitialized() || !CheckQuickActionThreadValidity())
	{return;}
	
	QuickActionThread->AddFunctionToQueue( [this, User, OnGetMembershipResponse, Include, Limit, Filter, Sort, PageNext, PagePrev,  Count]
	{
		GetMemberships_JSON_priv(User, OnGetMembershipResponse, Include, Limit, Filter, Sort, PageNext, PagePrev,  Count);
	});
}

void UPubnubSubsystem::SetMemberships(FString User, FString SetObj, FString Include)
{
	if(!CheckIsPubnubInitialized() || !CheckQuickActionThreadValidity())
	{return;}
	
	QuickActionThread->AddFunctionToQueue( [this, User, SetObj, Include]
	{
		SetMemberships_priv(User, SetObj, Include);
	});
}

void UPubnubSubsystem::RemoveMemberships(FString User, FString RemoveObj, FString Include)
{
	if(!CheckIsPubnubInitialized() || !CheckQuickActionThreadValidity())
	{return;}
	
	QuickActionThread->AddFunctionToQueue( [this, User, RemoveObj, Include]
	{
		RemoveMemberships_priv(User, RemoveObj, Include);
	});
}

void UPubnubSubsystem::GetChannelMembers(FString ChannelMetadataID, FOnGetChannelMembersResponse OnGetMembersResponse, FString Include, int Limit, FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count)
{
	if(!CheckIsPubnubInitialized() || !CheckQuickActionThreadValidity())
	{return;}
	
	QuickActionThread->AddFunctionToQueue( [this, ChannelMetadataID, OnGetMembersResponse, Include, Limit, Filter, Sort, PageNext, PagePrev,  Count]
	{
		GetChannelMembers_DATA_priv(ChannelMetadataID, OnGetMembersResponse, Include, Limit, Filter, Sort, PageNext, PagePrev,  Count);
	});
}

void UPubnubSubsystem::GetChannelMembers_JSON(FString ChannelMetadataID, FOnPubnubResponse OnGetMembersResponse, FString Include, int Limit, FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count)
{
	if(!CheckIsPubnubInitialized() || !CheckQuickActionThreadValidity())
	{return;}
	
	QuickActionThread->AddFunctionToQueue( [this, ChannelMetadataID, OnGetMembersResponse, Include, Limit, Filter, Sort, PageNext, PagePrev,  Count]
	{
		GetChannelMembers_JSON_priv(ChannelMetadataID, OnGetMembersResponse, Include, Limit, Filter, Sort, PageNext, PagePrev,  Count);
	});
}

void UPubnubSubsystem::AddChannelMembers(FString ChannelMetadataID, FString AddObj, FString Include)
{
	if(!CheckIsPubnubInitialized() || !CheckQuickActionThreadValidity())
	{return;}
	
	QuickActionThread->AddFunctionToQueue( [this, ChannelMetadataID, AddObj, Include]
	{
		AddChannelMembers_priv(ChannelMetadataID, AddObj, Include);
	});
}

void UPubnubSubsystem::SetChannelMembers(FString ChannelMetadataID, FString SetObj, FString Include)
{
	if(!CheckIsPubnubInitialized() || !CheckQuickActionThreadValidity())
	{return;}
	
	QuickActionThread->AddFunctionToQueue( [this, ChannelMetadataID, SetObj, Include]
	{
		SetChannelMembers_priv(ChannelMetadataID, SetObj, Include);
	});
}

void UPubnubSubsystem::RemoveChannelMembers(FString ChannelMetadataID, FString RemoveObj, FString Include)
{
	if(!CheckIsPubnubInitialized() || !CheckQuickActionThreadValidity())
	{return;}
	
	QuickActionThread->AddFunctionToQueue( [this, ChannelMetadataID, RemoveObj, Include]
	{
		RemoveChannelMembers_priv(ChannelMetadataID, RemoveObj, Include);
	});
}

void UPubnubSubsystem::AddMessageAction(FString ChannelName, FString MessageTimetoken, FString ActionType,  FString Value, FOnAddMessageActionsResponse AddActionResponse)
{
	if(!CheckIsPubnubInitialized() || !CheckQuickActionThreadValidity())
	{return;}
	
	QuickActionThread->AddFunctionToQueue( [this, ChannelName, MessageTimetoken, ActionType, Value, AddActionResponse]
	{
		AddMessageAction_priv(ChannelName, MessageTimetoken, ActionType, Value, AddActionResponse);
	});
}

void UPubnubSubsystem::GetMessageActions(FString ChannelName, FString Start, FString End, int SizeLimit, FOnGetMessageActionsResponse OnGetMessageActionsResponse)
{
	if(!CheckIsPubnubInitialized() || !CheckQuickActionThreadValidity())
	{return;}
	
	QuickActionThread->AddFunctionToQueue( [this, ChannelName, Start, End, SizeLimit, OnGetMessageActionsResponse]
	{
		GetMessageActions_DATA_priv(ChannelName, Start, End, SizeLimit, OnGetMessageActionsResponse);
	});
}

void UPubnubSubsystem::GetMessageActions_JSON(FString ChannelName, FString Start, FString End, int SizeLimit, FOnPubnubResponse OnGetMessageActionsResponse)
{
	if(!CheckIsPubnubInitialized() || !CheckQuickActionThreadValidity())
	{return;}
	
	QuickActionThread->AddFunctionToQueue( [this, ChannelName, Start, End, SizeLimit, OnGetMessageActionsResponse]
	{
		GetMessageActions_JSON_priv(ChannelName, Start, End, SizeLimit, OnGetMessageActionsResponse);
	});
}

void UPubnubSubsystem::RemoveMessageAction(FString ChannelName, FString MessageTimetoken, FString ActionTimetoken)
{
	if(!CheckIsPubnubInitialized() || !CheckQuickActionThreadValidity())
	{return;}
	
	QuickActionThread->AddFunctionToQueue( [this, ChannelName, MessageTimetoken, ActionTimetoken]
	{
		RemoveMessageAction_priv(ChannelName, MessageTimetoken, ActionTimetoken);
	});
}

/* DISABLED 
void UPubnubSubsystem::GetMessageActionsContinue(FOnPubnubResponse OnGetMessageActionsContinueResponse)
{
	if(!CheckIsPubnubInitialized() || !CheckQuickActionThreadValidity())
	{return;}
	
	QuickActionThread->AddFunctionToQueue( [this, OnGetMessageActionsContinueResponse]
	{
		GetMessageActionsContinue_priv(OnGetMessageActionsContinueResponse);
	});
}
*/

FString UPubnubSubsystem::GrantTokenStructureToJsonString(FPubnubGrantTokenStructure TokenStructure, bool &success)
{
	FString TokenJsonString;

	//Make sure token data is provided in correct form. There must be the same amount of object and permissions or just one permission,
	//then one permission is used for every object
	
	if(TokenStructure.Channels.Num() != TokenStructure.ChannelPermissions.Num() && TokenStructure.ChannelPermissions.Num() != 1)
	{
		PubnubError("Grant Token Structure To JsonString - Provide the same amount of ChannelPermissions and Channels (or only 1 ChannelPermission).");
		success = false;
		return TokenJsonString;
	}

	if(TokenStructure.ChannelGroups.Num() != TokenStructure.ChannelGroupPermissions.Num() && TokenStructure.ChannelGroupPermissions.Num() != 1)
	{
		PubnubError("Grant Token Structure To JsonString - Provide the same amount of ChannelGroupPermissions and ChannelGroups (or only 1 ChannelGroupPermissions).");
		success = false;
		return TokenJsonString;
	}

	if(TokenStructure.Users.Num() != TokenStructure.UserPermissions.Num() && TokenStructure.UserPermissions.Num() != 1)
	{
		PubnubError("Grant Token Structure To JsonString - Provide the same amount of UserPermissions and Users (or only 1 UserPermissions).");
		success = false;
		return TokenJsonString;
	}

	if(TokenStructure.ChannelPatterns.Num() != TokenStructure.ChannelPatternPermissions.Num() && TokenStructure.ChannelPatternPermissions.Num() != 1)
	{
		PubnubError("Grant Token Structure To JsonString - Provide the same amount of ChannelPatternPermissions and ChannelsPatterns (or only 1 ChannelPatternPermissions).");
		success = false;
		return TokenJsonString;
	}

	if(TokenStructure.ChannelGroupPatterns.Num() != TokenStructure.ChannelGroupPatternPermissions.Num() && TokenStructure.ChannelGroupPatternPermissions.Num() != 1)
	{
		PubnubError("Grant Token Structure To JsonString - Provide the same amount of ChannelGroupPatternPermissions and ChannelGroupsPatterns (or only 1 ChannelGroupPatternPermissions).");
		success = false;
		return TokenJsonString;
	}

	if(TokenStructure.UserPatterns.Num() != TokenStructure.UserPatternPermissions.Num() && TokenStructure.UserPatternPermissions.Num() != 1)
	{
		PubnubError("Grant Token Structure To JsonString - Provide the same amount of UserPatternPermissions and UsersPatterns (or only 1 UserPatternPermissions).");
		success = false;
		return TokenJsonString;
	}
	

	//Create Json objects with channels, groups, users permissions and their patterns
	TSharedPtr<FJsonObject> ChannelsJsonObject = AddChannelPermissionsToJson(TokenStructure.Channels, TokenStructure.ChannelPermissions);
	TSharedPtr<FJsonObject> ChannelGroupsJsonObject = AddChannelGroupPermissionsToJson(TokenStructure.ChannelGroups, TokenStructure.ChannelGroupPermissions);
	TSharedPtr<FJsonObject> UsersJsonObject = AddUserPermissionsToJson(TokenStructure.Users, TokenStructure.UserPermissions);
	TSharedPtr<FJsonObject> ChannelPatternsJsonObject = AddChannelPermissionsToJson(TokenStructure.ChannelPatterns, TokenStructure.ChannelPatternPermissions);
	TSharedPtr<FJsonObject> ChannelGroupPatternsJsonObject = AddChannelGroupPermissionsToJson(TokenStructure.ChannelGroupPatterns, TokenStructure.ChannelGroupPatternPermissions);
	TSharedPtr<FJsonObject> UserPatternsJsonObject = AddUserPermissionsToJson(TokenStructure.UserPatterns, TokenStructure.UserPatternPermissions);

	//Add resources fields
	TSharedPtr<FJsonObject> ResourcesJsonObject = MakeShareable(new FJsonObject);
	if(TokenStructure.Channels.Num() > 0)
	{
		ResourcesJsonObject->SetObjectField("channels", ChannelsJsonObject);
	}
	if(TokenStructure.ChannelGroups.Num() > 0)
	{
		ResourcesJsonObject->SetObjectField("groups", ChannelGroupsJsonObject);
	}
	if(TokenStructure.Users.Num() > 0)
	{
		ResourcesJsonObject->SetObjectField("uuids", UsersJsonObject);
	}

	//Add patterns fields
	TSharedPtr<FJsonObject> PatternsJsonObject = MakeShareable(new FJsonObject);
	if(TokenStructure.ChannelPatterns.Num() > 0)
	{
		PatternsJsonObject->SetObjectField("channels", ChannelPatternsJsonObject);
	}
	if(TokenStructure.ChannelGroupPatterns.Num() > 0)
	{
		PatternsJsonObject->SetObjectField("groups", ChannelGroupPatternsJsonObject);
	}
	if(TokenStructure.UserPatterns.Num() > 0)
	{
		PatternsJsonObject->SetObjectField("uuids", UserPatternsJsonObject);
	}

	TSharedPtr<FJsonObject> TokenStructureJsonObject = MakeShareable(new FJsonObject);
	TokenStructureJsonObject->SetObjectField("resources", ResourcesJsonObject);
	TokenStructureJsonObject->SetObjectField("patterns", PatternsJsonObject);

	TSharedPtr<FJsonObject> PermissionsJsonObject = MakeShareable(new FJsonObject);
	PermissionsJsonObject->SetNumberField("ttl", TokenStructure.TTLMinutes);
	PermissionsJsonObject->SetStringField("authorized_uuid", TokenStructure.AuthorizedUser);
	PermissionsJsonObject->SetObjectField("permissions", TokenStructureJsonObject);

	//Convert created Json object to string
	return UPubnubJsonUtilities::JsonObjectToString(PermissionsJsonObject);
}

void UPubnubSubsystem::SystemPublish(FString ChannelOpt)
{
	if(SubscribedChannels.IsEmpty() && SubscribedGroups.IsEmpty() && ChannelOpt.IsEmpty())
	{return;}
	
	FPubnubPublishSettings PublishSettings;
	PublishSettings.StoreInHistory = false;

	FString PublishChannel;
	ChannelOpt.IsEmpty() ? PublishChannel = SubscribedChannels[0] : PublishChannel = ChannelOpt;

	//TODO: this will not unlock context if user is subscribed only to groups, but not to any channels. This issue needs to be addressed.
	PublishMessage(PublishChannel, SystemPublishMessage, PublishSettings);
}

void UPubnubSubsystem::StartPubnubSubscribeLoop()
{
	if(!LongpollThread)
	{return;}

	LongpollThread->AddLoopingFunction([this]
	{
		if(SubscribedChannels.IsEmpty() && SubscribedGroups.IsEmpty())
		{return;}
		
		//Subscribe to channels - this is blocking function
		pubnub_subscribe(ctx_sub, TCHAR_TO_ANSI(*StringArrayToCommaSeparated(SubscribedChannels)), TCHAR_TO_ANSI(*StringArrayToCommaSeparated(SubscribedGroups)));
		
		//If context was released on deinitializing subsystem it should just return
		if(!IsInitialized)
		{return;}

		//Check for subscribe result
		pubnub_res SubscribeResult = pubnub_await(ctx_sub);
		if (SubscribeResult != PNR_OK)
		{
			PubnubResponseError(SubscribeResult, "Failed to subscribe to channel.");
			{return;}
		}

		//Check once again, as subsystem could be deinitialized during await
		if(!IsInitialized)
		{return;}

		//At this stage we received messages, so read them and get channel from where they were sent
		const char* MessageChar = pubnub_get(ctx_sub);
		const char* ChannelChar = pubnub_get_channel(ctx_sub);
		while(MessageChar != NULL)
		{
			FString Message(MessageChar);
			FString Channel(ChannelChar);

			//Skip system messages, we don't need to display them to user
			if(Message != SystemPublishMessage)
			{
				//Broadcast callback with message content
				//Message needs to be called back on Game Thread
				AsyncTask(ENamedThreads::GameThread, [this, Message, Channel]()
				{
				OnMessageReceived.Broadcast(Message, Channel);
				});
			}
			
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
		PubnubResponseError(PubnubResponse, "Failed to get last response.");
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
		PubnubResponseError(PubnubResponse, "Failed to get last channel response.");
	}
	return Response;
}

void UPubnubSubsystem::PubnubError(FString ErrorMessage, EPubnubErrorType ErrorType)
{
	//Log and broadcast error message
	if(ErrorType == EPubnubErrorType::PET_Error)
	{
		UE_LOG(PubnubLog, Error, TEXT("%s"), *ErrorMessage);
	}
	else
	{
		UE_LOG(PubnubLog, Warning, TEXT("%s"), *ErrorMessage);
	}

	if(!OnPubnubError.IsBound())
	{
		return;
	}

	//Errors has to be broadcasted on GameThread, otherwise engine will crash if someone uses them for example with widgets
	AsyncTask(ENamedThreads::GameThread, [this, ErrorMessage, ErrorType]()
	{
		//Broadcast bound delegate with JsonResponse
		OnPubnubError.Broadcast(ErrorMessage, ErrorType);
	});
	
}

void UPubnubSubsystem::PubnubResponseError(pubnub_res PubnubResponse, FString ErrorMessage)
{
	//Convert all error data into single string
	FString ResponseString(pubnub_res_2_string(PubnubResponse));
	FString FinalErrorMessage = FString::Printf(TEXT("%s Error: %s."), *ErrorMessage, *ResponseString);

	//Log and broadcast error
	UE_LOG(PubnubLog, Error, TEXT("%s"), *FinalErrorMessage);

	//Errors has to be broadcasted on GameThread, otherwise engine will crash if someone uses them for example with widgets
	AsyncTask(ENamedThreads::GameThread, [this, FinalErrorMessage]()
	{
		//Broadcast bound delegate with JsonResponse
		OnPubnubError.Broadcast(FinalErrorMessage, EPubnubErrorType::PET_Error);;
	});
}

void UPubnubSubsystem::PubnubPublishError()
{
	FString FinalErrorMessage;
	if(ctx_pub == nullptr)
	{
		FinalErrorMessage = "Can't publish message. Publish context is invalid";
	}
	
	FString PublishError(pubnub_last_publish_result(ctx_pub));
	FinalErrorMessage = FString::Printf(TEXT("Publish message failed. Error: %s."), *PublishError);

	//Log and broadcast error
	UE_LOG(PubnubLog, Error, TEXT("%s"), *FinalErrorMessage);

	//Errors has to be broadcasted on GameThread, otherwise engine will crash if someone uses them for example with widgets
	AsyncTask(ENamedThreads::GameThread, [this, FinalErrorMessage]()
	{
		//Broadcast bound delegate with JsonResponse
		OnPubnubError.Broadcast(FinalErrorMessage, EPubnubErrorType::PET_Error);;
	});
}

void UPubnubSubsystem::LoadPluginSettings()
{
	//Save all settings
	PubnubSettings = GetMutableDefault<UPubnubSettings>();
	
	//Copy memory for chars containing keys
	FMemory::Memcpy(PublishKey, TCHAR_TO_ANSI(*PubnubSettings->PublishKey), PublishKeySize);
	FMemory::Memcpy(SubscribeKey, TCHAR_TO_ANSI(*PubnubSettings->SubscribeKey), PublishKeySize);
	FMemory::Memcpy(SecretKey, TCHAR_TO_ANSI(*PubnubSettings->SecretKey), SecretKeySize);
	PublishKey[PublishKeySize] = '\0';
	SubscribeKey[PublishKeySize] = '\0';
	SecretKey[SecretKeySize] = '\0';
}

//This functions is a wrapper to IsInitialized bool, so it can print error if user is trying to do anything before initializing Pubnub correctly
bool UPubnubSubsystem::CheckIsPubnubInitialized()
{
	if(!IsInitialized)
	{
		PubnubError("Pubnub is not initialized correctly. Aborting operation. Make sure to call InitPubnub or "
			"set InitializeAutomatically to true in Pubnub SDK settings. Also make sure that publish and subscribe keys are correct.");
	}
	
	return IsInitialized;
}

//This functions is a wrapper to IsUserIDSet bool, so it can print error if user is trying to do anything before setting user ID correctly
bool UPubnubSubsystem::CheckIsUserIDSet()
{
	if(!IsUserIDSet)
	{
		PubnubError("Pubnub user ID is not set. Aborting operation");
	}
	
	return IsUserIDSet;
}

bool UPubnubSubsystem::CheckQuickActionThreadValidity()
{
	if(!QuickActionThread)
	{
		PubnubError("PublishThread is invalid. Aborting operation");
		return false;
	}
	
	return true;
}

FString UPubnubSubsystem::GetUserIDInternal()
{
	const char* UserIDChar = pubnub_user_id_get(ctx_pub);
	if(UserIDChar)
	{
		FString UserIDString(UserIDChar);
		return UserIDString;
	}
	else
	{return "";}
}

bool UPubnubSubsystem::CheckIsFieldEmpty(FString Field, FString FieldName, FString FunctionName)
{
	if(Field.IsEmpty())
	{
		FString ErrorMessage = FString::Printf(TEXT("Warning: Can't use %s function. %s field shouldn't be empty."), *FunctionName, *FieldName);
		PubnubError(ErrorMessage, EPubnubErrorType::PET_Warning);
		return true;
	}

	return false;
}


/* PRIV FUNCTIONS */

void UPubnubSubsystem::InitPubnub_priv()
{
	if(IsInitialized)
	{return;}
	
	//Make sure that keys are filled
	if(std::strlen(PublishKey) == 0 )
	{
		PubnubError("Publish key is empty, can't initialize Pubnub");
		return;
	}

	if(std::strlen(SubscribeKey) == 0 )
	{
		PubnubError("Subscribe key is empty, can't initialize Pubnub");
		return;
	}
	
	ctx_pub = pubnub_alloc();
	ctx_sub = pubnub_alloc();

	pubnub_init(ctx_pub, PublishKey, SubscribeKey);
	pubnub_init(ctx_sub, PublishKey, SubscribeKey);

	if(PubnubSettings->SetSecretKeyAutomatically)
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
	
	IsInitialized = false;
	
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
	
}

void UPubnubSubsystem::SetUserID_priv(FString UserID)
{
	if(UserID.IsEmpty())
	{
		PubnubError("Can't Set User ID. User ID can't be empty");
		return;
	}

	pubnub_set_user_id(ctx_pub, TCHAR_TO_ANSI(*UserID));
	pubnub_set_user_id(ctx_sub, TCHAR_TO_ANSI(*UserID));

	IsUserIDSet = true;
}

void UPubnubSubsystem::SetSecretKey_priv()
{
	if(std::strlen(SecretKey) == 0)
	{
		PubnubError("Can't set Secret Key. Secret Key is empty.");
		return;
	}

	pubnub_set_secret_key(ctx_pub, SecretKey);
	pubnub_set_secret_key(ctx_sub, SecretKey);
}

void UPubnubSubsystem::PublishMessage_priv(FString ChannelName, FString Message, FPubnubPublishSettings PublishSettings)
{
	if(!CheckIsUserIDSet())
	{return;}

	if(CheckIsFieldEmpty(ChannelName, "ChannelName", "PublishMessage") || CheckIsFieldEmpty(Message, "Message", "PublishMessage"))
	{return;}

	if(!UPubnubJsonUtilities::IsCorrectJsonString(Message))
	{
		PubnubError("Can't Publish Message, Message has to be a correct Json", EPubnubErrorType::PET_Warning);
		return;
	}
	
	//Convert all UE PublishSettings to Pubnub PublishOptions
	
	//Converted char needs to live in function scope, so we need to create it here
	pubnub_publish_options PubnubOptions;
	auto CharConverter = StringCast<ANSICHAR>(*PublishSettings.MetaData);
	PubnubOptions.meta = CharConverter.Get();

	PublishUESettingsToPubnubPublishOptions(PublishSettings, PubnubOptions);
	
	pubnub_publish_ex(ctx_pub, TCHAR_TO_ANSI(*ChannelName), TCHAR_TO_ANSI(*Message), PubnubOptions);
	pubnub_res PublishResult = pubnub_await(ctx_pub);

	if(PublishResult != PNR_OK)
	{
		PubnubPublishError();
	}
}

void UPubnubSubsystem::Signal_priv(FString ChannelName, FString Message)
{
	if(!CheckIsUserIDSet())
	{return;}

	if(CheckIsFieldEmpty(ChannelName, "ChannelName", "Signal") || CheckIsFieldEmpty(Message, "Message", "Signal"))
	{return;}

	pubnub_signal(ctx_pub, TCHAR_TO_ANSI(*ChannelName), TCHAR_TO_ANSI(*Message));
}

void UPubnubSubsystem::SubscribeToChannel_priv(FString ChannelName)
{
	if(!CheckIsUserIDSet())
	{return;}

	if(CheckIsFieldEmpty(ChannelName, "ChannelName", "SubscribeToChannel"))
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
	if(!CheckIsUserIDSet())
	{return;}

	if(CheckIsFieldEmpty(GroupName, "GroupName", "SubscribeToGroup"))
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
	if(!CheckIsUserIDSet())
	{return;}

	FString ChannelForSystemPublish;
	if(SubscribedChannels.Num() >= 1)
	{
		ChannelForSystemPublish = SubscribedChannels[0];
	}
	
	//make sure user was subscribed to that channel
	if(SubscribedChannels.Remove(ChannelName) == 0)
	{return;}

	pubnub_leave(ctx_pub, TCHAR_TO_ANSI(*ChannelName), NULL);

	SystemPublish(ChannelForSystemPublish);
}

void UPubnubSubsystem::UnsubscribeFromGroup_priv(FString GroupName)
{
	if(!CheckIsUserIDSet())
	{return;}
	
	//make sure user was subscribed to that channel
	if(SubscribedGroups.Remove(GroupName) == 0)
	{return;}

	pubnub_leave(ctx_pub, NULL, TCHAR_TO_ANSI(*GroupName));
}

void UPubnubSubsystem::UnsubscribeFromAll_priv()
{
	if(!CheckIsUserIDSet())
	{return;}

	FString ChannelForSystemPublish;
	if(SubscribedChannels.Num() >= 1)
	{
		ChannelForSystemPublish = SubscribedChannels[0];
	}
	
	//Cache and clear all groups and channels
	TArray<FString> SubscribedChannelsCached = SubscribedChannels;
	TArray<FString> SubscribedGroupsCached = SubscribedGroups;
	SubscribedChannels.Empty();
	SubscribedGroups.Empty();
	
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

	if(!ChannelForSystemPublish.IsEmpty())
	{
		SystemPublish(ChannelForSystemPublish);
	}
}

void UPubnubSubsystem::AddChannelToGroup_priv(FString ChannelName, FString ChannelGroup)
{
	if(!CheckIsUserIDSet())
	{return;}

	if(CheckIsFieldEmpty(ChannelName, "ChannelName", "AddChannelToGroup") || CheckIsFieldEmpty(ChannelGroup, "ChannelGroup", "AddChannelToGroup"))
	{return;}
	
	pubnub_add_channel_to_group(ctx_pub, TCHAR_TO_ANSI(*ChannelName), TCHAR_TO_ANSI(*ChannelGroup));
}

void UPubnubSubsystem::RemoveChannelFromGroup_priv(FString ChannelName, FString ChannelGroup)
{
	if(!CheckIsUserIDSet())
	{return;}
	
	if(CheckIsFieldEmpty(ChannelName, "ChannelName", "RemoveChannelFromGroup") || CheckIsFieldEmpty(ChannelGroup, "ChannelGroup", "RemoveChannelFromGroup"))
	{return;}

	pubnub_remove_channel_from_group(ctx_pub, TCHAR_TO_ANSI(*ChannelName), TCHAR_TO_ANSI(*ChannelGroup));
}

FString UPubnubSubsystem::ListChannelsFromGroup_pn(FString ChannelGroup)
{
	pubnub_list_channel_group(ctx_pub, TCHAR_TO_ANSI(*ChannelGroup));
	return GetLastChannelResponse(ctx_pub);
}

void UPubnubSubsystem::ListChannelsFromGroup_JSON_priv(FString ChannelGroup, FOnPubnubResponse OnListChannelsResponse)
{
	if(!CheckIsUserIDSet())
	{return;}

	if(CheckIsFieldEmpty(ChannelGroup, "ChannelGroup", "ListChannelsFromGroup"))
	{return;}
	
	FString JsonResponse = ListChannelsFromGroup_pn(ChannelGroup);

	//Delegate needs to be executed back on Game Thread
	AsyncTask(ENamedThreads::GameThread, [this, OnListChannelsResponse, JsonResponse]()
	{
		//Broadcast bound delegate with JsonResponse
		OnListChannelsResponse.ExecuteIfBound(JsonResponse);
	});
}

void UPubnubSubsystem::ListChannelsFromGroup_DATA_priv(FString ChannelGroup, FOnListChannelsFromGroupResponse OnListChannelsResponse)
{
	if(!CheckIsUserIDSet())
	{return;}

	if(CheckIsFieldEmpty(ChannelGroup, "ChannelGroup", "ListChannelsFromGroup"))
	{return;}
	
	FString JsonResponse = ListChannelsFromGroup_pn(ChannelGroup);

	//Delegate needs to be executed back on Game Thread
	AsyncTask(ENamedThreads::GameThread, [this, OnListChannelsResponse, JsonResponse]()
	{
		//Parse Json response into data
		bool Error;
		int Status;
		TArray<FString> Channels;
		UPubnubJsonUtilities::ListChannelsFromGroupJsonToData(JsonResponse, Error, Status, Channels);
		
		//Broadcast bound delegate with parsed response
		OnListChannelsResponse.ExecuteIfBound(Error, Status, Channels);
	});
}

void UPubnubSubsystem::RemoveChannelGroup_priv(FString ChannelGroup)
{
	if(!CheckIsUserIDSet())
	{return;}

	if(CheckIsFieldEmpty(ChannelGroup, "ChannelGroup", "RemoveChannelGroup"))
	{return;}

	pubnub_remove_channel_group(ctx_pub, TCHAR_TO_ANSI(*ChannelGroup));
}

FString UPubnubSubsystem::ListUsersFromChannel_pn(FString ChannelName, FPubnubListUsersFromChannelSettings ListUsersFromChannelSettings)
{
	//Set all options from ListUsersFromChannelSettings
	
	//Converted char needs to live in function scope, so we need to create it here
	pubnub_here_now_options HereNowOptions;
	auto CharConverter = StringCast<ANSICHAR>(*ListUsersFromChannelSettings.ChannelGroup);
	HereNowOptions.channel_group = CharConverter.Get();
	
	HereNowUESettingsToPubnubHereNowOptions(ListUsersFromChannelSettings, HereNowOptions);
	
	pubnub_here_now_ex(ctx_pub, TCHAR_TO_ANSI(*ChannelName), HereNowOptions);
	return GetLastResponse(ctx_pub);
}

void UPubnubSubsystem::ListUsersFromChannel_JSON_priv(FString ChannelName, FOnPubnubResponse ListUsersFromChannelResponse, FPubnubListUsersFromChannelSettings ListUsersFromChannelSettings)
{
	if(!CheckIsUserIDSet())
	{return;}
	
	if(CheckIsFieldEmpty(ChannelName, "ChannelName", "ListUsersFromChannel"))
	{return;}
	
	FString JsonResponse = ListUsersFromChannel_pn(ChannelName, ListUsersFromChannelSettings);

	//Delegate needs to be executed back on Game Thread
	AsyncTask(ENamedThreads::GameThread, [this, ListUsersFromChannelResponse, JsonResponse]()
	{
		//Broadcast bound delegate with JsonResponse
		ListUsersFromChannelResponse.ExecuteIfBound(JsonResponse);
	});
}

void UPubnubSubsystem::ListUsersFromChannel_DATA_priv(FString ChannelName, FOnListUsersFromChannelResponse ListUsersFromChannelResponse, FPubnubListUsersFromChannelSettings ListUsersFromChannelSettings)
{
	if(!CheckIsUserIDSet())
	{return;}
	
	if(CheckIsFieldEmpty(ChannelName, "ChannelName", "ListUsersFromChannel"))
	{return;}
	
	FString JsonResponse = ListUsersFromChannel_pn(ChannelName, ListUsersFromChannelSettings);

	//Delegate needs to be executed back on Game Thread
	AsyncTask(ENamedThreads::GameThread, [this, ListUsersFromChannelResponse, JsonResponse]()
	{
		//Parse Json response into data
		int Status;
		FString Message;
		FPubnubListUsersFromChannelWrapper Data;
		UPubnubJsonUtilities::ListUsersFromChannelJsonToData(JsonResponse, Status, Message, Data);
		
		//Broadcast bound delegate with parsed response
		ListUsersFromChannelResponse.ExecuteIfBound(Status, Message, Data);
	});
}

FString UPubnubSubsystem::ListUserSubscribedChannels_pn(FString UserID)
{
	pubnub_where_now(ctx_pub, TCHAR_TO_ANSI(*UserID));
	return GetLastResponse(ctx_pub);
}

void UPubnubSubsystem::ListUserSubscribedChannels_JSON_priv(FString UserID, FOnPubnubResponse ListUserSubscribedChannelsResponse)
{
	if(!CheckIsUserIDSet())
	{return;}

	if(CheckIsFieldEmpty(UserID, "UserID", "ListUserSubscribedChannels"))
	{return;}

	FString JsonResponse = ListUserSubscribedChannels_pn(UserID);
	
	//Delegate needs to be executed back on Game Thread
	AsyncTask(ENamedThreads::GameThread, [this, ListUserSubscribedChannelsResponse, JsonResponse]()
	{
		//Broadcast bound delegate with JsonResponse
		ListUserSubscribedChannelsResponse.ExecuteIfBound(JsonResponse);
	});
}

void UPubnubSubsystem::ListUserSubscribedChannels_DATA_priv(FString UserID, FOnListUsersSubscribedChannelsResponse ListUserSubscribedChannelsResponse)
{
	if(!CheckIsUserIDSet())
	{return;}

	if(CheckIsFieldEmpty(UserID, "UserID", "ListUserSubscribedChannels"))
	{return;}

	FString JsonResponse = ListUserSubscribedChannels_pn(UserID);
	
	//Delegate needs to be executed back on Game Thread
	AsyncTask(ENamedThreads::GameThread, [this, ListUserSubscribedChannelsResponse, JsonResponse]()
	{
		//Parse Json response into data
		int Status;
		FString Message;
		TArray<FString> Channels;
		UPubnubJsonUtilities::ListUserSubscribedChannelsJsonToData(JsonResponse, Status, Message, Channels);
		
		//Broadcast bound delegate with parsed response
		ListUserSubscribedChannelsResponse.ExecuteIfBound(Status, Message, Channels);
	});
}

void UPubnubSubsystem::SetState_priv(FString ChannelName, FString StateJson, FPubnubSetStateSettings SetStateSettings)
{
	if(!CheckIsUserIDSet())
	{return;}

	if(CheckIsFieldEmpty(ChannelName, "ChannelName", "SetState") || CheckIsFieldEmpty(StateJson, "StateJson", "SetState"))
	{return;}

	if(!UPubnubJsonUtilities::IsCorrectJsonString(StateJson, false))
	{
		PubnubError("Can't Set State, StateJson has to be a correct Json Object", EPubnubErrorType::PET_Warning);
		return;
	}
	
	//Set all options from SetStateSettings

	//Converted char needs to live in function scope, so we need to create it here
	pubnub_set_state_options SetStateOptions;
	auto CharConverter = StringCast<ANSICHAR>(*SetStateSettings.ChannelGroup);
	SetStateOptions.channel_group = CharConverter.Get();
	auto UserIDCharConverter = StringCast<ANSICHAR>(*SetStateSettings.UserID);
	SetStateOptions.user_id = UserIDCharConverter.Get();

	SetStateUESettingsToPubnubSetStateOptions(SetStateSettings, SetStateOptions);
	
	pubnub_set_state_ex(ctx_pub, TCHAR_TO_ANSI(*ChannelName), TCHAR_TO_ANSI(*StateJson), SetStateOptions);
	
	pubnub_res PubnubResponse = pubnub_await(ctx_pub);
	if (PNR_OK != PubnubResponse) {
		PubnubResponseError(PubnubResponse, "Failed to set state.");
	}
	
	//Clean up the responses
	pubnub_get(ctx_pub);
}

void UPubnubSubsystem::GetState_priv(FString ChannelName, FString ChannelGroup, FString UserID, FOnPubnubResponse OnGetStateResponse)
{
	if(!CheckIsUserIDSet())
	{return;}

	if(ChannelName.IsEmpty() && ChannelGroup.IsEmpty())
	{
		PubnubError("Warning: Can't use GetState function. At least one of: ChannelGroup, ChannelName can't be empty", EPubnubErrorType::PET_Warning);
		return;
	}

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
	if(!CheckIsUserIDSet())
	{return;}

	if(ChannelName.IsEmpty() && ChannelGroup.IsEmpty())
	{
		PubnubError("Warning: Can't use Heartbeat function. At least one of: ChannelGroup, ChannelName can't be empty", EPubnubErrorType::PET_Warning);
		return;
	}

	pubnub_heartbeat(ctx_pub, TCHAR_TO_ANSI(*ChannelName), TCHAR_TO_ANSI(*ChannelGroup));
}

void UPubnubSubsystem::GrantToken_priv(FString PermissionObject, FOnPubnubResponse OnGrantTokenResponse)
{
	if(!CheckIsUserIDSet())
	{return;}

	if(CheckIsFieldEmpty(PermissionObject, "PermissionObject", "GrantToken"))
	{return;}
	
	pubnub_grant_token(ctx_pub, TCHAR_TO_ANSI(*PermissionObject));

	pubnub_res PubnubResponse = pubnub_await(ctx_pub);
	if(PubnubResponse != PNR_OK)
	{
		PubnubResponseError(PubnubResponse, "Failed to Grant Token.");
		return;
	}

	pubnub_chamebl_t grant_token_resp = pubnub_get_grant_token(ctx_pub);
	if(!grant_token_resp.ptr)
	{
		PubnubError("Failed to get Grant Token - pointer to token is invalid.");
		return;
	}
	
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
	if(!CheckIsUserIDSet())
	{return;}

	if(CheckIsFieldEmpty(Token, "Token", "RevokeToken"))
	{return;}
	
	pubnub_revoke_token(ctx_pub, TCHAR_TO_ANSI(*Token));

	pubnub_res PubnubResponse = pubnub_await(ctx_pub);
	if(PubnubResponse != PNR_OK)
	{
		PubnubResponseError(PubnubResponse, "Failed to Revoke Token.");
	}
}

void UPubnubSubsystem::ParseToken_priv(FString Token, FOnPubnubResponse OnParseTokenResponse)
{
	if(!CheckIsUserIDSet())
	{return;}

	if(CheckIsFieldEmpty(Token, "Token", "ParseToken"))
	{return;}
	
	pubnub_parse_token(ctx_pub, TCHAR_TO_ANSI(*Token));

	pubnub_res PubnubResponse = pubnub_await(ctx_pub);
	if(PubnubResponse != PNR_OK)
	{
		PubnubResponseError(PubnubResponse, "Failed to Parse Token.");
	}

	pubnub_chamebl_t grant_token_resp = pubnub_get_grant_token(ctx_pub);
	if(!grant_token_resp.ptr)
	{
		PubnubError("Failed to get Parse Token - pointer to token is invalid.");
		return;
	}
	
	FString JsonResponse(grant_token_resp.ptr);
	
	//Delegate needs to be executed back on Game Thread
	AsyncTask(ENamedThreads::GameThread, [this, OnParseTokenResponse, JsonResponse]()
	{
		//Broadcast bound delegate with JsonResponse
		OnParseTokenResponse.ExecuteIfBound(JsonResponse);
	});
}

void UPubnubSubsystem::SetAuthToken_priv(FString Token)
{
	if(!CheckIsUserIDSet())
	{return;}

	if(CheckIsFieldEmpty(Token, "Token", "SetAuthToken"))
	{return;}
	
	pubnub_set_auth_token(ctx_pub, TCHAR_TO_ANSI(*Token));

	pubnub_res PubnubResponse = pubnub_await(ctx_pub);
	if(PubnubResponse != PNR_OK)
	{
		PubnubResponseError(PubnubResponse, "Failed to Set Auth Token.");
	}
}

FString UPubnubSubsystem::FetchHistory_pn(FString ChannelName, FPubnubFetchHistorySettings FetchHistorySettings)
{
	//Set all options from HistorySettings

	//Converted char needs to live in function scope, so we need to create it here
	pubnub_fetch_history_options FetchHistoryOptions;
	auto StartCharConverter = StringCast<ANSICHAR>(*FetchHistorySettings.Start);
	FetchHistoryOptions.start = StartCharConverter.Get();
	auto EndCharConverter = StringCast<ANSICHAR>(*FetchHistorySettings.End);
	FetchHistoryOptions.end = EndCharConverter.Get();

	FetchHistoryUESettingsToPbFetchHistoryOptions(FetchHistorySettings, FetchHistoryOptions);
	
	pubnub_fetch_history(ctx_pub, TCHAR_TO_ANSI(*ChannelName), FetchHistoryOptions);

	return GetLastResponse(ctx_pub);
}

void UPubnubSubsystem::FetchHistory_JSON_priv(FString ChannelName, FOnPubnubResponse OnFetchHistoryResponse, FPubnubFetchHistorySettings FetchHistorySettings)
{
	if(!CheckIsUserIDSet())
	{return;}

	if(CheckIsFieldEmpty(ChannelName, "ChannelName", "FetchHistory"))
	{return;}
	
	FString JsonResponse = FetchHistory_pn(ChannelName, FetchHistorySettings);

	//Delegate needs to be executed back on Game Thread
	AsyncTask(ENamedThreads::GameThread, [this, OnFetchHistoryResponse, JsonResponse]()
	{
		//Broadcast bound delegate with JsonResponse
		OnFetchHistoryResponse.ExecuteIfBound(JsonResponse);
	});
}

void UPubnubSubsystem::FetchHistory_DATA_priv(FString ChannelName, FOnFetchHistoryResponse OnFetchHistoryResponse, FPubnubFetchHistorySettings FetchHistorySettings)
{
	if(!CheckIsUserIDSet())
	{return;}

	if(CheckIsFieldEmpty(ChannelName, "ChannelName", "FetchHistory"))
	{return;}
	
	FString JsonResponse = FetchHistory_pn(ChannelName, FetchHistorySettings);

	//Delegate needs to be executed back on Game Thread
	AsyncTask(ENamedThreads::GameThread, [this, OnFetchHistoryResponse, JsonResponse]()
	{
		//Parse Json response into data
		bool Error;
		int Status;
		FString ErrorMessage;
		TArray<FPubnubHistoryMessageData> Messages;
		UPubnubJsonUtilities::FetchHistoryJsonToData(JsonResponse, Error, Status, ErrorMessage, Messages);
				
		//Broadcast bound delegate with parsed response
		OnFetchHistoryResponse.ExecuteIfBound(Error, Status, ErrorMessage, Messages);
	});
}

void UPubnubSubsystem::MessageCounts_priv(FString ChannelName, FString Timetoken, FOnPubnubIntResponse OnMessageCountsResponse)
{
	if(!CheckIsUserIDSet())
	{return;}

	if(CheckIsFieldEmpty(ChannelName, "ChannelName", "MessageCounts"))
	{return;}
	
	pubnub_message_counts(ctx_pub, TCHAR_TO_ANSI(*ChannelName), TCHAR_TO_ANSI(*Timetoken));

	int MessageCountsNumber = 0;
	pubnub_get_message_counts(ctx_pub, TCHAR_TO_ANSI(*ChannelName), &MessageCountsNumber);

	//Delegate needs to be executed back on Game Thread
	AsyncTask(ENamedThreads::GameThread, [this, OnMessageCountsResponse, MessageCountsNumber]()
	{
		//Broadcast bound delegate with JsonResponse
		OnMessageCountsResponse.ExecuteIfBound(MessageCountsNumber);
	});
}

FString UPubnubSubsystem::GetAllUserMetadata_pn(FString Include, int Limit, FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count)
{
	pubnub_getall_metadata_opts PubnubOptions = pubnub_getall_metadata_defopts();
	auto CharConverterInclude = StringCast<ANSICHAR>(*Include);
	PubnubOptions.include = CharConverterInclude.Get();
	auto CharConverterFilter = StringCast<ANSICHAR>(*Filter);
	PubnubOptions.filter = CharConverterFilter.Get();
	auto CharConverterSort = StringCast<ANSICHAR>(*Sort);
	PubnubOptions.sort = CharConverterSort.Get();
	auto CharConverterPageNext = StringCast<ANSICHAR>(*PageNext);
	PubnubOptions.page.next = CharConverterPageNext.Get();
	auto CharConverterPagePrev = StringCast<ANSICHAR>(*PagePrev);
	PubnubOptions.page.prev = CharConverterPagePrev.Get();
	PubnubOptions.limit = Limit;
	PubnubOptions.count = (pubnub_tribool)(uint8)Count;
	
	pubnub_getall_uuidmetadata_ex(ctx_pub, PubnubOptions);

	return GetLastResponse(ctx_pub);
}

void UPubnubSubsystem::GetAllUserMetadata_JSON_priv(FOnPubnubResponse OnGetAllUserMetadataResponse, FString Include, int Limit, FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count)
{
	if(!CheckIsUserIDSet())
	{return;}
	
	FString JsonResponse = GetAllUserMetadata_pn(Include, Limit, Filter, Sort, PageNext, PagePrev, Count);

	//Delegate needs to be executed back on Game Thread
	AsyncTask(ENamedThreads::GameThread, [this, OnGetAllUserMetadataResponse, JsonResponse]()
	{
		//Broadcast bound delegate with JsonResponse
		OnGetAllUserMetadataResponse.ExecuteIfBound(JsonResponse);
	});
}

void UPubnubSubsystem::GetAllUserMetadata_DATA_priv(FOnGetAllUserMetadataResponse OnGetAllUserMetadataResponse, FString Include, int Limit, FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count)
{
	if(!CheckIsUserIDSet())
	{return;}

	FString JsonResponse = GetAllUserMetadata_pn(Include, Limit, Filter, Sort, PageNext, PagePrev, Count);

	//Delegate needs to be executed back on Game Thread
	AsyncTask(ENamedThreads::GameThread, [this, OnGetAllUserMetadataResponse, JsonResponse]()
	{
		//Parse Json response into data
		int Status;
		TArray<FPubnubUserData> UsersData;
		FString PageNext;
		FString PagePrev;
		UPubnubJsonUtilities::GetAllUserMetadataJsonToData(JsonResponse, Status, UsersData, PageNext, PagePrev);
						
		//Broadcast bound delegate with parsed response
		OnGetAllUserMetadataResponse.ExecuteIfBound(Status, UsersData, PageNext, PagePrev);
	});
}

void UPubnubSubsystem::SetUserMetadata_priv(FString User, FString UserMetadataObj, FString Include)
{
	if(!CheckIsUserIDSet())
	{return;}

	if(CheckIsFieldEmpty(User, "User", "SetUserMetadata") || CheckIsFieldEmpty(UserMetadataObj, "UserMetadataObj", "SetUserMetadata"))
	{return;}

	if(!UPubnubJsonUtilities::IsCorrectJsonString(UserMetadataObj, false))
	{
		PubnubError("Can't Set User Metadata, UserMetadataObj has to be a correct Json Object", EPubnubErrorType::PET_Warning);
		return;
	}

	pubnub_set_uuidmetadata(ctx_pub, TCHAR_TO_ANSI(*User), TCHAR_TO_ANSI(*Include), TCHAR_TO_ANSI(*UserMetadataObj));

	pubnub_res PubnubResponse = pubnub_await(ctx_pub);
	if(PubnubResponse != PNR_OK)
	{
		PubnubResponseError(PubnubResponse, "Failed to Set User Metadata.");
	}
}

FString UPubnubSubsystem::GetUserMetadata_pn(FString User, FString Include)
{
	pubnub_get_uuidmetadata(ctx_pub, TCHAR_TO_ANSI(*Include), TCHAR_TO_ANSI(*User));

	return GetLastResponse(ctx_pub);
}

void UPubnubSubsystem::GetUserMetadata_JSON_priv(FString User, FOnPubnubResponse OnGetUserMetadataResponse, FString Include)
{
	if(!CheckIsUserIDSet())
	{return;}

	if(CheckIsFieldEmpty(User, "User", "GetUUIDMetadata"))
	{return;}
	
	FString JsonResponse = GetUserMetadata_pn(User, Include);

	//Delegate needs to be executed back on Game Thread
	AsyncTask(ENamedThreads::GameThread, [this, OnGetUserMetadataResponse, JsonResponse]()
	{
		//Broadcast bound delegate with JsonResponse
		OnGetUserMetadataResponse.ExecuteIfBound(JsonResponse);
	});
}

void UPubnubSubsystem::GetUserMetadata_DATA_priv(FString User, FOnGetUserMetadataResponse OnGetUserMetadataResponse, FString Include)
{
	if(!CheckIsUserIDSet())
	{return;}

	if(CheckIsFieldEmpty(User, "User", "GetUserMetadata"))
	{return;}

	FString JsonResponse = GetUserMetadata_pn(User, Include);

	//Delegate needs to be executed back on Game Thread
	AsyncTask(ENamedThreads::GameThread, [this, OnGetUserMetadataResponse, JsonResponse]()
	{
		//Parse Json response into data
		int Status;
		FPubnubUserData UserData;
		UPubnubJsonUtilities::GetUserMetadataJsonToData(JsonResponse, Status, UserData);
								
		//Broadcast bound delegate with parsed response
		OnGetUserMetadataResponse.ExecuteIfBound(Status, UserData);
	});
}

void UPubnubSubsystem::RemoveUserMetadata_priv(FString User)
{
	if(!CheckIsUserIDSet())
	{return;}

	if(CheckIsFieldEmpty(User, "User", "RemoveUserMetadata"))
	{return;}
	
	pubnub_remove_uuidmetadata(ctx_pub, TCHAR_TO_ANSI(*User));

	pubnub_res PubnubResponse = pubnub_await(ctx_pub);
	if(PubnubResponse != PNR_OK)
	{
		PubnubResponseError(PubnubResponse, "Failed to Remove User Metadata.");
	}
}

FString UPubnubSubsystem::GetAllChannelMetadata_pn(FString Include, int Limit, FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count)
{
	pubnub_getall_metadata_opts PubnubOptions = pubnub_getall_metadata_defopts();
	auto CharConverterInclude = StringCast<ANSICHAR>(*Include);
	PubnubOptions.include = CharConverterInclude.Get();
	auto CharConverterFilter = StringCast<ANSICHAR>(*Filter);
	PubnubOptions.filter = CharConverterFilter.Get();
	auto CharConverterSort = StringCast<ANSICHAR>(*Sort);
	PubnubOptions.sort = CharConverterSort.Get();
	auto CharConverterPageNext = StringCast<ANSICHAR>(*PageNext);
	PubnubOptions.page.next = CharConverterPageNext.Get();
	auto CharConverterPagePrev = StringCast<ANSICHAR>(*PagePrev);
	PubnubOptions.page.prev = CharConverterPagePrev.Get();
	PubnubOptions.limit = Limit;
	PubnubOptions.count = (pubnub_tribool)(uint8)Count;
	
	pubnub_getall_channelmetadata_ex(ctx_pub, PubnubOptions);

	return GetLastResponse(ctx_pub);
}

void UPubnubSubsystem::GetAllChannelMetadata_JSON_priv(FOnPubnubResponse OnGetAllChannelMetadataResponse, FString Include, int Limit, FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count)
{
	if(!CheckIsUserIDSet())
	{return;}
	
	FString JsonResponse = GetAllChannelMetadata_pn(Include, Limit, Filter, Sort, PageNext, PagePrev, Count);

	//Delegate needs to be executed back on Game Thread
	AsyncTask(ENamedThreads::GameThread, [this, OnGetAllChannelMetadataResponse, JsonResponse]()
	{
		//Broadcast bound delegate with JsonResponse
		OnGetAllChannelMetadataResponse.ExecuteIfBound(JsonResponse);
	});
}

void UPubnubSubsystem::GetAllChannelMetadata_DATA_priv(FOnGetAllChannelMetadataResponse OnGetAllChannelMetadataResponse, FString Include, int Limit, FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count)
{
	if(!CheckIsUserIDSet())
	{return;}
	
	FString JsonResponse = GetAllChannelMetadata_pn(Include, Limit, Filter, Sort, PageNext, PagePrev, Count);
	
	//Delegate needs to be executed back on Game Thread
	AsyncTask(ENamedThreads::GameThread, [this, OnGetAllChannelMetadataResponse, JsonResponse]()
	{
		//Parse Json response into data
		int Status;
		TArray<FPubnubChannelData> ChannelsData;
		FString PageNext;
		FString PagePrev;
		UPubnubJsonUtilities::GetAllChannelMetadataJsonToData(JsonResponse, Status, ChannelsData, PageNext, PagePrev);
								
		//Broadcast bound delegate with parsed response
		OnGetAllChannelMetadataResponse.ExecuteIfBound(Status, ChannelsData, PageNext, PagePrev);
	});
}

void UPubnubSubsystem::SetChannelMetadata_priv(FString ChannelMetadataID, FString ChannelMetadataObj, FString Include)
{
	if(!CheckIsUserIDSet())
	{return;}
	
	if(CheckIsFieldEmpty(ChannelMetadataID, "ChannelMetadataID", "SetChannelMetadata") || CheckIsFieldEmpty(ChannelMetadataObj, "ChannelMetadataObj", "SetChannelMetadata"))
	{return;}
	
	if(!UPubnubJsonUtilities::IsCorrectJsonString(ChannelMetadataObj, false))
	{
		PubnubError("Can't Set Channel Metadata, ChannelMetadataObj has to be a correct Json Object", EPubnubErrorType::PET_Warning);
		return;
	}
	
	pubnub_set_channelmetadata(ctx_pub, TCHAR_TO_ANSI(*ChannelMetadataID), TCHAR_TO_ANSI(*Include), TCHAR_TO_ANSI(*ChannelMetadataObj));

	pubnub_res PubnubResponse = pubnub_await(ctx_pub);
	if(PubnubResponse != PNR_OK)
	{
		PubnubResponseError(PubnubResponse, "Failed to Set Channel Metadata.");
	}
}

FString UPubnubSubsystem::GetChannelMetadata_pn(FString ChannelMetadataID, FString Include)
{
	pubnub_get_channelmetadata(ctx_pub, TCHAR_TO_ANSI(*Include), TCHAR_TO_ANSI(*ChannelMetadataID));

	return GetLastResponse(ctx_pub);
}

void UPubnubSubsystem::GetChannelMetadata_JSON_priv(FString ChannelMetadataID, FOnPubnubResponse OnGetChannelMetadataResponse, FString Include)
{
	if(!CheckIsUserIDSet())
	{return;}
	
	if(CheckIsFieldEmpty(ChannelMetadataID, "ChannelMetadataID", "GetChannelMetadata"))
	{return;}
	
	FString JsonResponse = GetChannelMetadata_pn(ChannelMetadataID, Include);

	//Delegate needs to be executed back on Game Thread
	AsyncTask(ENamedThreads::GameThread, [this, OnGetChannelMetadataResponse, JsonResponse]()
	{
		//Broadcast bound delegate with JsonResponse
		OnGetChannelMetadataResponse.ExecuteIfBound(JsonResponse);
	});
}

void UPubnubSubsystem::GetChannelMetadata_DATA_priv(FString ChannelMetadataID, FOnGetChannelMetadataResponse OnGetChannelMetadataResponse, FString Include)
{
	if(!CheckIsUserIDSet())
	{return;}
	
	if(CheckIsFieldEmpty(ChannelMetadataID, "ChannelMetadataID", "GetChannelMetadata"))
	{return;}

	FString JsonResponse = GetChannelMetadata_pn(ChannelMetadataID, Include);

	//Delegate needs to be executed back on Game Thread
	AsyncTask(ENamedThreads::GameThread, [this, OnGetChannelMetadataResponse, JsonResponse]()
	{
		//Parse Json response into data
		int Status;
		FPubnubChannelData ChannelData;
		UPubnubJsonUtilities::GetChannelMetadataJsonToData(JsonResponse, Status, ChannelData);
								
		//Broadcast bound delegate with parsed response
		OnGetChannelMetadataResponse.ExecuteIfBound(Status, ChannelData);
	});
}

void UPubnubSubsystem::RemoveChannelMetadata_priv(FString ChannelMetadataID)
{
	if(!CheckIsUserIDSet())
	{return;}

	if(CheckIsFieldEmpty(ChannelMetadataID, "ChannelMetadataID", "RemoveChannelMetadata"))
	{return;}

	pubnub_remove_channelmetadata(ctx_pub, TCHAR_TO_ANSI(*ChannelMetadataID));

	pubnub_res PubnubResponse = pubnub_await(ctx_pub);
	if(PubnubResponse != PNR_OK)
	{
		PubnubResponseError(PubnubResponse, "Failed to Remove Channel Metadata.");
	}
}

FString UPubnubSubsystem::GetMemberships_pn(FString User, FString Include, int Limit,
	FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count)
{
	pubnub_membership_opts PubnubOptions = pubnub_membership_opts();
	auto CharConverterUuid = StringCast<ANSICHAR>(*User);
	PubnubOptions.uuid = CharConverterUuid.Get();
	auto CharConverterInclude = StringCast<ANSICHAR>(*Include);
	PubnubOptions.include = CharConverterInclude.Get();
	auto CharConverterFilter = StringCast<ANSICHAR>(*Filter);
	PubnubOptions.filter = CharConverterFilter.Get();
	auto CharConverterSort = StringCast<ANSICHAR>(*Sort);
	PubnubOptions.sort = CharConverterSort.Get();
	auto CharConverterPageNext = StringCast<ANSICHAR>(*PageNext);
	PubnubOptions.page.next = CharConverterPageNext.Get();
	auto CharConverterPagePrev = StringCast<ANSICHAR>(*PagePrev);
	PubnubOptions.page.prev = CharConverterPagePrev.Get();
	PubnubOptions.limit = Limit;
	PubnubOptions.count = (pubnub_tribool)(uint8)Count;
	
	pubnub_get_memberships_ex(ctx_pub, PubnubOptions);

	return GetLastResponse(ctx_pub);
}

void UPubnubSubsystem::GetMemberships_JSON_priv(FString User, FOnPubnubResponse OnGetMembershipResponse, FString Include, int Limit,
	FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count)
{
	if(!CheckIsUserIDSet())
	{return;}

	if(CheckIsFieldEmpty(User, "User", "GetMemberships"))
	{return;}

	FString JsonResponse = GetMemberships_pn(User, Include, Limit, Filter, Sort, PageNext, PagePrev, Count);

	//Delegate needs to be executed back on Game Thread
	AsyncTask(ENamedThreads::GameThread, [this, OnGetMembershipResponse, JsonResponse]()
	{
		//Broadcast bound delegate with JsonResponse
		OnGetMembershipResponse.ExecuteIfBound(JsonResponse);
	});
}

void UPubnubSubsystem::GetMemberships_DATA_priv(FString User, FOnGetMembershipsResponse OnGetMembershipResponse, FString Include, int Limit,
	FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count)
{
	if(!CheckIsUserIDSet())
	{return;}

	if(CheckIsFieldEmpty(User, "User", "GetMemberships"))
	{return;}
	
	FString JsonResponse = GetMemberships_pn(User, Include, Limit, Filter, Sort, PageNext, PagePrev, Count);

	//Delegate needs to be executed back on Game Thread
	AsyncTask(ENamedThreads::GameThread, [this, OnGetMembershipResponse, JsonResponse]()
	{
		//Parse Json response into data
		int Status;
		TArray<FPubnubGetMembershipsWrapper> MembershipsData;
		FString PageNext;
		FString PagePrev;
		UPubnubJsonUtilities::GetMembershipsJsonToData(JsonResponse, Status, MembershipsData, PageNext, PagePrev);

		//Broadcast bound delegate with parsed response
		OnGetMembershipResponse.ExecuteIfBound(Status, MembershipsData, PageNext, PagePrev);
	});
}

void UPubnubSubsystem::SetMemberships_priv(FString User, FString SetObj, FString Include)
{
	if(!CheckIsUserIDSet())
	{return;}

	if(CheckIsFieldEmpty(User, "User", "SetMemberships") || CheckIsFieldEmpty(SetObj, "SetObj", "SetMemberships"))
	{return;}

	if(!UPubnubJsonUtilities::IsCorrectJsonString(SetObj, false))
	{
		PubnubError("Can't Set Memberships, SetObj has to be a correct Json Object", EPubnubErrorType::PET_Warning);
		return;
	}

	pubnub_set_memberships(ctx_pub, TCHAR_TO_ANSI(*User), TCHAR_TO_ANSI(*Include), TCHAR_TO_ANSI(*SetObj));

	pubnub_res PubnubResponse = pubnub_await(ctx_pub);
	if(PubnubResponse != PNR_OK)
	{
		PubnubResponseError(PubnubResponse, "Failed to Set Memberships.");
	}
}

void UPubnubSubsystem::RemoveMemberships_priv(FString User, FString RemoveObj, FString Include)
{
	if(!CheckIsUserIDSet())
	{return;}

	if(CheckIsFieldEmpty(User, "User", "RemoveMemberships") || CheckIsFieldEmpty(RemoveObj, "RemoveObj", "RemoveMemberships"))
	{return;}

	if(!UPubnubJsonUtilities::IsCorrectJsonString(RemoveObj, false))
	{
		PubnubError("Can't Remove Memberships, RemoveObj has to be a correct Json Object", EPubnubErrorType::PET_Warning);
		return;
	}

	pubnub_remove_memberships(ctx_pub, TCHAR_TO_ANSI(*User), TCHAR_TO_ANSI(*Include), TCHAR_TO_ANSI(*RemoveObj));

	pubnub_res PubnubResponse = pubnub_await(ctx_pub);
	if(PubnubResponse != PNR_OK)
	{
		PubnubResponseError(PubnubResponse, "Failed to Remove Memberships.");
	}
}

FString UPubnubSubsystem::GetChannelMembers_pn(FString ChannelMetadataID, FString Include, int Limit,
	FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count)
{
	pubnub_members_opts PubnubOptions = pubnub_members_opts();
	auto CharConverterInclude = StringCast<ANSICHAR>(*Include);
	PubnubOptions.include = CharConverterInclude.Get();
	auto CharConverterFilter = StringCast<ANSICHAR>(*Filter);
	PubnubOptions.filter = CharConverterFilter.Get();
	auto CharConverterSort = StringCast<ANSICHAR>(*Sort);
	PubnubOptions.sort = CharConverterSort.Get();
	auto CharConverterPageNext = StringCast<ANSICHAR>(*PageNext);
	PubnubOptions.page.next = CharConverterPageNext.Get();
	auto CharConverterPagePrev = StringCast<ANSICHAR>(*PagePrev);
	PubnubOptions.page.prev = CharConverterPagePrev.Get();
	PubnubOptions.limit = Limit;
	PubnubOptions.count = (pubnub_tribool)(uint8)Count;
	
	pubnub_get_members_ex(ctx_pub,TCHAR_TO_ANSI(*ChannelMetadataID), PubnubOptions);

	return GetLastResponse(ctx_pub);
}

void UPubnubSubsystem::GetChannelMembers_JSON_priv(FString ChannelMetadataID, FOnPubnubResponse OnGetMembersResponse, FString Include, int Limit,
	FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count)
{
	if(!CheckIsUserIDSet())
	{return;}

	if(CheckIsFieldEmpty(ChannelMetadataID, "User", "GetChannelMembers"))
	{return;}

	FString JsonResponse = GetChannelMembers_pn(ChannelMetadataID, Include, Limit, Filter, Sort, PageNext, PagePrev, Count);

	//Delegate needs to be executed back on Game Thread
	AsyncTask(ENamedThreads::GameThread, [this, OnGetMembersResponse, JsonResponse]()
	{
		//Broadcast bound delegate with JsonResponse
		OnGetMembersResponse.ExecuteIfBound(JsonResponse);
	});
}

void UPubnubSubsystem::GetChannelMembers_DATA_priv(FString ChannelMetadataID, FOnGetChannelMembersResponse OnGetMembersResponse, FString Include, int Limit,
	FString Filter, FString Sort, FString PageNext, FString PagePrev, EPubnubTribool Count)
{
	if(!CheckIsUserIDSet())
	{return;}

	if(CheckIsFieldEmpty(ChannelMetadataID, "User", "GetChannelMembers"))
	{return;}
	
	FString JsonResponse = GetChannelMembers_pn(ChannelMetadataID, Include, Limit, Filter, Sort, PageNext, PagePrev, Count);

	//Delegate needs to be executed back on Game Thread
	AsyncTask(ENamedThreads::GameThread, [this, OnGetMembersResponse, JsonResponse]()
	{
		//Parse Json response into data
		int Status;
		TArray<FPubnubGetChannelMembersWrapper> MembersData;
		FString PageNext;
		FString PagePrev;
		UPubnubJsonUtilities::GetChannelMembersJsonToData(JsonResponse, Status, MembersData, PageNext, PagePrev);

		//Broadcast bound delegate with parsed response
		OnGetMembersResponse.ExecuteIfBound(Status, MembersData, PageNext, PagePrev);
	});
}


void UPubnubSubsystem::AddChannelMembers_priv(FString ChannelMetadataID, FString AddObj, FString Include)
{
	if(!CheckIsUserIDSet())
	{return;}

	if(CheckIsFieldEmpty(ChannelMetadataID, "ChannelMetadataID", "AddChannelMembers") || CheckIsFieldEmpty(AddObj, "AddObj", "AddChannelMembers"))
	{return;}

	if(!UPubnubJsonUtilities::IsCorrectJsonString(AddObj, false))
	{
		PubnubError("Can't Add Channel Members, AddObj has to be a correct Json Object", EPubnubErrorType::PET_Warning);
		return;
	}

	pubnub_add_members(ctx_pub, TCHAR_TO_ANSI(*ChannelMetadataID), TCHAR_TO_ANSI(*Include), TCHAR_TO_ANSI(*AddObj));

	pubnub_res PubnubResponse = pubnub_await(ctx_pub);
	if(PubnubResponse != PNR_OK)
	{
		PubnubResponseError(PubnubResponse, "Failed to Add Channel Members.");
	}
}

void UPubnubSubsystem::SetChannelMembers_priv(FString ChannelMetadataID, FString SetObj, FString Include)
{
	if(!CheckIsUserIDSet())
	{return;}

	if(CheckIsFieldEmpty(ChannelMetadataID, "ChannelMetadataID", "SetChannelMembers") || CheckIsFieldEmpty(SetObj, "SetObj", "SetChannelMembers"))
	{return;}

	if(!UPubnubJsonUtilities::IsCorrectJsonString(SetObj, false))
	{
		PubnubError("Can't Set Channel Members, SetObj has to be a correct Json Object", EPubnubErrorType::PET_Warning);
		return;
	}

	pubnub_set_members(ctx_pub, TCHAR_TO_ANSI(*ChannelMetadataID), TCHAR_TO_ANSI(*Include), TCHAR_TO_ANSI(*SetObj));

	pubnub_res PubnubResponse = pubnub_await(ctx_pub);
	if(PubnubResponse != PNR_OK)
	{
		PubnubResponseError(PubnubResponse, "Failed to Set Channel Members.");
	}
}

void UPubnubSubsystem::RemoveChannelMembers_priv(FString ChannelMetadataID, FString RemoveObj, FString Include)
{
	if(!CheckIsUserIDSet())
	{return;}

	if(CheckIsFieldEmpty(ChannelMetadataID, "ChannelMetadataID", "RemoveChannelMembers") || CheckIsFieldEmpty(RemoveObj, "RemoveObj", "RemoveChannelMembers"))
	{return;}

	if(!UPubnubJsonUtilities::IsCorrectJsonString(RemoveObj, false))
	{
		PubnubError("Can't Remove Channel Members, RemoveObj has to be a correct Json Object", EPubnubErrorType::PET_Warning);
		return;
	}

	pubnub_remove_members(ctx_pub, TCHAR_TO_ANSI(*ChannelMetadataID), TCHAR_TO_ANSI(*Include), TCHAR_TO_ANSI(*RemoveObj));

	pubnub_res PubnubResponse = pubnub_await(ctx_pub);
	if(PubnubResponse != PNR_OK)
	{
		PubnubResponseError(PubnubResponse, "Failed to Remove Channel Members.");
	}
}

void UPubnubSubsystem::AddMessageAction_priv(FString ChannelName, FString MessageTimetoken, FString ActionType,  FString Value, FOnAddMessageActionsResponse AddActionResponse)
{
	if(!CheckIsUserIDSet())
	{return;}
	
	if(CheckIsFieldEmpty(ChannelName, "ChannelName", "AddMessageAction") || CheckIsFieldEmpty(MessageTimetoken, "MessageTimetoken", "AddMessageAction"))
	{return;}

	//Add quotes to these fields as they are required by C-Core
	FString FinalActionType = UPubnubUtilities::AddQuotesToString(ActionType);
	FString FinalValue = UPubnubUtilities::AddQuotesToString(Value);
	
	pubnub_add_message_action_str(ctx_pub, TCHAR_TO_ANSI(*ChannelName), TCHAR_TO_ANSI(*MessageTimetoken), TCHAR_TO_ANSI(*FinalActionType),  TCHAR_TO_ANSI(*FinalValue));
	pubnub_res PubnubResponse = pubnub_await(ctx_pub);
	if(PubnubResponse != PNR_OK)
	{
		PubnubResponseError(PubnubResponse, "Failed to Add Message Action.");
	}
	pubnub_chamebl_t AddMessageActionResponse = pubnub_get_message_action_timetoken(ctx_pub);

	if(!AddMessageActionResponse.ptr)
	{
		return;
	}
	FString ActionTimetoken(AddMessageActionResponse.ptr);
	//C-Core returns action timetoken in format: "17303705496647270"}} so we need to clean it up and get pure timetoken
	ActionTimetoken = ActionTimetoken.RightChop(1);
	ActionTimetoken = ActionTimetoken.LeftChop(3);
	
	//Delegate needs to be executed back on Game Thread
	AsyncTask(ENamedThreads::GameThread, [this, AddActionResponse, ActionTimetoken]()
	{
		//Broadcast bound delegate with JsonResponse
		AddActionResponse.ExecuteIfBound(ActionTimetoken);
	});
}

void UPubnubSubsystem::RemoveMessageAction_priv(FString ChannelName, FString MessageTimetoken, FString ActionTimetoken)
{
	if(!CheckIsUserIDSet())
	{return;}
	
	if(CheckIsFieldEmpty(ChannelName, "ChannelName", "RemoveMessageAction") || CheckIsFieldEmpty(MessageTimetoken, "MessageTimetoken", "RemoveMessageAction")
		|| CheckIsFieldEmpty(ActionTimetoken, "ActionTimetoken", "RemoveMessageAction"))
	{return;}

	//Add quotes to these fields as they are required by C-Core
	FString FinalMessageTimetoken = UPubnubUtilities::AddQuotesToString(MessageTimetoken);
	FString FinalActionTimetoken = UPubnubUtilities::AddQuotesToString(ActionTimetoken);

	auto MessageTimetokenConverter = StringCast<ANSICHAR>(*FinalMessageTimetoken);
	auto ActionTimetokenConverter = StringCast<ANSICHAR>(*FinalActionTimetoken);

	// Allocate memory for message_timetoken_char and copy the content
	char* message_timetoken_char = new char[FinalMessageTimetoken.Len() + 1];
	std::strcpy(message_timetoken_char, MessageTimetokenConverter.Get());

	pubnub_chamebl_t message_timetoken_chamebl;
	message_timetoken_chamebl.ptr = message_timetoken_char;
	message_timetoken_chamebl.size = FinalMessageTimetoken.Len();
	
	// Allocate memory for action_timetoken_char and copy the content
	char* action_timetoken_char = new char[FinalActionTimetoken.Len() + 1];
	std::strcpy(action_timetoken_char, ActionTimetokenConverter.Get());

	pubnub_chamebl_t action_timetoken_chamebl;
	action_timetoken_chamebl.ptr = action_timetoken_char;
	action_timetoken_chamebl.size = FinalActionTimetoken.Len();
	
	pubnub_remove_message_action(ctx_pub, TCHAR_TO_ANSI(*ChannelName), message_timetoken_chamebl, action_timetoken_chamebl);

	pubnub_res PubnubResponse = pubnub_await(ctx_pub);

	FString JsonResponse = GetLastResponse(ctx_pub);

	if(PubnubResponse != PNR_OK)
	{
		PubnubResponseError(PubnubResponse, "Failed to Remove Message Action.");
	}

	// Clean up allocated memory
	delete[] message_timetoken_char;
	delete[] action_timetoken_char;
}

FString UPubnubSubsystem::GetMessageActions_pn(FString ChannelName, FString Start, FString End, int SizeLimit)
{
	pubnub_get_message_actions(ctx_pub, TCHAR_TO_ANSI(*ChannelName), TCHAR_TO_ANSI(*Start), TCHAR_TO_ANSI(*End), SizeLimit);
	
	return GetLastResponse(ctx_pub);
}

void UPubnubSubsystem::GetMessageActions_JSON_priv(FString ChannelName, FString Start, FString End, int SizeLimit, FOnPubnubResponse OnGetMessageActionsResponse)
{
	if(!CheckIsUserIDSet())
	{return;}
	
	if(CheckIsFieldEmpty(ChannelName, "ChannelName", "HistoryWithMessageActions"))
	{return;}

	FString JsonResponse = GetMessageActions_pn(ChannelName, Start, End, SizeLimit);

	//Delegate needs to be executed back on Game Thread
	AsyncTask(ENamedThreads::GameThread, [this, OnGetMessageActionsResponse, JsonResponse]()
	{
		//Broadcast bound delegate with JsonResponse
		OnGetMessageActionsResponse.ExecuteIfBound(JsonResponse);
	});
}

void UPubnubSubsystem::GetMessageActions_DATA_priv(FString ChannelName, FString Start, FString End, int SizeLimit, FOnGetMessageActionsResponse OnGetMessageActionsResponse)
{
	if(!CheckIsUserIDSet())
	{return;}
	
	if(CheckIsFieldEmpty(ChannelName, "ChannelName", "HistoryWithMessageActions"))
	{return;}

	FString JsonResponse = GetMessageActions_pn(ChannelName, Start, End, SizeLimit);

	//Delegate needs to be executed back on Game Thread
	AsyncTask(ENamedThreads::GameThread, [this, OnGetMessageActionsResponse, JsonResponse]()
	{
		//Parse Json response into data
		int Status;
		TArray<FPubnubMessageActionData> MessageActions;
		UPubnubJsonUtilities::GetMessageActionsJsonToData(JsonResponse, Status, MessageActions);
										
		//Broadcast bound delegate with parsed response
		OnGetMessageActionsResponse.ExecuteIfBound(Status, MessageActions);
	});
}

void UPubnubSubsystem::GetMessageActionsContinue_priv(FOnPubnubResponse OnGetMessageActionsContinueResponse)
{
	if(!CheckIsUserIDSet())
	{return;}

	pubnub_get_message_actions_more(ctx_pub);

	FString JsonResponse = GetLastResponse(ctx_pub);

	//Delegate needs to be executed back on Game Thread
	AsyncTask(ENamedThreads::GameThread, [this, OnGetMessageActionsContinueResponse, JsonResponse]()
	{
		//Broadcast bound delegate with JsonResponse
		OnGetMessageActionsContinueResponse.ExecuteIfBound(JsonResponse);
	});
}

void UPubnubSubsystem::PublishUESettingsToPubnubPublishOptions(FPubnubPublishSettings &PublishSettings, pubnub_publish_options& PubnubPublishOptions)
{
	PubnubPublishOptions.store = PublishSettings.StoreInHistory;
	PubnubPublishOptions.replicate = PublishSettings.Replicate;
	PubnubPublishOptions.cipher_key = NULL;
	PublishSettings.MetaData.IsEmpty() ? PubnubPublishOptions.meta = NULL : nullptr;
	PubnubPublishOptions.method = (pubnub_method)(uint8)PublishSettings.PublishMethod;
}

void UPubnubSubsystem::HereNowUESettingsToPubnubHereNowOptions(FPubnubListUsersFromChannelSettings& HereNowSettings, pubnub_here_now_options& PubnubHereNowOptions)
{
	PubnubHereNowOptions.disable_uuids = HereNowSettings.DisableUserID;
	PubnubHereNowOptions.state = HereNowSettings.State;
	HereNowSettings.ChannelGroup.IsEmpty() ? PubnubHereNowOptions.channel_group = NULL : nullptr;
}

void UPubnubSubsystem::SetStateUESettingsToPubnubSetStateOptions(FPubnubSetStateSettings& SetStateSettings, pubnub_set_state_options& PubnubSetStateOptions)
{
	SetStateSettings.ChannelGroup.IsEmpty() ? PubnubSetStateOptions.channel_group = NULL : nullptr;
	SetStateSettings.UserID.IsEmpty() ? PubnubSetStateOptions.user_id = NULL : nullptr;
	PubnubSetStateOptions.heartbeat = SetStateSettings.HeartBeat;
}

void UPubnubSubsystem::FetchHistoryUESettingsToPbFetchHistoryOptions(FPubnubFetchHistorySettings& FetchHistorySettings, pubnub_fetch_history_options& PubnubFetchHistoryOptions)
{
	PubnubFetchHistoryOptions.max_per_channel = FetchHistorySettings.MaxPerChannel;
	PubnubFetchHistoryOptions.reverse = FetchHistorySettings.Reverse;
	PubnubFetchHistoryOptions.include_meta = FetchHistorySettings.IncludeMeta;
	PubnubFetchHistoryOptions.include_message_type = FetchHistorySettings.IncludeMessageType;
	PubnubFetchHistoryOptions.include_user_id = FetchHistorySettings.IncludeUserID;
	PubnubFetchHistoryOptions.include_message_actions = FetchHistorySettings.IncludeMessageActions;
	FetchHistorySettings.Start.IsEmpty() ? PubnubFetchHistoryOptions.start = NULL : nullptr;
	FetchHistorySettings.End.IsEmpty() ? PubnubFetchHistoryOptions.end = NULL : nullptr;
}

//This functions assumes that Channels and Permissions are already checked. It means that there is the same amount of permissions as channels or there is exactly one permission
TSharedPtr<FJsonObject> UPubnubSubsystem::AddChannelPermissionsToJson(TArray<FString> Channels, TArray<FPubnubChannelPermissions> ChannelPermissions)
{
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
	bool UseOnePermission = ChannelPermissions.Num() == 1;

	TArray<TSharedPtr<FJsonValue>> ObjectValues;
	
	for(int i = 0; i < Channels.Num(); i++)
	{
		if(Channels[i].IsEmpty())
		{
			continue;
		}
		
		//For permissions use the first index if this is the only valid index or corresponding channel index
		FPubnubChannelPermissions CurrentPermissions;
		UseOnePermission ? CurrentPermissions = ChannelPermissions[0] : CurrentPermissions = ChannelPermissions[i];

		//Create bit mask value from all permissions
		struct pam_permission ChPerm;
		ChPerm.read = CurrentPermissions.Read;
		ChPerm.write = CurrentPermissions.Write;
		ChPerm.del = CurrentPermissions.Delete;
		ChPerm.get = CurrentPermissions.Get;
		ChPerm.update = CurrentPermissions.Update;
		ChPerm.manage = CurrentPermissions.Manage;
		ChPerm.join = CurrentPermissions.Join;
		int PermBitMask = pubnub_get_grant_bit_mask_value(ChPerm);

		JsonObject->SetNumberField(Channels[i], PermBitMask);
	}
	
	return JsonObject;
}

TSharedPtr<FJsonObject> UPubnubSubsystem::AddChannelGroupPermissionsToJson(TArray<FString> ChannelGroups, TArray<FPubnubChannelGroupPermissions> ChannelGroupPermissions)
{
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
	bool UseOnePermission = ChannelGroupPermissions.Num() == 1;

	TArray<TSharedPtr<FJsonValue>> ObjectValues;
	
	for(int i = 0; i < ChannelGroups.Num(); i++)
	{
		if(ChannelGroups[i].IsEmpty())
		{
			continue;
		}
		
		//For permissions use the first index if this is the only valid index or corresponding channel index
		FPubnubChannelGroupPermissions CurrentPermissions;
		UseOnePermission ? CurrentPermissions = ChannelGroupPermissions[0] : CurrentPermissions = ChannelGroupPermissions[i];

		//Create bit mask value from all permissions
		struct pam_permission ChPerm;
		ChPerm.read = CurrentPermissions.Read;
		ChPerm.manage = CurrentPermissions.Manage;
		int PermBitMask = pubnub_get_grant_bit_mask_value(ChPerm);

		JsonObject->SetNumberField(ChannelGroups[i], PermBitMask);
	}
	
	return JsonObject;
}

TSharedPtr<FJsonObject> UPubnubSubsystem::AddUserPermissionsToJson(TArray<FString> Users, TArray<FPubnubUserPermissions> UserPermissions)
{
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
	bool UseOnePermission = UserPermissions.Num() == 1;

	TArray<TSharedPtr<FJsonValue>> ObjectValues;
	
	for(int i = 0; i < Users.Num(); i++)
	{
		if(Users[i].IsEmpty())
		{
			continue;
		}
		
		//For permissions use the first index if this is the only valid index or corresponding channel index
		FPubnubUserPermissions CurrentPermissions;
		UseOnePermission ? CurrentPermissions = UserPermissions[0] : CurrentPermissions = UserPermissions[i];

		//Create bit mask value from all permissions
		struct pam_permission ChPerm;
		ChPerm.del = CurrentPermissions.Delete;
		ChPerm.get = CurrentPermissions.Get;
		ChPerm.update = CurrentPermissions.Update;
		int PermBitMask = pubnub_get_grant_bit_mask_value(ChPerm);

		JsonObject->SetNumberField(Users[i], PermBitMask);
	}

	return JsonObject;
}
