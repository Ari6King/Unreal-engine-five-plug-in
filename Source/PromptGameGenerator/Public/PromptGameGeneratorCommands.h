// Copyright 2024 PromptGameGenerator Contributors. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "EditorStyleSet.h"

class FPromptGameGeneratorCommands : public TCommands<FPromptGameGeneratorCommands>
{
public:
	FPromptGameGeneratorCommands()
		: TCommands<FPromptGameGeneratorCommands>(
			TEXT("PromptGameGenerator"),
			NSLOCTEXT("Contexts", "PromptGameGenerator", "Prompt Game Generator Plugin"),
			NAME_None,
			FEditorStyle::GetStyleSetName())
	{
	}

	virtual void RegisterCommands() override;

	TSharedPtr<FUICommandInfo> OpenPluginWindow;
};
