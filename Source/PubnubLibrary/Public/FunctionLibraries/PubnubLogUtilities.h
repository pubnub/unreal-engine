#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "PubnubLogUtilities.generated.h"

UCLASS()
class PUBNUBLIBRARY_API UPubnubLogUtilities : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** Formats UTC time into ISO-8601 with milliseconds, e.g. 2026-02-25T12:18:11.797Z */
	UFUNCTION(BlueprintPure, Category = "Pubnub|Logger")
	static FString FormatIso8601UtcWithMilliseconds(const FDateTime& InUtcTime);

	static FString LogToString(const FString& Value);
	static FString LogToString(const FName& Value);
	static FString LogToString(const FText& Value);
	static FString LogToString(const UObject* Value);
	static FString LogToString(UObject* Value);

	template<typename T, typename TEnableIf<TIsEnum<T>::Value, int>::Type = 0>
	static FString LogToString(const T& Value)
	{
		if (const UEnum* Enum = StaticEnum<T>())
		{
			return Enum->GetNameStringByValue(static_cast<int64>(Value));
		}
		return LexToString(static_cast<int64>(Value));
	}

	template<typename T, typename TEnableIf<!TIsEnum<T>::Value && TIsArithmetic<T>::Value, int>::Type = 0>
	static FString LogToString(const T& Value)
	{
		return LexToString(Value);
	}

	template<typename T, typename TEnableIf<TIsPointer<T>::Value && TIsDerivedFrom<typename TRemovePointer<T>::Type, UObject>::Value, int>::Type = 0>
	static FString LogToString(const T& Value)
	{
		return LogToString(static_cast<const UObject*>(Value));
	}

	template<typename T, typename TEnableIf<!TIsEnum<T>::Value && !TIsArithmetic<T>::Value && !TIsPointer<T>::Value, int>::Type = 0>
	static FString LogToString(const T& Value)
	{
		const UScriptStruct* Struct = T::StaticStruct();
		check(Struct != nullptr);
		return LogUStructToString(Struct, &Value);
	}

private:
	static FString LogUStructToString(const UScriptStruct* Struct, const void* StructData);
};
