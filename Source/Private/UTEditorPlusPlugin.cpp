
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
	IConsoleCommand* ModClassesCommand = nullptr;
	IConsoleCommand* AddUTBindingsCommand = nullptr;

	void ModProps(const TArray<FString>& Args = {});
	void ModFuncs(const TArray<FString>& Args = {});
	void ModClasses(const TArray<FString>& Args = {});
	void AddUTBindings(const TArray<FString>& Args = {});
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

	ModClasses();
	ModClassesCommand = IConsoleManager::Get().RegisterConsoleCommand(TEXT("ModClasses"), TEXT("test"), FConsoleCommandWithArgsDelegate::CreateRaw(this, &FUTEditorPlusPlugin::ModClasses));

	AddUTBindings();
	AddUTBindingsCommand = IConsoleManager::Get().RegisterConsoleCommand(TEXT("AddUTBindings"), TEXT("test"), FConsoleCommandWithArgsDelegate::CreateRaw(this, &FUTEditorPlusPlugin::AddUTBindings));
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
					Prop->SetPropertyFlags(CPF_BlueprintReadOnly);	//crash fix
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

void FUTEditorPlusPlugin::ModClasses(const TArray<FString>& Args)
{
	UE_LOG(UTEditorPlus, Log, TEXT("--------------------------------------------------------------------------------"));
	UE_LOG(UTEditorPlus, Log, TEXT("Modding UClasses..."));

	int32 TotalStructs = 0;
	int32 TotalStructsModified = 0;
	int32 TotalClasses = 0;
	int32 TotalClassesModified = 0;
	for (TObjectIterator<UStruct> Struct; Struct; ++Struct)
	{
		if (Struct->IsA(UClass::StaticClass()))
		{
			TotalClasses++;

			UClass* Class = Cast<UClass>(*Struct);
			
			if (!Struct->GetBoolMetaDataHierarchical(FBlueprintMetadata::MD_AllowableBlueprintVariableType))
			{
				TotalClassesModified++;
				Struct->SetMetaData(FBlueprintMetadata::MD_AllowableBlueprintVariableType, TEXT("true"));
			}
		}
		else
		{
			TotalStructs++;

			if (!Struct->GetBoolMetaDataHierarchical(FBlueprintMetadata::MD_AllowableBlueprintVariableType) && !Struct->GetBoolMetaDataHierarchical(FBlueprintMetadata::MD_NotAllowableBlueprintVariableType))
			{
				TotalStructsModified++;
				Struct->SetMetaData(FBlueprintMetadata::MD_AllowableBlueprintVariableType, TEXT("true"));
			}
		}
		
	}
	/*
	if (UScriptStruct* ScriptStruct = Cast<UScriptStruct>(static_cast<UObject*>(It->Object)))
	{
		if (Packages.ContainsByPredicate([=](UPackage* Package) { return ScriptStruct->IsIn(Package); }) && ScriptStruct->GetCppStructOps())
		{
			ScriptStructs.Add(ScriptStruct);
		}
	}
	*/

	UE_LOG(UTEditorPlus, Log, TEXT("Total: %i structs - Turned visible: %i"), TotalStructs, TotalStructsModified);
	UE_LOG(UTEditorPlus, Log, TEXT("Total: %i classes - Turned visible: %i"), TotalClasses, TotalClassesModified);
	UE_LOG(UTEditorPlus, Log, TEXT("--------------------------------------------------------------------------------"));
}

void FUTEditorPlusPlugin::AddUTBindings(const TArray<FString>& Args)
{
	UE_LOG(UTEditorPlus, Log, TEXT("--------------------------------------------------------------------------------"));
	UE_LOG(UTEditorPlus, Log, TEXT("Adding UT Bindings..."));

	UInputSettings* Settings = UInputSettings::StaticClass()->GetDefaultObject<UInputSettings>();

	TMap<FString, FKey> UTAxis;
	UTAxis.Add(TEXT("MoveForward"), EKeys::W);
	UTAxis.Add(TEXT("MoveBackward"), EKeys::S);
	UTAxis.Add(TEXT("MoveLeft"), EKeys::A);
	UTAxis.Add(TEXT("MoveRight"), EKeys::D);
	UTAxis.Add(TEXT("MoveUp"), EKeys::SpaceBar);
	int32 AddedAxis = 0;
	for (const auto& NewAxis : UTAxis)
	{
		bool bFound = false;
		for (const auto& Mapping : Settings->AxisMappings)
		{
			if (Mapping.AxisName.ToString() == NewAxis.Key)
			{
				bFound = true;
				break;
			}
		}
		if (!bFound)
		{
			Settings->AxisMappings.Add(FInputAxisKeyMapping(FName(*NewAxis.Key), NewAxis.Value));
			AddedAxis++;
		}
	}

	TMap<FString, FKey> UTActions;
	UTActions.Add(TEXT("Jump"), EKeys::SpaceBar);
	UTActions.Add(TEXT("Crouch"), EKeys::LeftControl);
	UTActions.Add(TEXT("ToggleCrouch"), EKeys::LeftControl);
	UTActions.Add(TEXT("Slide"), EKeys::LeftAlt);
	UTActions.Add(TEXT("TapLeft"), EKeys::A);
	UTActions.Add(TEXT("TapRight"), EKeys::D);
	UTActions.Add(TEXT("TapForward"), EKeys::W);
	UTActions.Add(TEXT("TapBack"), EKeys::S);
	UTActions.Add(TEXT("SingleTapDodge"), EKeys::LeftShift);
	UTActions.Add(TEXT("PrevWeapon"), EKeys::MouseScrollUp);
	UTActions.Add(TEXT("NextWeapon"), EKeys::MouseScrollDown);
	UTActions.Add(TEXT("ThrowWeapon"), EKeys::G);
	UTActions.Add(TEXT("StartFire"), EKeys::LeftMouseButton);
	UTActions.Add(TEXT("StopFire"), EKeys::LeftMouseButton);
	UTActions.Add(TEXT("StartAltFire"), EKeys::RightMouseButton);
	UTActions.Add(TEXT("StopAltFire"), EKeys::RightMouseButton);
	UTActions.Add(TEXT("ShowScores"), EKeys::Tab);
	UTActions.Add(TEXT("Talk"), EKeys::T);
	UTActions.Add(TEXT("TeamTalk"), EKeys::Y);
	UTActions.Add(TEXT("FasterEmote"), EKeys::U);
	UTActions.Add(TEXT("SlowerEmote"), EKeys::I);
	UTActions.Add(TEXT("PlayTaunt"), EKeys::H);
	UTActions.Add(TEXT("PlayTaunt2"), EKeys::J);
	UTActions.Add(TEXT("PlayGroupTaunt"), EKeys::K);
	UTActions.Add(TEXT("DropCarriedObject"), EKeys::B);
	UTActions.Add(TEXT("ToggleComMenu"), EKeys::C);
	UTActions.Add(TEXT("ToggleWeaponWheel"), EKeys::MiddleMouseButton);
	UTActions.Add(TEXT("ActivateSpecial"), EKeys::E);
	UTActions.Add(TEXT("PushToTalk"), EKeys::X);
	UTActions.Add(TEXT("RequestRally"), EKeys::R);
	int32 AddedActions = 0;
	for (const auto& NewAction : UTActions)
	{
		bool bFound = false;
		for (const auto& Mapping : Settings->ActionMappings)
		{
			if (Mapping.ActionName.ToString() == NewAction.Key)
			{
				bFound = true;
				break;
			}
		}
		if (!bFound)
		{
			Settings->ActionMappings.Add(FInputActionKeyMapping(FName(*NewAction.Key), NewAction.Value));
			AddedActions++;
		}
	}

	Settings->SaveKeyMappings();

	UE_LOG(UTEditorPlus, Log, TEXT("Added: %i axis bindings - %i action bindings"), AddedAxis, AddedActions);
	UE_LOG(UTEditorPlus, Log, TEXT("--------------------------------------------------------------------------------"));
}


//================================================
// Shutdown
//================================================

void FUTEditorPlusPlugin::ShutdownModule()
{
	UE_LOG(LogLoad, Log, TEXT("[UTEditorPlus] ShutdownModule"));
}
