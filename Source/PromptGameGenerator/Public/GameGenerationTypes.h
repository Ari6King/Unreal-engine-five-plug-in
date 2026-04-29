// Copyright 2024 PromptGameGenerator Contributors. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameGenerationTypes.generated.h"

USTRUCT(BlueprintType)
struct FColorSpec
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite) float R = 1.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float G = 1.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float B = 1.0f;

	FLinearColor ToLinearColor() const { return FLinearColor(R, G, B, 1.0f); }
	FColor ToFColor() const { return ToLinearColor().ToFColor(true); }
};

USTRUCT(BlueprintType)
struct FRotationSpec
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite) float Pitch = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float Yaw = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float Roll = 0.0f;

	FRotator ToRotator() const { return FRotator(Pitch, Yaw, Roll); }
};

USTRUCT(BlueprintType)
struct FPositionSpec
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite) float X = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float Y = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float Z = 0.0f;

	FVector ToVector() const { return FVector(X, Y, Z); }
};

USTRUCT(BlueprintType)
struct FMaterialSpec
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite) FColorSpec Color;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float Metallic = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float Roughness = 0.5f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float EmissiveStrength = 0.0f;
};

USTRUCT(BlueprintType)
struct FActorSpec
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite) FString Type;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FString Name;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FPositionSpec Position;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FRotationSpec Rotation;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FPositionSpec Scale;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FMaterialSpec Material;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bPhysicsEnabled = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<FString> Tags;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FString CustomMesh;
};

USTRUCT(BlueprintType)
struct FDirectionalLightSpec
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite) float Intensity = 10.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FColorSpec Color;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FRotationSpec Rotation;
};

USTRUCT(BlueprintType)
struct FSkyLightSpec
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite) float Intensity = 1.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FColorSpec Color;
};

USTRUCT(BlueprintType)
struct FPointLightSpec
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite) FPositionSpec Position;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float Intensity = 5000.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FColorSpec Color;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float AttenuationRadius = 1000.0f;
};

USTRUCT(BlueprintType)
struct FLightingSpec
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite) FDirectionalLightSpec DirectionalLight;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FSkyLightSpec SkyLight;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<FPointLightSpec> PointLights;
};

USTRUCT(BlueprintType)
struct FEnvironmentSpec
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite) FString Type = TEXT("outdoor");
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FString TimeOfDay = TEXT("noon");
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FString Weather = TEXT("clear");
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FColorSpec SkyColor;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float AmbientIntensity = 1.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float FogDensity = 0.02f;
};

USTRUCT(BlueprintType)
struct FTerrainSpec
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bEnabled = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float SizeX = 8192.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float SizeY = 8192.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float HeightScale = 256.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float NoiseScale = 0.005f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 NoiseOctaves = 6;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FString MaterialType = TEXT("grass");
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FColorSpec Color;
};

USTRUCT(BlueprintType)
struct FObjectiveSpec
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite) FString Type;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FString Description;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 TargetCount = 1;
};

USTRUCT(BlueprintType)
struct FGameplaySpec
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite) FPositionSpec PlayerStart;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FString GameMode = TEXT("exploration");
	UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<FObjectiveSpec> Objectives;
};

USTRUCT(BlueprintType)
struct FPostProcessingSpec
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite) float BloomIntensity = 0.675f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float AutoExposureMin = 0.5f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float AutoExposureMax = 2.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float VignetteIntensity = 0.4f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float ColorSaturation = 1.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float ColorContrast = 1.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float AmbientOcclusionIntensity = 0.5f;
};

USTRUCT(BlueprintType)
struct FAudioSpec
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite) FString AmbientSound = TEXT("none");
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FString MusicMood = TEXT("none");
};

USTRUCT(BlueprintType)
struct FGameLevelSpec
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite) FString LevelName;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FString Description;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FEnvironmentSpec Environment;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FTerrainSpec Terrain;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FLightingSpec Lighting;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<FActorSpec> Actors;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FGameplaySpec Gameplay;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FPostProcessingSpec PostProcessing;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FAudioSpec Audio;
};
