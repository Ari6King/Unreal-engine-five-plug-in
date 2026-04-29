// Copyright 2024 PromptGameGenerator Contributors. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Input/SMultiLineEditableTextBox.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Notifications/SProgressBar.h"
#include "GameGenerator.h"
#include "LLMClient.h"
#include "PropChecklist.h"
#include "SPropChecklistWidget.h"

class PROMPTGAMEGENERATOR_API SPromptGameGeneratorWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SPromptGameGeneratorWidget) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

private:
	// UI Callbacks
	FReply OnGenerateClicked();
	FReply OnCancelClicked();
	FReply OnClearClicked();
	FReply OnSettingsToggleClicked();
	FReply OnSaveSettingsClicked();

	void OnPromptTextChanged(const FText& NewText);
	void OnAPIKeyChanged(const FText& NewText);
	void OnModelChanged(const FText& NewText);
	void OnCustomEndpointChanged(const FText& NewText);

	// Generation callbacks
	void OnGenerationProgress(float Progress, const FString& Status);
	void OnGenerationComplete(bool bSuccess, const FString& Message);

	// Helpers
	TSharedRef<SWidget> BuildPromptSection();
	TSharedRef<SWidget> BuildSettingsSection();
	TSharedRef<SWidget> BuildProgressSection();
	TSharedRef<SWidget> BuildLogSection();
	TSharedRef<SWidget> BuildPresetButtons();
	TSharedRef<SWidget> BuildPropChecklistSection();

	void AppendLog(const FString& Message);
	void LoadSettings();
	void SaveSettings();
	FString GetSettingsFilePath() const;

	// Prop checklist
	void ShowPropChecklist();
	void OnPropSelectionConfirmed();
	void OnPropSelectionCancelled();
	void ProceedWithGeneration();

	// State
	UPROPERTY()
	UGameGenerator* GameGenerator;

	FLLMSettings LLMSettings;
	FText PromptText;
	FString LogText;
	float CurrentProgress = 0.0f;
	FString StatusText = TEXT("Ready");
	bool bSettingsVisible = false;
	bool bIsGenerating = false;
	bool bChecklistVisible = false;

	UPROPERTY()
	UPropScanner* PropScanner;

	// Widget refs
	TSharedPtr<SMultiLineEditableTextBox> PromptTextBox;
	TSharedPtr<SMultiLineEditableTextBox> LogTextBox;
	TSharedPtr<SProgressBar> ProgressBar;
	TSharedPtr<STextBlock> StatusTextBlock;
	TSharedPtr<SBox> SettingsBox;
	TSharedPtr<SEditableTextBox> APIKeyTextBox;
	TSharedPtr<SEditableTextBox> ModelTextBox;
	TSharedPtr<SEditableTextBox> EndpointTextBox;
	TSharedPtr<SButton> GenerateButton;
	TSharedPtr<SButton> CancelButton;
	TSharedPtr<SBox> ChecklistContainer;
};
