#pragma once

#include "Core.h"
#include "Engine.h"

#include "UTEditorPlus.generated.h"

DEFINE_LOG_CATEGORY_STATIC(UTEditorPlus, Log, All);

#define DEBUGLOG Verbose
#define DEBUGLOG Log

UCLASS(Config = EditorPlus)
class UUTEditorPlusConfig : public UObject
{
	GENERATED_BODY()

	UUTEditorPlusConfig(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
	{
		ModProps_Visible.Empty();
		ModProps_Visible.Add(TEXT("*"));

		ModProps_ReadWrite.Empty();

		ModFuncs_Callable.Empty();
		ModFuncs_Callable.Add(TEXT("*"));

		ModClasses_Visible.Empty();
		ModClasses_Visible.Add(TEXT("*"));
	}

public:

	UPROPERTY(Config)
	TArray<FString> ModProps_Visible;

	UPROPERTY(Config)
	TArray<FString> ModProps_ReadWrite;

	UPROPERTY(Config)
	TArray<FString> ModFuncs_Callable;

	UPROPERTY(Config)
	TArray<FString> ModClasses_Visible;
};