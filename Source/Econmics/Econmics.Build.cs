// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;
using System.IO;

public class Econmics : ModuleRules
{
	public Econmics(TargetInfo Target)
	{
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore" });

		PrivateDependencyModuleNames.AddRange(new string[] {  });

        // Uncomment if you are using Slate UI
        // PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

        // Uncomment if you are using online features
        // PrivateDependencyModuleNames.Add("OnlineSubsystem");

        // To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true

        // Vyakhorev
        LoadPythonMagic(Target);

    }

    public bool LoadPythonMagic(TargetInfo Target)
    {
        bool isLibrarySupported = false;

        if (Target.Platform == UnrealTargetPlatform.Win64)
        {

            isLibrarySupported = true;

            // Go up two times
            var basePath = Directory.GetParent(Directory.GetParent(this.ModuleDirectory.ToString()).ToString()).ToString();

            string pythonHome = Path.Combine(basePath, "ThirdParty", "Python");
            System.Console.WriteLine("using python libraries and includes at: " + pythonHome);
            string librariesPath = Path.Combine(pythonHome, "libs");
            string includesPath = Path.Combine(pythonHome, "include");

            PublicLibraryPaths.Add(librariesPath);
            PublicIncludePaths.Add(includesPath);

        }

        return isLibrarySupported;

    }

    }
