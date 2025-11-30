// Copyright Epic Games, Inc. All Rights Reserved.
		
using UnrealBuildTool;
using System.Collections.Generic;
		
public class UltimaProjectServerTarget : TargetRules 
{
	public UltimaProjectServerTarget(TargetInfo Target) : base(Target) 
	{
		Type = TargetType.Server;
		DefaultBuildSettings = BuildSettingsVersion.V2;
		ExtraModuleNames.AddRange(new string[] { "UltimaProject", "UMG", "NetCore" });
		bWithPushModel = true;
	}
}