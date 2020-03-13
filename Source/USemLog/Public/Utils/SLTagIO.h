// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "SLTagIO.generated.h"

/*
* TagType;Key1,Value1;Key2,Value2;Key3,Value3;
*/
USTRUCT()
struct USEMLOG_API FSLTagIO
{
	GENERATED_BODY();

	// Get all tag key value pairs from world
	static TMap<AActor*, TMap<FString, FString>> GetWorldKVPairs(UWorld* World, const FString& TagType);

	// Get tag key value pairs from actor
	static TMap<FString, FString> GetKVPairs(AActor* Actor, const FString& TagType);

	// Add key value pair to the tag value
	static bool AddKVPair(FName& Tag, const FString& TagKey, const FString& TagValue, bool bOverwrite = false);

	// Add key value pair to actor
	static bool AddKVPair(AActor* Actor, const FString& TagType, const FString& TagKey, const FString& TagValue, bool bOverwrite = false);

private:
	// Get tag key value from tag
	FORCEINLINE static FString GetValue(const FName& InTag, const FString& TagKey);

	// Return the index where the tag type was found in the array
	FORCEINLINE static int32 IndexOfType(const TArray<FName>& InTags, const FString& TagType);

	// Return the Type;Key,Value; 
	FORCEINLINE static FString TKVString(const FString& TagType, const FString& TagKey, const FString& TagValue);

	// Return the Key,Value; as FName
	FORCEINLINE static FString KVString(const FString& TagKey, const FString& TagValue);

	// Return the tag as a string with the appended Key,Value; 
	FORCEINLINE static FString AppendKV(const FName& InTag, const FString& TagKey, const FString& TagValue);
};
