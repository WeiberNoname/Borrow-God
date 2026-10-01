using UnrealBuildTool;
using System.Collections.Generic;
public class BorrowedGodsTarget : TargetRules
{
    public BorrowedGodsTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Game;
        DefaultBuildSettings = BuildSettingsVersion.V6;
        IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_7;
        ExtraModuleNames.Add("BorrowedGods");
    }
}
