// Copyright 2024 PromptGameGenerator Contributors. All Rights Reserved.

#include "PromptGameGeneratorModule.h"
#include "PromptGameGeneratorCommands.h"
#include "SPromptGameGeneratorWidget.h"
#include "ToolMenus.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "LevelEditor.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"

#define LOCTEXT_NAMESPACE "FPromptGameGeneratorModule"

static const FName PromptGameGeneratorTabName("PromptGameGenerator");
const FName FPromptGameGeneratorModule::PromptGameGeneratorTabName("PromptGameGenerator");

void FPromptGameGeneratorModule::StartupModule()
{
	FPromptGameGeneratorCommands::Register();

	PluginCommands = MakeShareable(new FUICommandList);
	PluginCommands->MapAction(
		FPromptGameGeneratorCommands::Get().OpenPluginWindow,
		FExecuteAction::CreateRaw(this, &FPromptGameGeneratorModule::PluginButtonClicked),
		FCanExecuteAction());

	UToolMenus::RegisterStartupCallback(
		FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FPromptGameGeneratorModule::RegisterMenus));

	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(
		PromptGameGeneratorTabName,
		FOnSpawnTab::CreateRaw(this, &FPromptGameGeneratorModule::OnSpawnPluginTab))
		.SetDisplayName(LOCTEXT("TabTitle", "Prompt Game Generator"))
		.SetMenuType(ETabSpawnerMenuType::Hidden);

	UE_LOG(LogTemp, Log, TEXT("PromptGameGenerator: Plugin loaded successfully."));
}

void FPromptGameGeneratorModule::ShutdownModule()
{
	UToolMenus::UnRegisterStartupCallback(this);
	UToolMenus::UnregisterOwner(this);

	FPromptGameGeneratorCommands::Unregister();

	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(PromptGameGeneratorTabName);
}

TSharedRef<SDockTab> FPromptGameGeneratorModule::OnSpawnPluginTab(const FSpawnTabArgs& SpawnTabArgs)
{
	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			SNew(SBox)
			.MinDesiredWidth(450)
			.MinDesiredHeight(600)
			[
				SNew(SPromptGameGeneratorWidget)
			]
		];
}

void FPromptGameGeneratorModule::PluginButtonClicked()
{
	FGlobalTabmanager::Get()->TryInvokeTab(PromptGameGeneratorTabName);
}

void FPromptGameGeneratorModule::RegisterMenus()
{
	FToolMenuOwnerScoped OwnerScoped(this);

	// Add to the main toolbar
	UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar.PlayToolBar");
	{
		FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("PromptGameGenerator");
		FToolMenuEntry& Entry = Section.AddEntry(
			FToolMenuEntry::InitToolBarButton(FPromptGameGeneratorCommands::Get().OpenPluginWindow));
		Entry.SetCommandList(PluginCommands);
	}

	// Add to Window menu
	UToolMenu* WindowMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
	{
		FToolMenuSection& Section = WindowMenu->FindOrAddSection("PromptGameGenerator");
		Section.AddMenuEntryWithCommandList(
			FPromptGameGeneratorCommands::Get().OpenPluginWindow,
			PluginCommands);
	}
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FPromptGameGeneratorModule, PromptGameGenerator)
