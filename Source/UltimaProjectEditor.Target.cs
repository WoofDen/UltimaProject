// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class UltimaProjectEditorTarget : TargetRules
{
	public UltimaProjectEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		
		
		ExtraModuleNames.AddRange(new string[]
		{
			"UltimaProject", "UMG", "NetCore"
		});
	}
}