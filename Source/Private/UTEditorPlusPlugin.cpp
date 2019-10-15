
#include "UTEditorPlus.h"

#include "ModuleManager.h"
#include "ModuleInterface.h"

#include "EdGraphSchema_K2.h"

class FUTEditorPlusPlugin : public IModuleInterface
{
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	IConsoleCommand* ModPropsCommand = nullptr;
	IConsoleCommand* ModFuncsCommand = nullptr;

	void ModProps(const TArray<FString>& Args = {});
	void ModFuncs(const TArray<FString>& Args = {});
};

IMPLEMENT_MODULE( FUTEditorPlusPlugin, UTEditorPlus )

//================================================
// Startup
//================================================

void FUTEditorPlusPlugin::StartupModule()
{
	UE_LOG(LogLoad, Log, TEXT("[UTEditorPlus] StartupModule"));

	ModProps();
	ModPropsCommand = IConsoleManager::Get().RegisterConsoleCommand(TEXT("ModProps"), TEXT("test"), FConsoleCommandWithArgsDelegate::CreateRaw(this, &FUTEditorPlusPlugin::ModProps));

	ModFuncs();
	ModFuncsCommand = IConsoleManager::Get().RegisterConsoleCommand(TEXT("ModFuncs"), TEXT("test"), FConsoleCommandWithArgsDelegate::CreateRaw(this, &FUTEditorPlusPlugin::ModFuncs));
}

void FUTEditorPlusPlugin::ModProps(const TArray<FString>& Args)
{
	UE_LOG(UTEditorPlus, Log, TEXT("--------------------------------------------------------------------------------"));
	UE_LOG(UTEditorPlus, Log, TEXT("Modding UProperties..."));

	int32 TotalProps = 0;
	int32 InvisProps = 0;
	int32 ReadOnlyProps = 0;
	for (TObjectIterator<UProperty> Prop; Prop; ++Prop)
	{
		if (!(Prop->PropertyFlags & CPF_Parm))
		{
			TotalProps++;

			if (!Prop->HasAnyPropertyFlags(CPF_BlueprintVisible))
			{
				InvisProps++;
				Prop->SetPropertyFlags(CPF_BlueprintVisible);

				if (Prop->GetPathName() == TEXT("/Script/Engine.Brush:Brush"))
					Prop->SetPropertyFlags(CPF_BlueprintReadOnly);
			}
			else if (Prop->HasAnyPropertyFlags(CPF_BlueprintReadOnly))
			{
				ReadOnlyProps++;

				Prop->ClearPropertyFlags(CPF_BlueprintReadOnly);
				Prop->RemoveMetaData(FBlueprintMetadata::MD_Private);
			}
		}
	}

	UE_LOG(UTEditorPlus, Log, TEXT("Total: %i properties - Turned visible: %i - Turned writeable: %i"), TotalProps, InvisProps, ReadOnlyProps);
	UE_LOG(UTEditorPlus, Log, TEXT("--------------------------------------------------------------------------------"));
}

void FUTEditorPlusPlugin::ModFuncs(const TArray<FString>& Args)
{
	UE_LOG(UTEditorPlus, Log, TEXT("--------------------------------------------------------------------------------"));
	UE_LOG(UTEditorPlus, Log, TEXT("Modding UFunctions..."));

	int32 TotalFuncs = 0;
	int32 TotalModified = 0;
	for (TObjectIterator<UFunction> Func; Func; ++Func)
	{
		TotalFuncs++;

		if (!Func->HasAnyFunctionFlags(FUNC_BlueprintCallable))
		{
			TotalModified++;
			Func->FunctionFlags |= FUNC_BlueprintCallable;
		}
	}

	UE_LOG(UTEditorPlus, Log, TEXT("Total: %i functions - Turned callable: %i"), TotalFuncs, TotalModified);
	UE_LOG(UTEditorPlus, Log, TEXT("--------------------------------------------------------------------------------"));
}

//================================================
// Shutdown
//================================================

void FUTEditorPlusPlugin::ShutdownModule()
{
	UE_LOG(LogLoad, Log, TEXT("[UTEditorPlus] ShutdownModule"));
}
