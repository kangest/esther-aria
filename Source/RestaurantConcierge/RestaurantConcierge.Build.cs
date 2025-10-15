using UnrealBuildTool;

public class RestaurantConcierge : ModuleRules
{
    public RestaurantConcierge(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[] { 
            "Core", 
            "CoreUObject", 
            "Engine", 
            "InputCore",
            "UMG",
            "Slate",
            "SlateCore",
            "HTTP",
            "Json",
            "JsonUtilities",
            "AudioMixer",
            "AudioCapture",
            "SignalProcessing"
        });

        PrivateDependencyModuleNames.AddRange(new string[] { 
            "HTTP",
            "Json",
            "JsonUtilities",
            "AudioMixer",
            "AudioCapture"
        });

        // AWS SDK integration (will be added when available)
        if (Target.Platform == UnrealTargetPlatform.Win64 || Target.Platform == UnrealTargetPlatform.Mac)
        {
            // Add AWS SDK paths when configured
            // PublicIncludePaths.Add("ThirdParty/AWS/include");
            // PublicLibraryPaths.Add("ThirdParty/AWS/lib");
        }

        // Uncomment if you are using online features
        // PrivateDependencyModuleNames.Add("OnlineSubsystem");

        // To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
    }
}