// Copyright 2024 PromptGameGenerator Contributors. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "LLMClient.h"
#include "WorldBuilder.h"
#include "GameGenerationTypes.h"
#include "GameLevelParser.h"
#include "GameGenerator.generated.h"

UENUM(BlueprintType)
enum class EGenerationState : uint8
{
	Idle,
	SendingPrompt,
	WaitingForResponse,
	ParsingResponse,
	BuildingLevel,
	Complete,
	Error
};

DECLARE_DELEGATE_TwoParams(FOnGenerationProgress, float /*Progress*/, const FString& /*Status*/);
DECLARE_DELEGATE_TwoParams(FOnGenerationComplete, bool /*bSuccess*/, const FString& /*Message*/);

UCLASS()
class PROMPTGAMEGENERATOR_API UGameGenerator : public UObject
{
	GENERATED_BODY()

public:
	UGameGenerator();

	void Initialize(const FLLMSettings& LLMSettings);

	void GenerateFromPrompt(const FString& UserPrompt, UWorld* TargetWorld);
	void CancelGeneration();

	EGenerationState GetState() const { return CurrentState; }
	const FGameLevelSpec& GetLastLevelSpec() const { return LastLevelSpec; }

	FOnGenerationProgress OnProgress;
	FOnGenerationComplete OnComplete;

	UPROPERTY()
	ULLMClient* LLMClient;

	UPROPERTY()
	UWorldBuilder* WorldBuilder;

private:
	void OnLLMResponse(const FString& Response, bool bSuccess);
	void BuildFromSpec(const FGameLevelSpec& Spec);
	void OnBuildProgress(float Progress, const FString& Status);
	void OnBuildComplete(bool bSuccess);

	EGenerationState CurrentState = EGenerationState::Idle;
	FGameLevelSpec LastLevelSpec;

	UPROPERTY()
	UWorld* TargetWorld;

	FString LastRawResponse;
};
