// Copyright 2025 PubNub Inc. All Rights Reserved.

#include "Entities/PubnubUserMetadataEntity.h"
#include "PubnubSubsystem.h"


UPubnubUserMetadataEntity::UPubnubUserMetadataEntity()
{
	EntityType = EPubnubEntityType::PEnT_UserMetadata;
}

FPubnubUserMetadataResult UPubnubUserMetadataEntity::SetUserMetadata(FPubnubUserData UserMetadata, FPubnubGetMetadataInclude Include)
{
	if (!PubnubClient)
	{
		UE_LOG(PubnubLog, Error, TEXT("Cannot set user metadata - PubnubClient is null. Entity not properly initialized."));
		return FPubnubUserMetadataResult();
	}
	if (EntityID.IsEmpty())
	{
		UE_LOG(PubnubLog, Error, TEXT("Cannot set user metadata - EntityID is empty."));
		return FPubnubUserMetadataResult();
	}
	return PubnubClient->SetUserMetadata(EntityID, UserMetadata, Include);
}

void UPubnubUserMetadataEntity::SetUserMetadataAsync(FPubnubUserData UserMetadata, FOnPubnubSetUserMetadataResponse OnSetUserMetadataResponse, FPubnubGetMetadataInclude Include)
{
	if (!PubnubClient)
	{
		UE_LOG(PubnubLog, Error, TEXT("Cannot set user metadata - PubnubClient is null. Entity not properly initialized."));
		return;
	}
	PubnubClient->SetUserMetadataAsync(EntityID, UserMetadata, OnSetUserMetadataResponse, Include);
}

void UPubnubUserMetadataEntity::SetUserMetadataAsync(FPubnubUserData UserMetadata, FOnPubnubSetUserMetadataResponseNative NativeCallback, FPubnubGetMetadataInclude Include)
{
	if (!PubnubClient)
	{
		UE_LOG(PubnubLog, Error, TEXT("Cannot set user metadata - PubnubClient is null. Entity not properly initialized."));
		return;
	}
	PubnubClient->SetUserMetadataAsync(EntityID, UserMetadata, NativeCallback, Include);
}

FPubnubUserMetadataResult UPubnubUserMetadataEntity::GetUserMetadata(FPubnubGetMetadataInclude Include)
{
	if (!PubnubClient)
	{
		UE_LOG(PubnubLog, Error, TEXT("Cannot get user metadata - PubnubClient is null. Entity not properly initialized."));
		return FPubnubUserMetadataResult();
	}
	if (EntityID.IsEmpty())
	{
		UE_LOG(PubnubLog, Error, TEXT("Cannot get user metadata - EntityID is empty."));
		return FPubnubUserMetadataResult();
	}
	return PubnubClient->GetUserMetadata(EntityID, Include);
}

void UPubnubUserMetadataEntity::GetUserMetadataAsync(FOnPubnubGetUserMetadataResponse OnGetUserMetadataResponse, FPubnubGetMetadataInclude Include)
{
	if (!PubnubClient)
	{
		UE_LOG(PubnubLog, Error, TEXT("Cannot get user metadata - PubnubClient is null. Entity not properly initialized."));
		return;
	}
	PubnubClient->GetUserMetadataAsync(EntityID, OnGetUserMetadataResponse, Include);
}

void UPubnubUserMetadataEntity::GetUserMetadataAsync(FOnPubnubGetUserMetadataResponseNative NativeCallback, FPubnubGetMetadataInclude Include)
{
	if (!PubnubClient)
	{
		UE_LOG(PubnubLog, Error, TEXT("Cannot get user metadata - PubnubClient is null. Entity not properly initialized."));
		return;
	}
	PubnubClient->GetUserMetadataAsync(EntityID, NativeCallback, Include);
}

FPubnubOperationResult UPubnubUserMetadataEntity::RemoveUserMetadata()
{
	if (!PubnubClient)
	{
		UE_LOG(PubnubLog, Error, TEXT("Cannot remove user metadata - PubnubClient is null. Entity not properly initialized."));
		return FPubnubOperationResult();
	}
	if (EntityID.IsEmpty())
	{
		UE_LOG(PubnubLog, Error, TEXT("Cannot remove user metadata - EntityID is empty."));
		return FPubnubOperationResult();
	}
	return PubnubClient->RemoveUserMetadata(EntityID);
}

void UPubnubUserMetadataEntity::RemoveUserMetadataAsync(FOnPubnubRemoveUserMetadataResponse OnRemoveUserMetadataResponse)
{
	if (!PubnubClient)
	{
		UE_LOG(PubnubLog, Error, TEXT("Cannot remove user metadata - PubnubClient is null. Entity not properly initialized."));
		return;
	}
	PubnubClient->RemoveUserMetadataAsync(EntityID, OnRemoveUserMetadataResponse);
}

void UPubnubUserMetadataEntity::RemoveUserMetadataAsync(FOnPubnubRemoveUserMetadataResponseNative NativeCallback)
{
	if (!PubnubClient)
	{
		UE_LOG(PubnubLog, Error, TEXT("Cannot remove user metadata - PubnubClient is null. Entity not properly initialized."));
		return;
	}
	PubnubClient->RemoveUserMetadataAsync(EntityID, NativeCallback);
}

