// Copyright 2018 Sebastien Rombauts (sebastien.rombauts@gmail.com)

using UnrealBuildTool;
using System.Collections.Generic;

public class CoopGameTarget : TargetRules
{
	public CoopGameTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;

		ExtraModuleNames.AddRange( new string[] { "CoopGame" } );
	}
}
