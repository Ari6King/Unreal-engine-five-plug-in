// Copyright 2024 PromptGameGenerator Contributors. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Http.h"
#include "Dom/JsonObject.h"
#include "LLMClient.generated.h"

DECLARE_DELEGATE_TwoParams(FOnLLMResponseReceived, const FString& /*Response*/, bool /*bSuccess*/);

UENUM(BlueprintType)
enum class ELLMProvider : uint8
{
	OpenAI		UMETA(DisplayName = "OpenAI (GPT-4)"),
	Anthropic	UMETA(DisplayName = "Anthropic (Claude)"),
	Local		UMETA(DisplayName = "Local LLM (Ollama)")
};

USTRUCT(BlueprintType)
struct FLLMSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LLM")
	ELLMProvider Provider = ELLMProvider::OpenAI;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LLM")
	FString APIKey;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LLM")
	FString Model = TEXT("gpt-4");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LLM")
	FString CustomEndpoint;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LLM")
	float Temperature = 0.7f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LLM")
	int32 MaxTokens = 4096;
};

UCLASS()
class PROMPTGAMEGENERATOR_API ULLMClient : public UObject
{
	GENERATED_BODY()

public:
	ULLMClient();

	void Initialize(const FLLMSettings& InSettings);

	void SendPrompt(const FString& SystemPrompt, const FString& UserPrompt, FOnLLMResponseReceived OnResponse);

	void SetSettings(const FLLMSettings& InSettings) { Settings = InSettings; }
	const FLLMSettings& GetSettings() const { return Settings; }

	bool IsRequestInProgress() const { return bRequestInProgress; }
	void CancelRequest();

	static FString GetGameGenerationSystemPrompt();

private:
	void SendOpenAIRequest(const FString& SystemPrompt, const FString& UserPrompt, FOnLLMResponseReceived OnResponse);
	void SendAnthropicRequest(const FString& SystemPrompt, const FString& UserPrompt, FOnLLMResponseReceived OnResponse);
	void SendLocalRequest(const FString& SystemPrompt, const FString& UserPrompt, FOnLLMResponseReceived OnResponse);

	void HandleResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful, FOnLLMResponseReceived OnResponse);

	FLLMSettings Settings;
	bool bRequestInProgress = false;
	TSharedPtr<IHttpRequest, ESPMode::ThreadSafe> CurrentRequest;
};
