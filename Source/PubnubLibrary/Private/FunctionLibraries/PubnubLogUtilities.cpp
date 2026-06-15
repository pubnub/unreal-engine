// Copyright 2026 PubNub Inc. All Rights Reserved.

#include "FunctionLibraries/PubnubLogUtilities.h"

#include "UObject/UnrealType.h"

FString UPubnubLogUtilities::FormatIso8601UtcWithMilliseconds(const FDateTime& InUtcTime)
{
	return FString::Printf(
		TEXT("%04d-%02d-%02dT%02d:%02d:%02d.%03dZ"),
		InUtcTime.GetYear(),
		InUtcTime.GetMonth(),
		InUtcTime.GetDay(),
		InUtcTime.GetHour(),
		InUtcTime.GetMinute(),
		InUtcTime.GetSecond(),
		InUtcTime.GetMillisecond()
	);
}

FString UPubnubLogUtilities::LogToString(const FString& Value)
{
	return Value;
}

FString UPubnubLogUtilities::LogToString(const FName& Value)
{
	return Value.ToString();
}

FString UPubnubLogUtilities::LogToString(const FText& Value)
{
	return Value.ToString();
}

FString UPubnubLogUtilities::LogToString(const UObject* Value)
{
	if (!Value)
	{
		return TEXT("null");
	}
	return FString::Printf(TEXT("%s('%s')"), *Value->GetClass()->GetName(), *Value->GetName());
}

FString UPubnubLogUtilities::LogToString(UObject* Value)
{
	return LogToString(static_cast<const UObject*>(Value));
}

FString UPubnubLogUtilities::LogConfigToString(const FPubnubConfig& Config)
{
	FPubnubConfig ConfigToLog = Config;
	ConfigToLog.SecretKey = MaskSecretKeyForLog(Config.SecretKey);
	return LogToString(ConfigToLog);
}

FString UPubnubLogUtilities::MaskSecretKeyForLog(const FString& SecretKey)
{
	if (SecretKey.IsEmpty())
	{
		return SecretKey;
	}

	const int32 Length = SecretKey.Len();
	const FString Prefix = SecretKey.Left(FMath::Min(8, Length));
	return FString::Printf(TEXT("%s… (len=%d)"), *Prefix, Length);
}

FString UPubnubLogUtilities::LogUStructToString(const UScriptStruct* Struct, const void* StructData)
{
	if (!Struct || !StructData)
	{
		return TEXT("<invalid-struct>");
	}

	TArray<FString> Parts;
	for (TFieldIterator<FProperty> It(Struct); It; ++It)
	{
		const FProperty* Property = *It;
		if (!Property)
		{
			continue;
		}

		FString ValueText;
		const void* ValuePtr = Property->ContainerPtrToValuePtr<void>(StructData);
		Property->ExportTextItem_Direct(ValueText, ValuePtr, nullptr, nullptr, PPF_None);
		Parts.Add(FString::Printf(TEXT("%s=%s"), *Property->GetName(), *ValueText));
	}

	return FString::Printf(TEXT("%s(%s)"), *Struct->GetName(), *FString::Join(Parts, TEXT(", ")));
}
