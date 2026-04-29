// Copyright 2024 PromptGameGenerator Contributors. All Rights Reserved.

#include "PromptGameGeneratorCommands.h"

#define LOCTEXT_NAMESPACE "FPromptGameGeneratorModule"

void FPromptGameGeneratorCommands::RegisterCommands()
{
	UI_COMMAND(OpenPluginWindow,
		"Prompt Game Generator",
		"Open the Prompt Game Generator panel to create games from text descriptions",
		EUserInterfaceActionType::Button,
		FInputChord());
}

#undef LOCTEXT_NAMESPACE
