// Copyright 2024 PromptGameGenerator Contributors. All Rights Reserved.

#include "SPromptGameGeneratorWidget.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SExpandableArea.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SSlider.h"
#include "Editor.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"

#define LOCTEXT_NAMESPACE "PromptGameGenerator"

void SPromptGameGeneratorWidget::Construct(const FArguments& InArgs)
{
	LoadSettings();

	GameGenerator = NewObject<UGameGenerator>();
	GameGenerator->AddToRoot();
	GameGenerator->Initialize(LLMSettings);
	GameGenerator->OnProgress.BindRaw(this, &SPromptGameGeneratorWidget::OnGenerationProgress);
	GameGenerator->OnComplete.BindRaw(this, &SPromptGameGeneratorWidget::OnGenerationComplete);

	ChildSlot
	[
		SNew(SScrollBox)
		+ SScrollBox::Slot()
		.Padding(8)
		[
			SNew(SVerticalBox)

			// Title
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 0, 0, 8)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("Title", "🎮 Prompt Game Generator"))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 18))
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 0, 0, 4)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("Subtitle", "Describe your game and AI will build it in Unreal Engine"))
				.Font(FCoreStyle::GetDefaultFontStyle("Regular", 10))
				.ColorAndOpacity(FSlateColor(FLinearColor(0.6f, 0.6f, 0.6f)))
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 4, 0, 8)
			[
				SNew(SSeparator)
			]

			// Settings toggle
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 0, 0, 8)
			[
				BuildSettingsSection()
			]

			// Preset buttons
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 0, 0, 8)
			[
				BuildPresetButtons()
			]

			// Prompt section
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 0, 0, 8)
			[
				BuildPromptSection()
			]

			// Progress section
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 0, 0, 8)
			[
				BuildProgressSection()
			]

			// Log section
			+ SVerticalBox::Slot()
			.FillHeight(1.0f)
			.Padding(0, 0, 0, 4)
			[
				BuildLogSection()
			]
		]
	];

	AppendLog(TEXT("Plugin initialized. Configure your API key in Settings, then describe your game!"));
}

TSharedRef<SWidget> SPromptGameGeneratorWidget::BuildPromptSection()
{
	return SNew(SVerticalBox)

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0, 0, 0, 4)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("PromptLabel", "Game Description"))
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 11))
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.MaxHeight(200)
		[
			SAssignNew(PromptTextBox, SMultiLineEditableTextBox)
			.HintText(LOCTEXT("PromptHint",
				"Describe your game here...\n\n"
				"Examples:\n"
				"- A medieval castle with a moat, drawbridge, and guard towers\n"
				"- A space station orbiting a red planet with airlocks and corridors\n"
				"- A forest platformer with floating islands and glowing mushrooms"))
			.OnTextChanged(this, &SPromptGameGeneratorWidget::OnPromptTextChanged)
			.AutoWrapText(true)
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0, 8, 0, 0)
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			[
				SAssignNew(GenerateButton, SButton)
				.HAlign(HAlign_Center)
				.OnClicked(this, &SPromptGameGeneratorWidget::OnGenerateClicked)
				.IsEnabled_Lambda([this]() { return !bIsGenerating && !PromptText.IsEmpty(); })
				[
					SNew(STextBlock)
					.Text(LOCTEXT("GenerateBtn", "🚀 Generate Game"))
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
					.Justification(ETextJustify::Center)
				]
			]

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(4, 0, 0, 0)
			[
				SAssignNew(CancelButton, SButton)
				.HAlign(HAlign_Center)
				.OnClicked(this, &SPromptGameGeneratorWidget::OnCancelClicked)
				.IsEnabled_Lambda([this]() { return bIsGenerating; })
				[
					SNew(STextBlock)
					.Text(LOCTEXT("CancelBtn", "Cancel"))
				]
			]

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(4, 0, 0, 0)
			[
				SNew(SButton)
				.HAlign(HAlign_Center)
				.OnClicked(this, &SPromptGameGeneratorWidget::OnClearClicked)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("ClearBtn", "Clear"))
				]
			]
		];
}

TSharedRef<SWidget> SPromptGameGeneratorWidget::BuildSettingsSection()
{
	return SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SButton)
			.OnClicked(this, &SPromptGameGeneratorWidget::OnSettingsToggleClicked)
			[
				SNew(STextBlock)
				.Text_Lambda([this]()
				{
					return bSettingsVisible
						? LOCTEXT("HideSettings", "⚙ Hide Settings")
						: LOCTEXT("ShowSettings", "⚙ Show Settings");
				})
			]
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SAssignNew(SettingsBox, SBox)
			.Visibility_Lambda([this]() { return bSettingsVisible ? EVisibility::Visible : EVisibility::Collapsed; })
			[
				SNew(SVerticalBox)

				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0, 8, 0, 4)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("APIKeyLabel", "API Key"))
					.Font(FCoreStyle::GetDefaultFontStyle("Regular", 9))
				]

				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SAssignNew(APIKeyTextBox, SEditableTextBox)
					.IsPassword(true)
					.HintText(LOCTEXT("APIKeyHint", "Enter your OpenAI/Anthropic API key"))
					.Text(FText::FromString(LLMSettings.APIKey))
					.OnTextChanged(this, &SPromptGameGeneratorWidget::OnAPIKeyChanged)
				]

				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0, 8, 0, 4)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("ModelLabel", "Model"))
					.Font(FCoreStyle::GetDefaultFontStyle("Regular", 9))
				]

				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SAssignNew(ModelTextBox, SEditableTextBox)
					.HintText(LOCTEXT("ModelHint", "e.g., gpt-4, gpt-4o, claude-3-sonnet-20240229"))
					.Text(FText::FromString(LLMSettings.Model))
					.OnTextChanged(this, &SPromptGameGeneratorWidget::OnModelChanged)
				]

				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0, 8, 0, 4)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("EndpointLabel", "Custom Endpoint (optional)"))
					.Font(FCoreStyle::GetDefaultFontStyle("Regular", 9))
				]

				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SAssignNew(EndpointTextBox, SEditableTextBox)
					.HintText(LOCTEXT("EndpointHint", "Leave empty for default, or enter custom URL"))
					.Text(FText::FromString(LLMSettings.CustomEndpoint))
					.OnTextChanged(this, &SPromptGameGeneratorWidget::OnCustomEndpointChanged)
				]

				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0, 8, 0, 0)
				[
					SNew(SButton)
					.HAlign(HAlign_Center)
					.OnClicked(this, &SPromptGameGeneratorWidget::OnSaveSettingsClicked)
					[
						SNew(STextBlock)
						.Text(LOCTEXT("SaveSettings", "Save Settings"))
					]
				]
			]
		];
}

TSharedRef<SWidget> SPromptGameGeneratorWidget::BuildProgressSection()
{
	return SNew(SVerticalBox)

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0, 0, 0, 4)
		[
			SAssignNew(StatusTextBlock, STextBlock)
			.Text_Lambda([this]() { return FText::FromString(StatusText); })
			.Font(FCoreStyle::GetDefaultFontStyle("Regular", 10))
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SAssignNew(ProgressBar, SProgressBar)
			.Percent_Lambda([this]() { return CurrentProgress; })
		];
}

TSharedRef<SWidget> SPromptGameGeneratorWidget::BuildLogSection()
{
	return SNew(SVerticalBox)

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0, 0, 0, 4)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("LogLabel", "Generation Log"))
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 10))
		]

		+ SVerticalBox::Slot()
		.FillHeight(1.0f)
		.MinDesiredHeight(150)
		[
			SAssignNew(LogTextBox, SMultiLineEditableTextBox)
			.IsReadOnly(true)
			.AutoWrapText(true)
			.Text_Lambda([this]() { return FText::FromString(LogText); })
		];
}

TSharedRef<SWidget> SPromptGameGeneratorWidget::BuildPresetButtons()
{
	return SNew(SVerticalBox)

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0, 0, 0, 4)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("PresetsLabel", "Quick Presets"))
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 10))
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			.Padding(0, 0, 2, 0)
			[
				SNew(SButton)
				.HAlign(HAlign_Center)
				.OnClicked_Lambda([this]()
				{
					PromptText = FText::FromString(TEXT("A medieval fantasy castle on a hilltop with stone walls, a moat with a drawbridge, four guard towers with torches, a courtyard with training dummies, and a throne room entrance. Sunset lighting with dramatic clouds."));
					PromptTextBox->SetText(PromptText);
					return FReply::Handled();
				})
				[
					SNew(STextBlock)
					.Text(LOCTEXT("Preset1", "Medieval Castle"))
					.Font(FCoreStyle::GetDefaultFontStyle("Regular", 9))
				]
			]

			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			.Padding(2, 0)
			[
				SNew(SButton)
				.HAlign(HAlign_Center)
				.OnClicked_Lambda([this]()
				{
					PromptText = FText::FromString(TEXT("A sci-fi space station interior with metallic corridors, airlock doors, a command bridge with holographic displays, an engine room with glowing reactors, crew quarters, and zero-gravity cargo bay. Blue and orange accent lighting."));
					PromptTextBox->SetText(PromptText);
					return FReply::Handled();
				})
				[
					SNew(STextBlock)
					.Text(LOCTEXT("Preset2", "Space Station"))
					.Font(FCoreStyle::GetDefaultFontStyle("Regular", 9))
				]
			]

			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			.Padding(2, 0, 0, 0)
			[
				SNew(SButton)
				.HAlign(HAlign_Center)
				.OnClicked_Lambda([this]()
				{
					PromptText = FText::FromString(TEXT("An enchanted forest platformer level with floating moss-covered islands connected by wooden bridges, giant glowing mushrooms, a crystal-clear waterfall, ancient tree houses, and collectible fairy orbs scattered throughout. Magical twilight atmosphere with fireflies."));
					PromptTextBox->SetText(PromptText);
					return FReply::Handled();
				})
				[
					SNew(STextBlock)
					.Text(LOCTEXT("Preset3", "Magic Forest"))
					.Font(FCoreStyle::GetDefaultFontStyle("Regular", 9))
				]
			]
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0, 4, 0, 0)
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			.Padding(0, 0, 2, 0)
			[
				SNew(SButton)
				.HAlign(HAlign_Center)
				.OnClicked_Lambda([this]()
				{
					PromptText = FText::FromString(TEXT("A post-apocalyptic urban survival level with crumbling skyscrapers, abandoned vehicles, makeshift barricades, a rooftop safe house, supply caches, and enemy spawn points in dark alleys. Overcast sky with toxic green fog."));
					PromptTextBox->SetText(PromptText);
					return FReply::Handled();
				})
				[
					SNew(STextBlock)
					.Text(LOCTEXT("Preset4", "Post-Apocalypse"))
					.Font(FCoreStyle::GetDefaultFontStyle("Regular", 9))
				]
			]

			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			.Padding(2, 0)
			[
				SNew(SButton)
				.HAlign(HAlign_Center)
				.OnClicked_Lambda([this]()
				{
					PromptText = FText::FromString(TEXT("An ancient Egyptian pyramid puzzle temple with sand-covered stone corridors, hidden trap rooms, a pharaoh's burial chamber with golden sarcophagus, torch-lit passages, sliding block puzzles, and treasure rooms with collectible artifacts."));
					PromptTextBox->SetText(PromptText);
					return FReply::Handled();
				})
				[
					SNew(STextBlock)
					.Text(LOCTEXT("Preset5", "Pyramid Temple"))
					.Font(FCoreStyle::GetDefaultFontStyle("Regular", 9))
				]
			]

			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			.Padding(2, 0, 0, 0)
			[
				SNew(SButton)
				.HAlign(HAlign_Center)
				.OnClicked_Lambda([this]()
				{
					PromptText = FText::FromString(TEXT("An underwater ocean exploration level with coral reef formations, sunken pirate ship wreckage, bioluminescent deep-sea caves, schools of fish areas, treasure chests, ancient underwater ruins with stone columns, and a deep trench with mysterious glowing artifacts."));
					PromptTextBox->SetText(PromptText);
					return FReply::Handled();
				})
				[
					SNew(STextBlock)
					.Text(LOCTEXT("Preset6", "Underwater World"))
					.Font(FCoreStyle::GetDefaultFontStyle("Regular", 9))
				]
			]
		];
}

FReply SPromptGameGeneratorWidget::OnGenerateClicked()
{
	if (PromptText.IsEmpty())
	{
		AppendLog(TEXT("ERROR: Please enter a game description first."));
		return FReply::Handled();
	}

	if (LLMSettings.APIKey.IsEmpty() && LLMSettings.Provider != ELLMProvider::Local)
	{
		AppendLog(TEXT("ERROR: Please set your API key in Settings first."));
		bSettingsVisible = true;
		return FReply::Handled();
	}

	bIsGenerating = true;
	CurrentProgress = 0.0f;
	StatusText = TEXT("Starting generation...");

	AppendLog(TEXT("---"));
	AppendLog(FString::Printf(TEXT("Generating from prompt: %s"), *PromptText.ToString()));

	UWorld* World = GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
	if (!World)
	{
		AppendLog(TEXT("ERROR: No editor world available. Please open a level."));
		bIsGenerating = false;
		return FReply::Handled();
	}

	GameGenerator->Initialize(LLMSettings);
	GameGenerator->GenerateFromPrompt(PromptText.ToString(), World);

	return FReply::Handled();
}

FReply SPromptGameGeneratorWidget::OnCancelClicked()
{
	if (GameGenerator)
	{
		GameGenerator->CancelGeneration();
	}
	bIsGenerating = false;
	CurrentProgress = 0.0f;
	StatusText = TEXT("Cancelled");
	AppendLog(TEXT("Generation cancelled by user."));
	return FReply::Handled();
}

FReply SPromptGameGeneratorWidget::OnClearClicked()
{
	PromptText = FText::GetEmpty();
	if (PromptTextBox.IsValid())
	{
		PromptTextBox->SetText(PromptText);
	}
	LogText.Empty();
	CurrentProgress = 0.0f;
	StatusText = TEXT("Ready");
	return FReply::Handled();
}

FReply SPromptGameGeneratorWidget::OnSettingsToggleClicked()
{
	bSettingsVisible = !bSettingsVisible;
	return FReply::Handled();
}

FReply SPromptGameGeneratorWidget::OnSaveSettingsClicked()
{
	SaveSettings();
	AppendLog(TEXT("Settings saved successfully."));

	FNotificationInfo Info(LOCTEXT("SettingsSaved", "Settings saved!"));
	Info.ExpireDuration = 2.0f;
	FSlateNotificationManager::Get().AddNotification(Info);

	return FReply::Handled();
}

void SPromptGameGeneratorWidget::OnPromptTextChanged(const FText& NewText)
{
	PromptText = NewText;
}

void SPromptGameGeneratorWidget::OnAPIKeyChanged(const FText& NewText)
{
	LLMSettings.APIKey = NewText.ToString();
}

void SPromptGameGeneratorWidget::OnModelChanged(const FText& NewText)
{
	LLMSettings.Model = NewText.ToString();
}

void SPromptGameGeneratorWidget::OnCustomEndpointChanged(const FText& NewText)
{
	LLMSettings.CustomEndpoint = NewText.ToString();
}

void SPromptGameGeneratorWidget::OnGenerationProgress(float Progress, const FString& Status)
{
	CurrentProgress = Progress;
	StatusText = Status;
	AppendLog(FString::Printf(TEXT("[%.0f%%] %s"), Progress * 100.0f, *Status));
}

void SPromptGameGeneratorWidget::OnGenerationComplete(bool bSuccess, const FString& Message)
{
	bIsGenerating = false;

	if (bSuccess)
	{
		CurrentProgress = 1.0f;
		StatusText = TEXT("Generation complete!");
		AppendLog(FString::Printf(TEXT("SUCCESS: %s"), *Message));

		FNotificationInfo Info(FText::FromString(Message));
		Info.ExpireDuration = 5.0f;
		FSlateNotificationManager::Get().AddNotification(Info);
	}
	else
	{
		CurrentProgress = 0.0f;
		StatusText = TEXT("Generation failed");
		AppendLog(FString::Printf(TEXT("FAILED: %s"), *Message));

		FNotificationInfo Info(FText::FromString(FString::Printf(TEXT("Generation failed: %s"), *Message)));
		Info.ExpireDuration = 8.0f;
		FSlateNotificationManager::Get().AddNotification(Info);
	}
}

void SPromptGameGeneratorWidget::AppendLog(const FString& Message)
{
	FDateTime Now = FDateTime::Now();
	FString Timestamp = Now.ToString(TEXT("[%H:%M:%S] "));
	LogText += Timestamp + Message + TEXT("\n");
}

FString SPromptGameGeneratorWidget::GetSettingsFilePath() const
{
	return FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("PromptGameGenerator"), TEXT("Settings.json"));
}

void SPromptGameGeneratorWidget::LoadSettings()
{
	FString SettingsPath = GetSettingsFilePath();
	FString JsonString;

	if (FFileHelper::LoadFileToString(JsonString, *SettingsPath))
	{
		TSharedPtr<FJsonObject> JsonObject;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);

		if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
		{
			LLMSettings.APIKey = JsonObject->GetStringField(TEXT("api_key"));
			LLMSettings.Model = JsonObject->GetStringField(TEXT("model"));
			LLMSettings.CustomEndpoint = JsonObject->GetStringField(TEXT("custom_endpoint"));

			double Temp;
			if (JsonObject->TryGetNumberField(TEXT("temperature"), Temp))
				LLMSettings.Temperature = static_cast<float>(Temp);

			double MaxTokens;
			if (JsonObject->TryGetNumberField(TEXT("max_tokens"), MaxTokens))
				LLMSettings.MaxTokens = static_cast<int32>(MaxTokens);

			int32 Provider;
			if (JsonObject->TryGetNumberField(TEXT("provider"), Provider))
				LLMSettings.Provider = static_cast<ELLMProvider>(Provider);

			UE_LOG(LogTemp, Log, TEXT("PromptGameGenerator: Settings loaded."));
		}
	}
	else
	{
		LLMSettings.Provider = ELLMProvider::OpenAI;
		LLMSettings.Model = TEXT("gpt-4");
		LLMSettings.Temperature = 0.7f;
		LLMSettings.MaxTokens = 4096;
	}
}

void SPromptGameGeneratorWidget::SaveSettings()
{
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject());
	JsonObject->SetStringField(TEXT("api_key"), LLMSettings.APIKey);
	JsonObject->SetStringField(TEXT("model"), LLMSettings.Model);
	JsonObject->SetStringField(TEXT("custom_endpoint"), LLMSettings.CustomEndpoint);
	JsonObject->SetNumberField(TEXT("temperature"), LLMSettings.Temperature);
	JsonObject->SetNumberField(TEXT("max_tokens"), LLMSettings.MaxTokens);
	JsonObject->SetNumberField(TEXT("provider"), static_cast<int32>(LLMSettings.Provider));

	FString JsonString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonString);
	FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

	FString SettingsPath = GetSettingsFilePath();
	FString Directory = FPaths::GetPath(SettingsPath);
	IFileManager::Get().MakeDirectory(*Directory, true);

	FFileHelper::SaveStringToFile(JsonString, *SettingsPath);
	UE_LOG(LogTemp, Log, TEXT("PromptGameGenerator: Settings saved to %s"), *SettingsPath);
}

#undef LOCTEXT_NAMESPACE
