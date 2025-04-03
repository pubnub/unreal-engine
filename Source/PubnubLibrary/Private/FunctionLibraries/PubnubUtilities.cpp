// Copyright 2024 PubNub Inc. All Rights Reserved.


#include "FunctionLibraries/PubnubUtilities.h"
#include "Runtime/Launch/Resources/Version.h"
#include "Json.h"


FString UPubnubUtilities::AddQuotesToString(const FString InString, bool SkipIfHasQuotes)
{
	if(InString.Left(1) != "\"" || InString.Right(1) != "\"" || !SkipIfHasQuotes)
	{
		return "\"" + InString + "\"";
	}

	return InString;
}

FString UPubnubUtilities::PubnubCharMemBlockToString(const pubnub_char_mem_block PnChar)
{
	if(!PnChar.ptr)
	{
		return "";
	}

#if ENGINE_MINOR_VERSION <= 3
	//This constructor is deprecated since 5.4
	return FString(PnChar.size, PnChar.ptr);
#else
	return FString::ConstructFromPtrSize(PnChar.ptr, PnChar.size);
#endif
}

FString UPubnubUtilities::MembershipIncludeToString(const FPubnubMembershipInclude& MembershipInclude)
{
	FString FinalString = "";
	if(MembershipInclude.IncludeCustom)			{FinalString.Append("Custom,");}
	if(MembershipInclude.IncludeStatus)			{FinalString.Append("Status,");}
	if(MembershipInclude.IncludeType)			{FinalString.Append("Type,");}
	if(MembershipInclude.IncludeChannel)		{FinalString.Append("Channel,");}
	if(MembershipInclude.IncludeChannelCustom)	{FinalString.Append("Channel.Custom,");}
	if(MembershipInclude.IncludeChannelStatus)	{FinalString.Append("Channel.Status,");}
	if(MembershipInclude.IncludeChannelType)	{FinalString.Append("Channel.Type,");}
	//Total count is passed as a separate parameter, so it's not included directly in the final string

	//If there was any include remove the last comma
	if(!FinalString.IsEmpty())
	{
		FinalString.RemoveAt(FinalString.Len() - 1);
	}

	return FinalString;
}

FString UPubnubUtilities::MemberIncludeToString(const FPubnubMemberInclude& MemberInclude)
{
	FString FinalString = "";
	if(MemberInclude.IncludeCustom)			{FinalString.Append("Custom,");}
	if(MemberInclude.IncludeStatus)			{FinalString.Append("Status,");}
	if(MemberInclude.IncludeType)			{FinalString.Append("Type,");}
	if(MemberInclude.IncludeUser)			{FinalString.Append("User,");}
	if(MemberInclude.IncludeUserCustom)		{FinalString.Append("User.Custom,");}
	if(MemberInclude.IncludeUserStatus)		{FinalString.Append("User.Status,");}
	if(MemberInclude.IncludeUserType)		{FinalString.Append("User.Type,");}
	//Total count is passed as a separate parameter, so it's not included directly in the final string

	//If there was any include remove the last comma
	if(!FinalString.IsEmpty())
	{
		FinalString.RemoveAt(FinalString.Len() - 1);
	}

	return FinalString;
}

FString UPubnubUtilities::MembershipSortTypeToString(const EPubnubMembershipSortType SortType)
{
	switch (SortType)
	{
	case EPubnubMembershipSortType::PMST_ChannelID:
		return "channel.id";
	case EPubnubMembershipSortType::PMST_ChannelName:
		return "channel.name";
	case EPubnubMembershipSortType::PMST_ChannelUpdated:
		return "channel.updated";
	case EPubnubMembershipSortType::PMST_ChannelStatus:
		return "channel.status";
	case EPubnubMembershipSortType::PMST_ChannelType:
		return "channel.type";
	case EPubnubMembershipSortType::PMST_Updated:
		return "updated";
	case EPubnubMembershipSortType::PMST_Status:
		return "status";
	case EPubnubMembershipSortType::PMST_Type:
		return "type";
	}
	return "";
}

FString UPubnubUtilities::MemberSortTypeToString(const EPubnubMemberSortType SortType)
{
	switch (SortType)
	{
	case EPubnubMemberSortType::PMST_UserID:
		return "user.id";
	case EPubnubMemberSortType::PMST_UserName:
		return "user.name";
	case EPubnubMemberSortType::PMST_UserUpdated:
		return "user.updated";
	case EPubnubMemberSortType::PMST_UserStatus:
		return "user.status";
	case EPubnubMemberSortType::PMST_UserType:
		return "user.type";
	case EPubnubMemberSortType::PMST_Updated:
		return "updated";
	case EPubnubMemberSortType::PMST_Status:
		return "status";
	case EPubnubMemberSortType::PMST_Type:
		return "type";
	}
	return "";
}

FString UPubnubUtilities::MembershipSortToString(const FPubnubMembershipSort& MemberInclude)
{
	FString FinalString = "";
	//Form comma separated string of sorts
	for(auto SingleSort : MemberInclude.MembershipSort)
	{
		if(!FinalString.IsEmpty()) {FinalString.Append(",");}
		FinalString.Append(MembershipSortTypeToString(SingleSort.SortType));
		//Default sort is ascending, so we only specify order when it's descending
		if(SingleSort.SortOrder)
		{
			FinalString.Append(":desc");
		}
	}

	return FinalString;
}

FString UPubnubUtilities::MemberSortToString(const FPubnubMemberSort& MemberInclude)
{
	FString FinalString = "";
	//Form comma separated string of sorts
	for(auto SingleSort : MemberInclude.MemberSort)
	{
		if(!FinalString.IsEmpty()) {FinalString.Append(",");}
		FinalString.Append(MemberSortTypeToString(SingleSort.SortType));
		//Default sort is ascending, so we only specify order when it's descending
		if(SingleSort.SortOrder)
		{
			FinalString.Append(":desc");
		}
	}

	return FinalString;
}
