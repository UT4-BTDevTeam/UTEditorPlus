
namespace UnrealBuildTool.Rules
{
	public class UTEditorPlus : ModuleRules
	{
		public UTEditorPlus(TargetInfo Target)
        {
            PrivateIncludePaths.Add("UTEditorPlus/Private");

			PublicDependencyModuleNames.AddRange(
				new string[]
				{
					"Core",
					"CoreUObject",
                    "Engine",
                    "BlueprintGraph"
                }
			);
		}
	}
}