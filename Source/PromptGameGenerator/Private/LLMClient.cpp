// Copyright 2024 PromptGameGenerator Contributors. All Rights Reserved.

#include "LLMClient.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

ULLMClient::ULLMClient()
{
}

void ULLMClient::Initialize(const FLLMSettings& InSettings)
{
	Settings = InSettings;
}

void ULLMClient::SendPrompt(const FString& SystemPrompt, const FString& UserPrompt, FOnLLMResponseReceived OnResponse)
{
	if (bRequestInProgress)
	{
		UE_LOG(LogTemp, Warning, TEXT("PromptGameGenerator: LLM request already in progress."));
		OnResponse.ExecuteIfBound(TEXT(""), false);
		return;
	}

	if (Settings.APIKey.IsEmpty() && Settings.Provider != ELLMProvider::Local)
	{
		UE_LOG(LogTemp, Error, TEXT("PromptGameGenerator: API key is not set."));
		OnResponse.ExecuteIfBound(TEXT("API key is not configured. Please set your API key in the plugin settings."), false);
		return;
	}

	switch (Settings.Provider)
	{
	case ELLMProvider::OpenAI:
		SendOpenAIRequest(SystemPrompt, UserPrompt, OnResponse);
		break;
	case ELLMProvider::Anthropic:
		SendAnthropicRequest(SystemPrompt, UserPrompt, OnResponse);
		break;
	case ELLMProvider::Local:
		SendLocalRequest(SystemPrompt, UserPrompt, OnResponse);
		break;
	}
}

void ULLMClient::SendOpenAIRequest(const FString& SystemPrompt, const FString& UserPrompt, FOnLLMResponseReceived OnResponse)
{
	FString Endpoint = Settings.CustomEndpoint.IsEmpty()
		? TEXT("https://api.openai.com/v1/chat/completions")
		: Settings.CustomEndpoint;

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
	Request->SetURL(Endpoint);
	Request->SetVerb(TEXT("POST"));
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	Request->SetHeader(TEXT("Authorization"), FString::Printf(TEXT("Bearer %s"), *Settings.APIKey));

	TSharedPtr<FJsonObject> RootObject = MakeShareable(new FJsonObject());
	RootObject->SetStringField(TEXT("model"), Settings.Model);
	RootObject->SetNumberField(TEXT("temperature"), Settings.Temperature);
	RootObject->SetNumberField(TEXT("max_tokens"), Settings.MaxTokens);

	TArray<TSharedPtr<FJsonValue>> Messages;

	TSharedPtr<FJsonObject> SystemMessage = MakeShareable(new FJsonObject());
	SystemMessage->SetStringField(TEXT("role"), TEXT("system"));
	SystemMessage->SetStringField(TEXT("content"), SystemPrompt);
	Messages.Add(MakeShareable(new FJsonValueObject(SystemMessage)));

	TSharedPtr<FJsonObject> UserMessage = MakeShareable(new FJsonObject());
	UserMessage->SetStringField(TEXT("role"), TEXT("user"));
	UserMessage->SetStringField(TEXT("content"), UserPrompt);
	Messages.Add(MakeShareable(new FJsonValueObject(UserMessage)));

	RootObject->SetArrayField(TEXT("messages"), Messages);

	TSharedPtr<FJsonObject> ResponseFormat = MakeShareable(new FJsonObject());
	ResponseFormat->SetStringField(TEXT("type"), TEXT("json_object"));
	RootObject->SetObjectField(TEXT("response_format"), ResponseFormat);

	FString RequestBody;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBody);
	FJsonSerializer::Serialize(RootObject.ToSharedRef(), Writer);

	Request->SetContentAsString(RequestBody);
	Request->OnProcessRequestComplete().BindUObject(this, &ULLMClient::HandleResponse, OnResponse);

	bRequestInProgress = true;
	CurrentRequest = Request;
	Request->ProcessRequest();

	UE_LOG(LogTemp, Log, TEXT("PromptGameGenerator: Sent request to OpenAI API."));
}

void ULLMClient::SendAnthropicRequest(const FString& SystemPrompt, const FString& UserPrompt, FOnLLMResponseReceived OnResponse)
{
	FString Endpoint = Settings.CustomEndpoint.IsEmpty()
		? TEXT("https://api.anthropic.com/v1/messages")
		: Settings.CustomEndpoint;

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
	Request->SetURL(Endpoint);
	Request->SetVerb(TEXT("POST"));
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	Request->SetHeader(TEXT("x-api-key"), Settings.APIKey);
	Request->SetHeader(TEXT("anthropic-version"), TEXT("2023-06-01"));

	TSharedPtr<FJsonObject> RootObject = MakeShareable(new FJsonObject());
	RootObject->SetStringField(TEXT("model"), Settings.Model.IsEmpty() ? TEXT("claude-3-sonnet-20240229") : Settings.Model);
	RootObject->SetNumberField(TEXT("max_tokens"), Settings.MaxTokens);
	RootObject->SetStringField(TEXT("system"), SystemPrompt);

	TArray<TSharedPtr<FJsonValue>> Messages;
	TSharedPtr<FJsonObject> UserMessage = MakeShareable(new FJsonObject());
	UserMessage->SetStringField(TEXT("role"), TEXT("user"));
	UserMessage->SetStringField(TEXT("content"), UserPrompt);
	Messages.Add(MakeShareable(new FJsonValueObject(UserMessage)));

	RootObject->SetArrayField(TEXT("messages"), Messages);

	FString RequestBody;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBody);
	FJsonSerializer::Serialize(RootObject.ToSharedRef(), Writer);

	Request->SetContentAsString(RequestBody);
	Request->OnProcessRequestComplete().BindUObject(this, &ULLMClient::HandleResponse, OnResponse);

	bRequestInProgress = true;
	CurrentRequest = Request;
	Request->ProcessRequest();

	UE_LOG(LogTemp, Log, TEXT("PromptGameGenerator: Sent request to Anthropic API."));
}

void ULLMClient::SendLocalRequest(const FString& SystemPrompt, const FString& UserPrompt, FOnLLMResponseReceived OnResponse)
{
	FString Endpoint = Settings.CustomEndpoint.IsEmpty()
		? TEXT("http://localhost:11434/api/chat")
		: Settings.CustomEndpoint;

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
	Request->SetURL(Endpoint);
	Request->SetVerb(TEXT("POST"));
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));

	TSharedPtr<FJsonObject> RootObject = MakeShareable(new FJsonObject());
	RootObject->SetStringField(TEXT("model"), Settings.Model.IsEmpty() ? TEXT("llama3") : Settings.Model);
	RootObject->SetBoolField(TEXT("stream"), false);
	RootObject->SetStringField(TEXT("format"), TEXT("json"));

	TArray<TSharedPtr<FJsonValue>> Messages;

	TSharedPtr<FJsonObject> SystemMessage = MakeShareable(new FJsonObject());
	SystemMessage->SetStringField(TEXT("role"), TEXT("system"));
	SystemMessage->SetStringField(TEXT("content"), SystemPrompt);
	Messages.Add(MakeShareable(new FJsonValueObject(SystemMessage)));

	TSharedPtr<FJsonObject> UserMessage = MakeShareable(new FJsonObject());
	UserMessage->SetStringField(TEXT("role"), TEXT("user"));
	UserMessage->SetStringField(TEXT("content"), UserPrompt);
	Messages.Add(MakeShareable(new FJsonValueObject(UserMessage)));

	RootObject->SetArrayField(TEXT("messages"), Messages);

	FString RequestBody;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBody);
	FJsonSerializer::Serialize(RootObject.ToSharedRef(), Writer);

	Request->SetContentAsString(RequestBody);
	Request->OnProcessRequestComplete().BindUObject(this, &ULLMClient::HandleResponse, OnResponse);

	bRequestInProgress = true;
	CurrentRequest = Request;
	Request->ProcessRequest();

	UE_LOG(LogTemp, Log, TEXT("PromptGameGenerator: Sent request to local LLM."));
}

void ULLMClient::HandleResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful, FOnLLMResponseReceived OnResponse)
{
	bRequestInProgress = false;
	CurrentRequest = nullptr;

	if (!bWasSuccessful || !Response.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("PromptGameGenerator: HTTP request failed."));
		OnResponse.ExecuteIfBound(TEXT("HTTP request failed. Check your network connection and API settings."), false);
		return;
	}

	int32 ResponseCode = Response->GetResponseCode();
	FString ResponseBody = Response->GetContentAsString();

	if (ResponseCode != 200)
	{
		UE_LOG(LogTemp, Error, TEXT("PromptGameGenerator: API returned error %d: %s"), ResponseCode, *ResponseBody);
		OnResponse.ExecuteIfBound(FString::Printf(TEXT("API error %d: %s"), ResponseCode, *ResponseBody), false);
		return;
	}

	TSharedPtr<FJsonObject> JsonResponse;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseBody);

	if (!FJsonSerializer::Deserialize(Reader, JsonResponse) || !JsonResponse.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("PromptGameGenerator: Failed to parse API response."));
		OnResponse.ExecuteIfBound(TEXT("Failed to parse API response."), false);
		return;
	}

	FString Content;

	if (Settings.Provider == ELLMProvider::OpenAI)
	{
		const TArray<TSharedPtr<FJsonValue>>* Choices;
		if (JsonResponse->TryGetArrayField(TEXT("choices"), Choices) && Choices->Num() > 0)
		{
			TSharedPtr<FJsonObject> Message = (*Choices)[0]->AsObject()->GetObjectField(TEXT("message"));
			Content = Message->GetStringField(TEXT("content"));
		}
	}
	else if (Settings.Provider == ELLMProvider::Anthropic)
	{
		const TArray<TSharedPtr<FJsonValue>>* ContentArray;
		if (JsonResponse->TryGetArrayField(TEXT("content"), ContentArray) && ContentArray->Num() > 0)
		{
			Content = (*ContentArray)[0]->AsObject()->GetStringField(TEXT("text"));
		}
	}
	else if (Settings.Provider == ELLMProvider::Local)
	{
		TSharedPtr<FJsonObject> Message = JsonResponse->GetObjectField(TEXT("message"));
		if (Message.IsValid())
		{
			Content = Message->GetStringField(TEXT("content"));
		}
	}

	if (Content.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("PromptGameGenerator: Empty response from LLM."));
		OnResponse.ExecuteIfBound(TEXT("Received empty response from LLM."), false);
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("PromptGameGenerator: Received LLM response (%d chars)."), Content.Len());
	OnResponse.ExecuteIfBound(Content, true);
}

void ULLMClient::CancelRequest()
{
	if (CurrentRequest.IsValid())
	{
		CurrentRequest->CancelRequest();
		CurrentRequest = nullptr;
		bRequestInProgress = false;
		UE_LOG(LogTemp, Log, TEXT("PromptGameGenerator: LLM request cancelled."));
	}
}

FString ULLMClient::GetGameGenerationSystemPrompt()
{
	return TEXT(R"(You are an Unreal Engine 5 game level designer AI. Given a user's game description prompt, generate a detailed JSON specification for a complete game level.

Your response MUST be valid JSON with the following structure:

{
  "level_name": "string - name for the level",
  "description": "string - brief description",
  "environment": {
    "type": "string - one of: outdoor, indoor, cave, space, underwater, city, forest, desert, arctic, volcanic",
    "time_of_day": "string - one of: morning, noon, afternoon, evening, night",
    "weather": "string - one of: clear, cloudy, rainy, snowy, foggy, stormy",
    "sky_color": {"r": 0.5, "g": 0.7, "b": 1.0},
    "ambient_intensity": 1.0,
    "fog_density": 0.02
  },
  "terrain": {
    "enabled": true,
    "size_x": 8192,
    "size_y": 8192,
    "height_scale": 256.0,
    "noise_scale": 0.005,
    "noise_octaves": 6,
    "material": "string - one of: grass, sand, snow, rock, dirt, lava, water_ground",
    "color": {"r": 0.3, "g": 0.6, "b": 0.2}
  },
  "lighting": {
    "directional_light": {
      "intensity": 10.0,
      "color": {"r": 1.0, "g": 0.95, "b": 0.85},
      "rotation": {"pitch": -45.0, "yaw": 180.0, "roll": 0.0}
    },
    "sky_light": {
      "intensity": 1.0,
      "color": {"r": 0.6, "g": 0.8, "b": 1.0}
    },
    "point_lights": [
      {
        "position": {"x": 0, "y": 0, "z": 300},
        "intensity": 5000.0,
        "color": {"r": 1.0, "g": 0.8, "b": 0.5},
        "attenuation_radius": 1000.0
      }
    ]
  },
  "actors": [
    {
      "type": "string - one of: cube, sphere, cylinder, cone, plane, stairs, wall, pillar, arch, platform, ramp, trigger_zone, spawn_point, checkpoint, collectible, enemy_spawn, npc_location, decoration",
      "name": "string - descriptive name",
      "position": {"x": 0.0, "y": 0.0, "z": 0.0},
      "rotation": {"pitch": 0.0, "yaw": 0.0, "roll": 0.0},
      "scale": {"x": 1.0, "y": 1.0, "z": 1.0},
      "material": {
        "color": {"r": 0.8, "g": 0.8, "b": 0.8},
        "metallic": 0.0,
        "roughness": 0.5,
        "emissive_strength": 0.0
      },
      "physics_enabled": false,
      "custom_mesh": "optional string - asset path to a project mesh (e.g. /Game/Props/SM_Barrel.SM_Barrel). Only use this if the user has provided a list of available project assets. Leave empty or omit to use the default shape for the type.",
      "tags": ["string"]
    }
  ],
  "gameplay": {
    "player_start": {"x": 0, "y": 0, "z": 200},
    "game_mode": "string - one of: exploration, platformer, shooter, puzzle, racing, survival, sandbox",
    "objectives": [
      {
        "type": "string - one of: reach_location, collect_items, defeat_enemies, survive_time, solve_puzzle",
        "description": "string",
        "target_count": 1
      }
    ]
  },
  "post_processing": {
    "bloom_intensity": 0.675,
    "auto_exposure_min": 0.5,
    "auto_exposure_max": 2.0,
    "vignette_intensity": 0.4,
    "color_saturation": 1.0,
    "color_contrast": 1.0,
    "ambient_occlusion_intensity": 0.5
  },
  "audio": {
    "ambient_sound": "string - one of: wind, rain, forest, ocean, city, cave, space_hum, lava, underwater, none",
    "music_mood": "string - one of: epic, calm, tense, mysterious, cheerful, dark, heroic, none"
  }
}

Generate a richly detailed level with at least 15-30 actors creating an interesting and playable environment. Be creative with placement, materials, and the overall atmosphere. Make the level feel complete and immersive. Use varied scales and materials to create visual interest. Include gameplay elements like spawn points, collectibles, and objectives that fit the theme.

If the user has provided a list of available project assets, use the "custom_mesh" field to reference those assets by their exact asset path. Match assets thematically to the actor's role in the level. Only reference assets from the provided list - do not invent asset paths.)");
}
