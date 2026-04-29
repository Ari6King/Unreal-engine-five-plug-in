// Copyright 2024 PromptGameGenerator Contributors. All Rights Reserved.

#include "GameGenerator.h"

UGameGenerator::UGameGenerator()
{
}

void UGameGenerator::Initialize(const FLLMSettings& LLMSettings)
{
	LLMClient = NewObject<ULLMClient>(this);
	LLMClient->Initialize(LLMSettings);

	WorldBuilder = NewObject<UWorldBuilder>(this);
	WorldBuilder->OnProgress.BindUObject(this, &UGameGenerator::OnBuildProgress);
	WorldBuilder->OnComplete.BindUObject(this, &UGameGenerator::OnBuildComplete);
}

void UGameGenerator::GenerateFromPrompt(const FString& UserPrompt, UWorld* InTargetWorld)
{
	if (CurrentState == EGenerationState::SendingPrompt ||
		CurrentState == EGenerationState::WaitingForResponse ||
		CurrentState == EGenerationState::BuildingLevel)
	{
		UE_LOG(LogTemp, Warning, TEXT("PromptGameGenerator: Generation already in progress."));
		return;
	}

	TargetWorld = InTargetWorld;
	CurrentState = EGenerationState::SendingPrompt;

	OnProgress.ExecuteIfBound(0.0f, TEXT("Sending prompt to AI..."));

	FString SystemPrompt = ULLMClient::GetGameGenerationSystemPrompt();

	FOnLLMResponseReceived ResponseDelegate;
	ResponseDelegate.BindUObject(this, &UGameGenerator::OnLLMResponse);

	CurrentState = EGenerationState::WaitingForResponse;
	OnProgress.ExecuteIfBound(0.05f, TEXT("Waiting for AI response..."));

	LLMClient->SendPrompt(SystemPrompt, UserPrompt, ResponseDelegate);
}

void UGameGenerator::CancelGeneration()
{
	if (LLMClient)
	{
		LLMClient->CancelRequest();
	}
	CurrentState = EGenerationState::Idle;
	OnProgress.ExecuteIfBound(0.0f, TEXT("Generation cancelled."));
}

void UGameGenerator::OnLLMResponse(const FString& Response, bool bSuccess)
{
	if (!bSuccess)
	{
		CurrentState = EGenerationState::Error;
		OnComplete.ExecuteIfBound(false, FString::Printf(TEXT("LLM Error: %s"), *Response));
		return;
	}

	LastRawResponse = Response;
	CurrentState = EGenerationState::ParsingResponse;
	OnProgress.ExecuteIfBound(0.3f, TEXT("Parsing AI response..."));

	FGameLevelSpec Spec;
	if (!FGameLevelParser::ParseLevelSpec(Response, Spec))
	{
		CurrentState = EGenerationState::Error;
		OnComplete.ExecuteIfBound(false, TEXT("Failed to parse AI response into a level specification. The AI may have returned invalid JSON."));
		return;
	}

	LastLevelSpec = Spec;
	OnProgress.ExecuteIfBound(0.35f, FString::Printf(TEXT("Parsed level '%s' - Building world..."), *Spec.LevelName));

	BuildFromSpec(Spec);
}

void UGameGenerator::BuildFromSpec(const FGameLevelSpec& Spec)
{
	if (!TargetWorld)
	{
		CurrentState = EGenerationState::Error;
		OnComplete.ExecuteIfBound(false, TEXT("No target world available. Please open a level first."));
		return;
	}

	CurrentState = EGenerationState::BuildingLevel;
	WorldBuilder->BuildLevel(TargetWorld, Spec);
}

void UGameGenerator::OnBuildProgress(float Progress, const FString& Status)
{
	float OverallProgress = 0.35f + (Progress * 0.65f);
	OnProgress.ExecuteIfBound(OverallProgress, Status);
}

void UGameGenerator::OnBuildComplete(bool bSuccess)
{
	if (bSuccess)
	{
		CurrentState = EGenerationState::Complete;
		OnComplete.ExecuteIfBound(true, FString::Printf(
			TEXT("Successfully generated level '%s' with %d actors!"),
			*LastLevelSpec.LevelName, LastLevelSpec.Actors.Num()));
	}
	else
	{
		CurrentState = EGenerationState::Error;
		OnComplete.ExecuteIfBound(false, TEXT("Failed to build level in the editor."));
	}
}
