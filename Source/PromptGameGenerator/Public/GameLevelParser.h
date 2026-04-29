// Copyright 2024 PromptGameGenerator Contributors. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameGenerationTypes.h"

class PROMPTGAMEGENERATOR_API FGameLevelParser
{
public:
	static bool ParseLevelSpec(const FString& JsonString, FGameLevelSpec& OutSpec);

private:
	static FColorSpec ParseColor(const TSharedPtr<FJsonObject>& Obj);
	static FPositionSpec ParsePosition(const TSharedPtr<FJsonObject>& Obj);
	static FRotationSpec ParseRotation(const TSharedPtr<FJsonObject>& Obj);
	static FMaterialSpec ParseMaterial(const TSharedPtr<FJsonObject>& Obj);
	static FActorSpec ParseActor(const TSharedPtr<FJsonObject>& Obj);
	static FEnvironmentSpec ParseEnvironment(const TSharedPtr<FJsonObject>& Obj);
	static FTerrainSpec ParseTerrain(const TSharedPtr<FJsonObject>& Obj);
	static FLightingSpec ParseLighting(const TSharedPtr<FJsonObject>& Obj);
	static FGameplaySpec ParseGameplay(const TSharedPtr<FJsonObject>& Obj);
	static FPostProcessingSpec ParsePostProcessing(const TSharedPtr<FJsonObject>& Obj);
	static FAudioSpec ParseAudio(const TSharedPtr<FJsonObject>& Obj);
};
