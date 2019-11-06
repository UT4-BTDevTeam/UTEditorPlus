
#include "UTEditorPlus.h"

#include "ModuleManager.h"
#include "ModuleInterface.h"

#include "EdGraphSchema_K2.h"

class FUTEditorPlusPlugin : public IModuleInterface
{
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	UUTEditorPlusConfig* Config;

	TArray<IConsoleCommand*> Commands;

	FString FactorCode_Begin(const TArray<FString>& Args, TArray<FString>& ConfigList, TCHAR* CmdName, TCHAR* ParamDesc, TCHAR* FullExample, TCHAR* PartialExample);
	void FactorCode_End(const FString& Target, const TArray<FString>& Found, const TArray<FString>& Modified, TCHAR* ItemName, TArray<FString>& ConfigList);

	void ModProps_Visible(const TArray<FString>& Args = {});
	void ModProps_ReadWrite(const TArray<FString>& Args = {});
	void ModFuncs_Callable(const TArray<FString>& Args = {});
	void ModClasses_Visible(const TArray<FString>& Args = {});

	void AddUTBindings(const TArray<FString>& Args = {});

	bool bStartup;
};

IMPLEMENT_MODULE(FUTEditorPlusPlugin, UTEditorPlus)

//================================================
// Startup
//================================================

void FUTEditorPlusPlugin::StartupModule()
{
	UE_LOG(LogLoad, Log, TEXT("[UTEditorPlus] StartupModule"));

	Commands.Add(IConsoleManager::Get().RegisterConsoleCommand(TEXT("ModProps_Visible"), TEXT("Makes target properties blueprint-visible"), FConsoleCommandWithArgsDelegate::CreateRaw(this, &FUTEditorPlusPlugin::ModProps_Visible)));
	Commands.Add(IConsoleManager::Get().RegisterConsoleCommand(TEXT("ModProps_ReadWrite"), TEXT("Makes target properties blueprint-read-write"), FConsoleCommandWithArgsDelegate::CreateRaw(this, &FUTEditorPlusPlugin::ModProps_ReadWrite)));
	Commands.Add(IConsoleManager::Get().RegisterConsoleCommand(TEXT("ModFuncs_Callable"), TEXT("Makes target functions blueprint-callable"), FConsoleCommandWithArgsDelegate::CreateRaw(this, &FUTEditorPlusPlugin::ModFuncs_Callable)));
	Commands.Add(IConsoleManager::Get().RegisterConsoleCommand(TEXT("ModClasses_Visible"), TEXT("Makes target classes/structs blueprint-visible"), FConsoleCommandWithArgsDelegate::CreateRaw(this, &FUTEditorPlusPlugin::ModClasses_Visible)));

	AddUTBindings();
	Commands.Add(IConsoleManager::Get().RegisterConsoleCommand(TEXT("AddUTBindings"), TEXT("Generates all missing UT bindings in project settings"), FConsoleCommandWithArgsDelegate::CreateRaw(this, &FUTEditorPlusPlugin::AddUTBindings)));

	Config = GetMutableDefault<UUTEditorPlusConfig>();
	Config->SaveConfig();

	UE_LOG(UTEditorPlus, Log, TEXT("--------------------------------------------------------------------------------"));

	bStartup = true;

	for (const FString& Arg : Config->ModProps_Visible)
	{
		ModProps_Visible({ Arg });
	}

	for (const FString& Arg : Config->ModProps_ReadWrite)
	{
		ModProps_ReadWrite({ Arg });
	}

	for (const FString& Arg : Config->ModFuncs_Callable)
	{
		ModFuncs_Callable({ Arg });
	}

	for (const FString& Arg : Config->ModClasses_Visible)
	{
		ModClasses_Visible({ Arg });
	}

	bStartup = false;

	UE_LOG(UTEditorPlus, Log, TEXT("--------------------------------------------------------------------------------"));
}


//================================================
// Blueprint extended access
//================================================

// Parse arguments, send usage if necessary, handle the Remove case.
FString FUTEditorPlusPlugin::FactorCode_Begin(const TArray<FString>& Args, TArray<FString>& ConfigList, TCHAR* CmdName, TCHAR* ParamDesc, TCHAR* FullExample, TCHAR* PartialExample)
{
	const FString& Target = (Args.Num() >= 1 ? Args[0] : TEXT(""));

	if (Target.IsEmpty())
	{
		UE_LOG(UTEditorPlus, Log, TEXT("Usage: %s <Target> [Remove]"), CmdName);
		UE_LOG(UTEditorPlus, Log, TEXT("Target can be a fully qualified %s like \"%s\""), ParamDesc, FullExample);
		UE_LOG(UTEditorPlus, Log, TEXT("Or a partial %s like \"%s\", in which case it may be found in multiple places."), ParamDesc, PartialExample);
		UE_LOG(UTEditorPlus, Log, TEXT("If additional parameter \"remove\" is passed, target is removed from modlist. Takes effect upon restart."));
		return TEXT("");
	}

	bool bAll = (Target == TEXT("*"));

	if (Args.Num() >= 2 && Args[1] == TEXT("remove"))
	{
		ConfigList.RemoveAll([&](const FString& Other) {
			if (bAll || Other.Contains(Target))
			{
				UE_LOG(UTEditorPlus, Log, TEXT("Removed '%s' from the modlist"), *Other);
				return true;
			}
			return false;
		});
		Config->SaveConfig();
		return TEXT("");
	}

	return Target;
}

// Print result of command, update & save config if necessary
void FUTEditorPlusPlugin::FactorCode_End(const FString& Target, const TArray<FString>& Found, const TArray<FString>& Modified, TCHAR* ItemName, TArray<FString>& ConfigList)
{
	UE_LOG(UTEditorPlus, Log, TEXT("Found: %i %s - Modified: %i"), Found.Num(), ItemName, Modified.Num());

	if (Modified.Num() <= 20)
	{
		for (const FString& PathName : Modified)
		{
			UE_LOG(UTEditorPlus, Log, TEXT("Modified: %s"), *PathName);
		}
	}

	if (!bStartup && Found.Num() > 0)
	{
		ConfigList.AddUnique(Target);
		Config->SaveConfig();
	}
}

// Prepare iteration
#define PREPARE_ITERATION(UTYPE) \
	if (Target.IsEmpty()) return; \
	UE_LOG(UTEditorPlus, Log, TEXT("Looking up %s matching '%s' ..."), TEXT(UTYPE), *Target); \
	bool bAll = (Target == TEXT("*")); \
	bool bFullPath = Target.Contains(TEXT(":")); \
	TArray<FString> Found; \
	TArray<FString> Modified;

void FUTEditorPlusPlugin::ModProps_Visible(const TArray<FString>& Args)
{
	FString Target = FactorCode_Begin(Args, Config->ModProps_Visible, TEXT("ModProps_Visible"), TEXT("property name"), TEXT("NetConnection:CurrentNetSpeed"), TEXT("CurrentNetSpeed"));
	PREPARE_ITERATION("UProperties");
	for (TObjectIterator<UProperty> Prop; Prop; ++Prop)
	{
		if (!(Prop->PropertyFlags & CPF_Parm))
		{
			if (bAll || (bFullPath ? Prop->GetPathName().Contains(Target) : Prop->GetName().Contains(Target)))
			{
				Found.Emplace(Prop->GetPathName());
				if (!Prop->HasAnyPropertyFlags(CPF_BlueprintVisible))
				{
					Prop->SetPropertyFlags(CPF_BlueprintVisible | CPF_BlueprintReadOnly);
					Modified.Emplace(Prop->GetPathName());
				}
			}
		}
	}
	FactorCode_End(Target, Found, Modified, TEXT("properties"), Config->ModProps_Visible);
}

void FUTEditorPlusPlugin::ModProps_ReadWrite(const TArray<FString>& Args)
{
	FString Target = FactorCode_Begin(Args, Config->ModProps_ReadWrite, TEXT("ModProps_ReadWrite"), TEXT("property name"), TEXT("NetConnection:CurrentNetSpeed"), TEXT("CurrentNetSpeed"));
	PREPARE_ITERATION("UProperties");
	for (TObjectIterator<UProperty> Prop; Prop; ++Prop)
	{
		if (!(Prop->PropertyFlags & CPF_Parm))
		{
			if (bAll || (bFullPath ? Prop->GetPathName().Contains(Target) : Prop->GetName().Contains(Target)))
			{
				Found.Emplace(Prop->GetPathName());
				if (Prop->HasAnyPropertyFlags(CPF_BlueprintReadOnly))
				{
					Prop->ClearPropertyFlags(CPF_BlueprintReadOnly);
					Prop->RemoveMetaData(FBlueprintMetadata::MD_Private);
					Modified.Emplace(Prop->GetPathName());
				}
			}
		}
	}
	FactorCode_End(Target, Found, Modified, TEXT("properties"), Config->ModProps_ReadWrite);
}

void FUTEditorPlusPlugin::ModFuncs_Callable(const TArray<FString>& Args)
{
	FString Target = FactorCode_Begin(Args, Config->ModFuncs_Callable, TEXT("ModFuncs_Callable"), TEXT("function name"), TEXT("UTCharacter:SetHatClass"), TEXT("SetHatClass"));
	PREPARE_ITERATION("UFunctions");
	for (TObjectIterator<UFunction> Func; Func; ++Func)
	{
		if (bAll || (bFullPath ? Func->GetPathName().Contains(Target) : Func->GetName().Contains(Target)))
		{
			Found.Emplace(Func->GetPathName());
			if (!Func->HasAnyFunctionFlags(FUNC_BlueprintCallable))
			{
				Func->FunctionFlags |= FUNC_BlueprintCallable;
				Modified.Emplace(Func->GetPathName());
			}
		}
	}
	FactorCode_End(Target, Found, Modified, TEXT("functions"), Config->ModFuncs_Callable);
}

void FUTEditorPlusPlugin::ModClasses_Visible(const TArray<FString>& Args)
{
	FString Target = FactorCode_Begin(Args, Config->ModClasses_Visible, TEXT("ModClasses_Visible"), TEXT("class name"), TEXT("/Script/UnrealTournament.UTLocalPlayer"), TEXT("LocalPlayer"));
	PREPARE_ITERATION("UClasses and UStructs");
	for (TObjectIterator<UStruct> Struct; Struct; ++Struct)
	{
		if (bAll || (bFullPath ? Struct->GetPathName().Contains(Target) : Struct->GetName().Contains(Target)))
		{
			Found.Emplace(Struct->GetPathName());
			if (!Struct->GetBoolMetaDataHierarchical(FBlueprintMetadata::MD_AllowableBlueprintVariableType) && !Struct->GetBoolMetaDataHierarchical(FBlueprintMetadata::MD_NotAllowableBlueprintVariableType))
			{
				Struct->SetMetaData(FBlueprintMetadata::MD_AllowableBlueprintVariableType, TEXT("true"));
				Modified.Emplace(Struct->GetPathName());
			}
		}
	}
	FactorCode_End(Target, Found, Modified, TEXT("classes/structs"), Config->ModClasses_Visible);
}


//================================================
// UT Bindings
//================================================

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

	for (IConsoleCommand* Command : Commands)
	{
		if (Command)
			IConsoleManager::Get().UnregisterConsoleObject(Command);
	}
	Commands.Empty();
}
