// Copyright 2026 PubNub Inc. All Rights Reserved.


#include "FunctionLibraries/PubnubUtilities.h"
#include "Kismet/KismetMathLibrary.h"


FString UPubnubUtilities::GetNameFromFunctionMacro(FString FunctionName)
{
	if(FunctionName.IsEmpty()) {return "";}
	int Index = -1;
	FunctionName.FindLastChar(TEXT(':'), Index);
	//Leave Class name, just take function name
	FString FinalFunctionName = FunctionName.Mid(Index + 1);
	//Remove "_priv" from function name
	FinalFunctionName.ReplaceInline(TEXT("_priv"), TEXT(""));
	return FinalFunctionName;
}
FString UPubnubUtilities::MembershipIncludeToString(const FPubnubMembershipInclude& MembershipInclude)
{
	FString FinalString = "";
	if(MembershipInclude.IncludeCustom)			{FinalString.Append("custom,");}
	if(MembershipInclude.IncludeStatus)			{FinalString.Append("status,");}
	if(MembershipInclude.IncludeType)			{FinalString.Append("type,");}
	if(MembershipInclude.IncludeChannel)		{FinalString.Append("channel,");}
	if(MembershipInclude.IncludeChannelCustom)	{FinalString.Append("channel.custom,");}
	if(MembershipInclude.IncludeChannelStatus)	{FinalString.Append("channel.status,");}
	if(MembershipInclude.IncludeChannelType)	{FinalString.Append("channel.type,");}
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
	if(MemberInclude.IncludeCustom)			{FinalString.Append("custom,");}
	if(MemberInclude.IncludeStatus)			{FinalString.Append("status,");}
	if(MemberInclude.IncludeType)			{FinalString.Append("type,");}
	if(MemberInclude.IncludeUser)			{FinalString.Append("uuid,");}
	if(MemberInclude.IncludeUserCustom)		{FinalString.Append("uuid.custom,");}
	if(MemberInclude.IncludeUserStatus)		{FinalString.Append("uuid.status,");}
	if(MemberInclude.IncludeUserType)		{FinalString.Append("uuid.type,");}
	//Total count is passed as a separate parameter, so it's not included directly in the final string

	//If there was any include remove the last comma
	if(!FinalString.IsEmpty())
	{
		FinalString.RemoveAt(FinalString.Len() - 1);
	}

	return FinalString;
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
	case EPubnubMemberSortType::PMeST_UserID:
		return "uuid.id";
	case EPubnubMemberSortType::PMeST_UserName:
		return "uuid.name";
	case EPubnubMemberSortType::PMeST_UserUpdated:
		return "uuid.updated";
	case EPubnubMemberSortType::PMeST_UserStatus:
		return "uuid.status";
	case EPubnubMemberSortType::PMeST_UserType:
		return "uuid.type";
	case EPubnubMemberSortType::PMeST_Updated:
		return "updated";
	case EPubnubMemberSortType::PMeST_Status:
		return "status";
	case EPubnubMemberSortType::PMeST_Type:
		return "type";
	}
	return "";
}

int UPubnubUtilities::RoundLimitForPubnubFunctions(int ProvidedLimit)
{
	return UKismetMathLibrary::Clamp(ProvidedLimit, 0, PUBNUB_MAX_LIMIT);
}

FString UPubnubUtilities::GetAllIncludeToString(const FPubnubGetAllInclude& GetAllInclude)
{
	FString FinalString;
	if (GetAllInclude.IncludeCustom) { FinalString.Append(TEXT("custom,")); }
	if (GetAllInclude.IncludeStatus) { FinalString.Append(TEXT("status,")); }
	if (GetAllInclude.IncludeType) { FinalString.Append(TEXT("type,")); }
	// Total count is a separate tribool, so it is not part of the include string.

	if (!FinalString.IsEmpty())
	{
		FinalString.RemoveAt(FinalString.Len() - 1);
	}

	return FinalString;
}

FString UPubnubUtilities::GetMetadataIncludeToString(const FPubnubGetMetadataInclude& GetMetadataInclude)
{
	FString FinalString;
	if (GetMetadataInclude.IncludeCustom) { FinalString.Append(TEXT("custom,")); }
	if (GetMetadataInclude.IncludeStatus) { FinalString.Append(TEXT("status,")); }
	if (GetMetadataInclude.IncludeType) { FinalString.Append(TEXT("type,")); }

	if (!FinalString.IsEmpty())
	{
		FinalString.RemoveAt(FinalString.Len() - 1);
	}

	return FinalString;
}

FString UPubnubUtilities::GetAllSortToString(const FPubnubGetAllSort& GetAllSort)
{
	FString FinalString;
	for (const FPubnubGetAllSingleSort& SingleSort : GetAllSort.GetAllSort)
	{
		if (!FinalString.IsEmpty())
		{
			FinalString.Append(TEXT(","));
		}
		FinalString.Append(GetAllSortTypeToString(SingleSort.SortType));
		// Ascending is the server default, so only descending is written out.
		if (SingleSort.SortOrder)
		{
			FinalString.Append(TEXT(":desc"));
		}
	}

	return FinalString;
}

FString UPubnubUtilities::GetAllSortTypeToString(const EPubnubGetAllSortType SortType)
{
	switch (SortType)
	{
	case EPubnubGetAllSortType::PGAST_ID:
		return TEXT("id");
	case EPubnubGetAllSortType::PGAST_Name:
		return TEXT("name");
	case EPubnubGetAllSortType::PGAST_Updated:
		return TEXT("updated");
	case EPubnubGetAllSortType::PGAST_Status:
		return TEXT("status");
	case EPubnubGetAllSortType::PGAST_Type:
		return TEXT("type");
	}
	return TEXT("");
}