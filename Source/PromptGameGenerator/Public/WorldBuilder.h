// Copyright 2024 PromptGameGenerator Contributors. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameGenerationTypes.h"
#include "WorldBuilder.generated.h"

DECLARE_DELEGATE_TwoParams(FOnWorldBuildProgress, float /*Progress 0-1*/, const FString& /*StatusMessage*/);
DECLARE_DELEGATE_OneParam(FOnWorldBuildComplete, bool /*bSuccess*/);

UCLASS()
class PROMPTGAMEGENERATOR_API UWorldBuilder : public UObject
{
	GENERATED_BODY()

public:
	UWorldBuilder();

	void BuildLevel(UWorld* World, const FGameLevelSpec& Spec);

	FOnWorldBuildProgress OnProgress;
	FOnWorldBuildComplete OnComplete;

private:
	void ClearExistingGenerated(UWorld* World);
	void SetupEnvironment(UWorld* World, const FEnvironmentSpec& Env);
	void BuildTerrain(UWorld* World, const FTerrainSpec& Terrain);
	void SetupLighting(UWorld* World, const FLightingSpec& Lighting);
	void SpawnActors(UWorld* World, const TArray<FActorSpec>& Actors);
	void SetupPlayerStart(UWorld* World, const FGameplaySpec& Gameplay);
	void SetupPostProcessing(UWorld* World, const FPostProcessingSpec& PP);
	void PlaceGameplayMarkers(UWorld* World, const FGameplaySpec& Gameplay);

	AActor* SpawnPrimitiveActor(UWorld* World, const FActorSpec& Spec);
	AActor* SpawnGameplayActor(UWorld* World, const FActorSpec& Spec);
	UMaterialInstanceDynamic* CreateDynamicMaterial(const FMaterialSpec& MatSpec);
	void ApplyMaterialToActor(AActor* Actor, UMaterialInstanceDynamic* Material);
	void ConfigurePhysics(AActor* Actor, bool bEnablePhysics);
	void ApplyTags(AActor* Actor, const TArray<FString>& Tags);

	float GeneratePerlinNoise(float X, float Y, int32 Octaves, float Scale) const;

	UPROPERTY()
	TArray<AActor*> GeneratedActors;

	static const FName GeneratedTag;
};
