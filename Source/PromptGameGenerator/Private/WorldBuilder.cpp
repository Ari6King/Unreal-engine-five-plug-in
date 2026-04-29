// Copyright 2024 PromptGameGenerator Contributors. All Rights Reserved.

#include "WorldBuilder.h"
#include "Engine/World.h"
#include "Engine/StaticMeshActor.h"
#include "GameFramework/PlayerStart.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/PostProcessComponent.h"
#include "Components/SphereComponent.h"
#include "Components/BoxComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/PointLight.h"
#include "Engine/SkyLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/PostProcessVolume.h"
#include "Engine/TriggerBox.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "ProceduralMeshComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/KismetMathLibrary.h"
#include "Editor.h"
#include "EditorLevelUtils.h"

const FName UWorldBuilder::GeneratedTag = FName("PromptGameGenerated");

UWorldBuilder::UWorldBuilder()
{
}

void UWorldBuilder::BuildLevel(UWorld* World, const FGameLevelSpec& Spec)
{
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("PromptGameGenerator: Invalid world reference."));
		OnComplete.ExecuteIfBound(false);
		return;
	}

	OnProgress.ExecuteIfBound(0.0f, TEXT("Clearing previous generated content..."));
	ClearExistingGenerated(World);

	OnProgress.ExecuteIfBound(0.1f, TEXT("Setting up environment..."));
	SetupEnvironment(World, Spec.Environment);

	OnProgress.ExecuteIfBound(0.2f, TEXT("Building terrain..."));
	BuildTerrain(World, Spec.Terrain);

	OnProgress.ExecuteIfBound(0.35f, TEXT("Setting up lighting..."));
	SetupLighting(World, Spec.Lighting);

	OnProgress.ExecuteIfBound(0.5f, TEXT("Spawning actors..."));
	SpawnActors(World, Spec.Actors);

	OnProgress.ExecuteIfBound(0.75f, TEXT("Setting up player start..."));
	SetupPlayerStart(World, Spec.Gameplay);

	OnProgress.ExecuteIfBound(0.85f, TEXT("Configuring post-processing..."));
	SetupPostProcessing(World, Spec.PostProcessing);

	OnProgress.ExecuteIfBound(0.92f, TEXT("Placing gameplay markers..."));
	PlaceGameplayMarkers(World, Spec.Gameplay);

	OnProgress.ExecuteIfBound(1.0f, FString::Printf(TEXT("Level '%s' generated with %d actors!"), *Spec.LevelName, Spec.Actors.Num()));

	UE_LOG(LogTemp, Log, TEXT("PromptGameGenerator: Level '%s' built successfully with %d actors."),
		*Spec.LevelName, Spec.Actors.Num());

	OnComplete.ExecuteIfBound(true);
}

void UWorldBuilder::ClearExistingGenerated(UWorld* World)
{
	TArray<AActor*> ToDestroy;
	for (TActorIterator<AActor> It(World); It; ++It)
	{
		AActor* Actor = *It;
		if (Actor && Actor->ActorHasTag(GeneratedTag))
		{
			ToDestroy.Add(Actor);
		}
	}

	for (AActor* Actor : ToDestroy)
	{
		Actor->Destroy();
	}

	GeneratedActors.Empty();
	UE_LOG(LogTemp, Log, TEXT("PromptGameGenerator: Cleared %d previously generated actors."), ToDestroy.Num());
}

void UWorldBuilder::SetupEnvironment(UWorld* World, const FEnvironmentSpec& Env)
{
	AExponentialHeightFog* Fog = World->SpawnActor<AExponentialHeightFog>();
	if (Fog)
	{
		Fog->Tags.Add(GeneratedTag);
		Fog->SetActorLabel(TEXT("PGG_Fog"));
		UExponentialHeightFogComponent* FogComp = Fog->GetComponent();
		if (FogComp)
		{
			FogComp->SetFogDensity(Env.FogDensity);
			FogComp->SetFogInscatteringColor(Env.SkyColor.ToLinearColor());

			if (Env.Weather == TEXT("foggy"))
			{
				FogComp->SetFogDensity(FMath::Max(Env.FogDensity, 0.1f));
			}
			else if (Env.Weather == TEXT("rainy") || Env.Weather == TEXT("stormy"))
			{
				FogComp->SetFogDensity(FMath::Max(Env.FogDensity, 0.05f));
				FogComp->SetFogInscatteringColor(FLinearColor(0.3f, 0.3f, 0.35f));
			}

			FogComp->MarkRenderStateDirty();
		}
		GeneratedActors.Add(Fog);
	}
}

void UWorldBuilder::BuildTerrain(UWorld* World, const FTerrainSpec& Terrain)
{
	if (!Terrain.bEnabled) return;

	AActor* TerrainActor = World->SpawnActor<AActor>();
	if (!TerrainActor) return;

	TerrainActor->Tags.Add(GeneratedTag);
	TerrainActor->SetActorLabel(TEXT("PGG_Terrain"));

	UProceduralMeshComponent* ProcMesh = NewObject<UProceduralMeshComponent>(TerrainActor);
	ProcMesh->RegisterComponent();
	TerrainActor->SetRootComponent(ProcMesh);

	const int32 GridSize = 128;
	const float CellSize = Terrain.SizeX / static_cast<float>(GridSize);

	TArray<FVector> Vertices;
	TArray<int32> Triangles;
	TArray<FVector> Normals;
	TArray<FVector2D> UVs;
	TArray<FLinearColor> VertexColors;

	Vertices.Reserve((GridSize + 1) * (GridSize + 1));
	UVs.Reserve((GridSize + 1) * (GridSize + 1));
	VertexColors.Reserve((GridSize + 1) * (GridSize + 1));

	for (int32 Y = 0; Y <= GridSize; Y++)
	{
		for (int32 X = 0; X <= GridSize; X++)
		{
			float WorldX = (X - GridSize / 2) * CellSize;
			float WorldY = (Y - GridSize / 2) * CellSize;
			float Height = GeneratePerlinNoise(
				static_cast<float>(X), static_cast<float>(Y),
				Terrain.NoiseOctaves, Terrain.NoiseScale) * Terrain.HeightScale;

			Vertices.Add(FVector(WorldX, WorldY, Height));
			UVs.Add(FVector2D(static_cast<float>(X) / GridSize, static_cast<float>(Y) / GridSize));
			VertexColors.Add(Terrain.Color.ToLinearColor());
		}
	}

	Triangles.Reserve(GridSize * GridSize * 6);
	for (int32 Y = 0; Y < GridSize; Y++)
	{
		for (int32 X = 0; X < GridSize; X++)
		{
			int32 TopLeft = Y * (GridSize + 1) + X;
			int32 TopRight = TopLeft + 1;
			int32 BottomLeft = (Y + 1) * (GridSize + 1) + X;
			int32 BottomRight = BottomLeft + 1;

			Triangles.Add(TopLeft);
			Triangles.Add(BottomLeft);
			Triangles.Add(TopRight);

			Triangles.Add(TopRight);
			Triangles.Add(BottomLeft);
			Triangles.Add(BottomRight);
		}
	}

	Normals.SetNum(Vertices.Num());
	for (int32 i = 0; i < Normals.Num(); i++)
	{
		Normals[i] = FVector::UpVector;
	}

	// Compute proper normals from triangles
	for (int32 i = 0; i < Triangles.Num(); i += 3)
	{
		FVector V0 = Vertices[Triangles[i]];
		FVector V1 = Vertices[Triangles[i + 1]];
		FVector V2 = Vertices[Triangles[i + 2]];
		FVector Normal = FVector::CrossProduct(V1 - V0, V2 - V0).GetSafeNormal();

		Normals[Triangles[i]] += Normal;
		Normals[Triangles[i + 1]] += Normal;
		Normals[Triangles[i + 2]] += Normal;
	}
	for (FVector& N : Normals)
	{
		N = N.GetSafeNormal();
	}

	ProcMesh->CreateMeshSection_LinearColor(0, Vertices, Triangles, Normals, UVs, VertexColors, TArray<FProcMeshTangent>(), true);

	FMaterialSpec TerrainMat;
	TerrainMat.Color = Terrain.Color;
	TerrainMat.Roughness = 0.85f;
	UMaterialInstanceDynamic* Material = CreateDynamicMaterial(TerrainMat);
	if (Material)
	{
		ProcMesh->SetMaterial(0, Material);
	}

	ProcMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	ProcMesh->SetCollisionResponseToAllChannels(ECR_Block);

	GeneratedActors.Add(TerrainActor);
	UE_LOG(LogTemp, Log, TEXT("PromptGameGenerator: Terrain generated with %d vertices."), Vertices.Num());
}

void UWorldBuilder::SetupLighting(UWorld* World, const FLightingSpec& Lighting)
{
	// Directional light (sun)
	ADirectionalLight* DirLight = World->SpawnActor<ADirectionalLight>();
	if (DirLight)
	{
		DirLight->Tags.Add(GeneratedTag);
		DirLight->SetActorLabel(TEXT("PGG_Sun"));
		DirLight->SetActorRotation(Lighting.DirectionalLight.Rotation.ToRotator());

		UDirectionalLightComponent* LightComp = DirLight->GetComponent();
		if (LightComp)
		{
			LightComp->SetIntensity(Lighting.DirectionalLight.Intensity);
			LightComp->SetLightColor(Lighting.DirectionalLight.Color.ToLinearColor());
			LightComp->SetAtmosphereSunLight(true);
			LightComp->SetDynamicShadowCascades(4);
			LightComp->MarkRenderStateDirty();
		}
		GeneratedActors.Add(DirLight);
	}

	// Sky light
	ASkyLight* SkyLightActor = World->SpawnActor<ASkyLight>();
	if (SkyLightActor)
	{
		SkyLightActor->Tags.Add(GeneratedTag);
		SkyLightActor->SetActorLabel(TEXT("PGG_SkyLight"));

		USkyLightComponent* SkyComp = SkyLightActor->GetComponent();
		if (SkyComp)
		{
			SkyComp->SetIntensity(Lighting.SkyLight.Intensity);
			SkyComp->SetLightColor(Lighting.SkyLight.Color.ToLinearColor());
			SkyComp->MarkRenderStateDirty();
			SkyComp->RecaptureSky();
		}
		GeneratedActors.Add(SkyLightActor);
	}

	// Point lights
	for (int32 i = 0; i < Lighting.PointLights.Num(); i++)
	{
		const FPointLightSpec& PLSpec = Lighting.PointLights[i];

		APointLight* PointLight = World->SpawnActor<APointLight>(
			APointLight::StaticClass(), &FTransform::Identity);
		if (PointLight)
		{
			PointLight->Tags.Add(GeneratedTag);
			PointLight->SetActorLabel(FString::Printf(TEXT("PGG_PointLight_%d"), i));
			PointLight->SetActorLocation(PLSpec.Position.ToVector());

			UPointLightComponent* PLComp = PointLight->GetComponent();
			if (PLComp)
			{
				PLComp->SetIntensity(PLSpec.Intensity);
				PLComp->SetLightColor(PLSpec.Color.ToLinearColor());
				PLComp->SetAttenuationRadius(PLSpec.AttenuationRadius);
				PLComp->SetCastShadows(true);
				PLComp->MarkRenderStateDirty();
			}
			GeneratedActors.Add(PointLight);
		}
	}
}

void UWorldBuilder::SpawnActors(UWorld* World, const TArray<FActorSpec>& Actors)
{
	for (int32 i = 0; i < Actors.Num(); i++)
	{
		const FActorSpec& Spec = Actors[i];
		AActor* SpawnedActor = nullptr;

		if (Spec.Type == TEXT("trigger_zone") || Spec.Type == TEXT("spawn_point") ||
			Spec.Type == TEXT("checkpoint") || Spec.Type == TEXT("collectible") ||
			Spec.Type == TEXT("enemy_spawn") || Spec.Type == TEXT("npc_location"))
		{
			SpawnedActor = SpawnGameplayActor(World, Spec);
		}
		else
		{
			SpawnedActor = SpawnPrimitiveActor(World, Spec);
		}

		if (SpawnedActor)
		{
			SpawnedActor->Tags.Add(GeneratedTag);
			SpawnedActor->SetActorLabel(FString::Printf(TEXT("PGG_%s"), *Spec.Name));
			ApplyTags(SpawnedActor, Spec.Tags);
			GeneratedActors.Add(SpawnedActor);
		}

		float Progress = 0.5f + (0.25f * static_cast<float>(i + 1) / Actors.Num());
		OnProgress.ExecuteIfBound(Progress,
			FString::Printf(TEXT("Spawned %d/%d: %s"), i + 1, Actors.Num(), *Spec.Name));
	}
}

AActor* UWorldBuilder::SpawnPrimitiveActor(UWorld* World, const FActorSpec& Spec)
{
	UStaticMesh* Mesh = nullptr;
	FString MeshPath;

	// Try custom mesh from prop checklist first
	if (!Spec.CustomMesh.IsEmpty())
	{
		Mesh = LoadObject<UStaticMesh>(nullptr, *Spec.CustomMesh);
		if (Mesh)
		{
			UE_LOG(LogTemp, Log, TEXT("PromptGameGenerator: Using custom mesh '%s' for actor '%s'."), *Spec.CustomMesh, *Spec.Name);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("PromptGameGenerator: Custom mesh '%s' not found, falling back to basic shape."), *Spec.CustomMesh);
		}
	}

	// Fallback to basic shapes if no custom mesh loaded
	if (!Mesh)
	{
		if (Spec.Type == TEXT("cube") || Spec.Type == TEXT("wall") || Spec.Type == TEXT("platform") || Spec.Type == TEXT("decoration"))
		{
			MeshPath = TEXT("/Engine/BasicShapes/Cube.Cube");
		}
		else if (Spec.Type == TEXT("sphere") || Spec.Type == TEXT("collectible"))
		{
			MeshPath = TEXT("/Engine/BasicShapes/Sphere.Sphere");
		}
		else if (Spec.Type == TEXT("cylinder") || Spec.Type == TEXT("pillar"))
		{
			MeshPath = TEXT("/Engine/BasicShapes/Cylinder.Cylinder");
		}
		else if (Spec.Type == TEXT("cone"))
		{
			MeshPath = TEXT("/Engine/BasicShapes/Cone.Cone");
		}
		else if (Spec.Type == TEXT("plane"))
		{
			MeshPath = TEXT("/Engine/BasicShapes/Plane.Plane");
		}
		else if (Spec.Type == TEXT("stairs") || Spec.Type == TEXT("ramp"))
		{
			MeshPath = TEXT("/Engine/BasicShapes/Cube.Cube");
		}
		else if (Spec.Type == TEXT("arch"))
		{
			MeshPath = TEXT("/Engine/BasicShapes/Cylinder.Cylinder");
		}
		else
		{
			MeshPath = TEXT("/Engine/BasicShapes/Cube.Cube");
		}

		Mesh = LoadObject<UStaticMesh>(nullptr, *MeshPath);
	}
	if (!Mesh)
	{
		UE_LOG(LogTemp, Warning, TEXT("PromptGameGenerator: Failed to load mesh: %s"), *MeshPath);
		return nullptr;
	}

	FTransform SpawnTransform;
	SpawnTransform.SetLocation(Spec.Position.ToVector());
	SpawnTransform.SetRotation(FQuat(Spec.Rotation.ToRotator()));
	SpawnTransform.SetScale3D(Spec.Scale.ToVector());

	AStaticMeshActor* MeshActor = World->SpawnActor<AStaticMeshActor>(
		AStaticMeshActor::StaticClass(), &SpawnTransform);

	if (MeshActor)
	{
		UStaticMeshComponent* MeshComp = MeshActor->GetStaticMeshComponent();
		if (MeshComp)
		{
			MeshComp->SetStaticMesh(Mesh);

			UMaterialInstanceDynamic* DynMat = CreateDynamicMaterial(Spec.Material);
			if (DynMat)
			{
				MeshComp->SetMaterial(0, DynMat);
			}

			ConfigurePhysics(MeshActor, Spec.bPhysicsEnabled);
		}
	}

	return MeshActor;
}

AActor* UWorldBuilder::SpawnGameplayActor(UWorld* World, const FActorSpec& Spec)
{
	FTransform SpawnTransform;
	SpawnTransform.SetLocation(Spec.Position.ToVector());
	SpawnTransform.SetRotation(FQuat(Spec.Rotation.ToRotator()));

	if (Spec.Type == TEXT("trigger_zone"))
	{
		ATriggerBox* Trigger = World->SpawnActor<ATriggerBox>(
			ATriggerBox::StaticClass(), &SpawnTransform);
		if (Trigger)
		{
			Trigger->SetActorScale3D(Spec.Scale.ToVector());
		}
		return Trigger;
	}

	if (Spec.Type == TEXT("spawn_point") || Spec.Type == TEXT("enemy_spawn") || Spec.Type == TEXT("npc_location"))
	{
		AActor* Marker = World->SpawnActor<AActor>(AActor::StaticClass(), &SpawnTransform);
		if (Marker)
		{
			UStaticMesh* SphereMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Sphere.Sphere"));
			if (SphereMesh)
			{
				UStaticMeshComponent* MeshComp = NewObject<UStaticMeshComponent>(Marker);
				MeshComp->SetStaticMesh(SphereMesh);
				MeshComp->SetWorldScale3D(FVector(0.25f));
				MeshComp->RegisterComponent();
				Marker->SetRootComponent(MeshComp);

				FMaterialSpec MarkerMat;
				if (Spec.Type == TEXT("enemy_spawn"))
				{
					MarkerMat.Color = {1.0f, 0.0f, 0.0f};
					MarkerMat.EmissiveStrength = 2.0f;
				}
				else if (Spec.Type == TEXT("npc_location"))
				{
					MarkerMat.Color = {0.0f, 0.0f, 1.0f};
					MarkerMat.EmissiveStrength = 2.0f;
				}
				else
				{
					MarkerMat.Color = {0.0f, 1.0f, 0.0f};
					MarkerMat.EmissiveStrength = 2.0f;
				}

				UMaterialInstanceDynamic* DynMat = CreateDynamicMaterial(MarkerMat);
				if (DynMat)
				{
					MeshComp->SetMaterial(0, DynMat);
				}
				MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			}

#if WITH_EDITOR
			UTextRenderComponent* TextComp = NewObject<UTextRenderComponent>(Marker);
			TextComp->SetText(FText::FromString(Spec.Name));
			TextComp->SetWorldSize(30.0f);
			TextComp->SetHorizontalAlignment(EHTA_Center);
			TextComp->SetRelativeLocation(FVector(0, 0, 80));
			TextComp->RegisterComponent();
			TextComp->AttachToComponent(Marker->GetRootComponent(),
				FAttachmentTransformRules::KeepRelativeTransform);
#endif
		}
		return Marker;
	}

	if (Spec.Type == TEXT("checkpoint") || Spec.Type == TEXT("collectible"))
	{
		AStaticMeshActor* Actor = World->SpawnActor<AStaticMeshActor>(
			AStaticMeshActor::StaticClass(), &SpawnTransform);
		if (Actor)
		{
			UStaticMeshComponent* MeshComp = Actor->GetStaticMeshComponent();
			FString MeshPath = (Spec.Type == TEXT("collectible"))
				? TEXT("/Engine/BasicShapes/Sphere.Sphere")
				: TEXT("/Engine/BasicShapes/Cylinder.Cylinder");

			UStaticMesh* MeshAsset = LoadObject<UStaticMesh>(nullptr, *MeshPath);
			if (MeshAsset && MeshComp)
			{
				MeshComp->SetStaticMesh(MeshAsset);
				Actor->SetActorScale3D(Spec.Scale.ToVector());

				FMaterialSpec GlowMat = Spec.Material;
				if (GlowMat.EmissiveStrength < 1.0f)
				{
					GlowMat.EmissiveStrength = 3.0f;
				}

				UMaterialInstanceDynamic* DynMat = CreateDynamicMaterial(GlowMat);
				if (DynMat)
				{
					MeshComp->SetMaterial(0, DynMat);
				}
				MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
			}
		}
		return Actor;
	}

	return nullptr;
}

UMaterialInstanceDynamic* UWorldBuilder::CreateDynamicMaterial(const FMaterialSpec& MatSpec)
{
	UMaterial* BaseMaterial = LoadObject<UMaterial>(nullptr,
		TEXT("/Engine/EngineMaterials/DefaultMaterial.DefaultMaterial"));

	if (!BaseMaterial) return nullptr;

	UMaterialInstanceDynamic* DynMat = UMaterialInstanceDynamic::Create(BaseMaterial, this);
	if (!DynMat) return nullptr;

	DynMat->SetVectorParameterValue(FName("BaseColor"), MatSpec.Color.ToLinearColor());
	DynMat->SetScalarParameterValue(FName("Metallic"), MatSpec.Metallic);
	DynMat->SetScalarParameterValue(FName("Roughness"), MatSpec.Roughness);

	if (MatSpec.EmissiveStrength > 0.0f)
	{
		FLinearColor EmissiveColor = MatSpec.Color.ToLinearColor() * MatSpec.EmissiveStrength;
		DynMat->SetVectorParameterValue(FName("EmissiveColor"), EmissiveColor);
	}

	return DynMat;
}

void UWorldBuilder::ApplyMaterialToActor(AActor* Actor, UMaterialInstanceDynamic* Material)
{
	if (!Actor || !Material) return;

	TArray<UStaticMeshComponent*> MeshComponents;
	Actor->GetComponents<UStaticMeshComponent>(MeshComponents);

	for (UStaticMeshComponent* Comp : MeshComponents)
	{
		for (int32 i = 0; i < Comp->GetNumMaterials(); i++)
		{
			Comp->SetMaterial(i, Material);
		}
	}
}

void UWorldBuilder::ConfigurePhysics(AActor* Actor, bool bEnablePhysics)
{
	if (!Actor) return;

	TArray<UPrimitiveComponent*> PrimitiveComponents;
	Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);

	for (UPrimitiveComponent* Comp : PrimitiveComponents)
	{
		Comp->SetSimulatePhysics(bEnablePhysics);
		if (bEnablePhysics)
		{
			Comp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			Comp->SetCollisionResponseToAllChannels(ECR_Block);
		}
	}
}

void UWorldBuilder::ApplyTags(AActor* Actor, const TArray<FString>& Tags)
{
	if (!Actor) return;

	for (const FString& Tag : Tags)
	{
		Actor->Tags.AddUnique(FName(*Tag));
	}
}

void UWorldBuilder::SetupPlayerStart(UWorld* World, const FGameplaySpec& Gameplay)
{
	// Remove existing player starts that we generated
	for (TActorIterator<APlayerStart> It(World); It; ++It)
	{
		if ((*It)->ActorHasTag(GeneratedTag))
		{
			(*It)->Destroy();
		}
	}

	FTransform StartTransform;
	StartTransform.SetLocation(Gameplay.PlayerStart.ToVector());

	APlayerStart* PlayerStart = World->SpawnActor<APlayerStart>(
		APlayerStart::StaticClass(), &StartTransform);
	if (PlayerStart)
	{
		PlayerStart->Tags.Add(GeneratedTag);
		PlayerStart->SetActorLabel(TEXT("PGG_PlayerStart"));
		GeneratedActors.Add(PlayerStart);
	}
}

void UWorldBuilder::SetupPostProcessing(UWorld* World, const FPostProcessingSpec& PP)
{
	FTransform PPTransform;
	PPTransform.SetLocation(FVector::ZeroVector);

	APostProcessVolume* PPVolume = World->SpawnActor<APostProcessVolume>(
		APostProcessVolume::StaticClass(), &PPTransform);

	if (PPVolume)
	{
		PPVolume->Tags.Add(GeneratedTag);
		PPVolume->SetActorLabel(TEXT("PGG_PostProcess"));
		PPVolume->bUnbound = true;

		FPostProcessSettings& Settings = PPVolume->Settings;
		Settings.bOverride_BloomIntensity = true;
		Settings.BloomIntensity = PP.BloomIntensity;

		Settings.bOverride_AutoExposureMinBrightness = true;
		Settings.AutoExposureMinBrightness = PP.AutoExposureMin;

		Settings.bOverride_AutoExposureMaxBrightness = true;
		Settings.AutoExposureMaxBrightness = PP.AutoExposureMax;

		Settings.bOverride_VignetteIntensity = true;
		Settings.VignetteIntensity = PP.VignetteIntensity;

		Settings.bOverride_AmbientOcclusionIntensity = true;
		Settings.AmbientOcclusionIntensity = PP.AmbientOcclusionIntensity;

		GeneratedActors.Add(PPVolume);
	}
}

void UWorldBuilder::PlaceGameplayMarkers(UWorld* World, const FGameplaySpec& Gameplay)
{
	for (int32 i = 0; i < Gameplay.Objectives.Num(); i++)
	{
		const FObjectiveSpec& Objective = Gameplay.Objectives[i];

		AActor* Marker = World->SpawnActor<AActor>();
		if (Marker)
		{
			Marker->Tags.Add(GeneratedTag);
			Marker->Tags.Add(FName(TEXT("Objective")));
			Marker->SetActorLabel(FString::Printf(TEXT("PGG_Objective_%d"), i));

#if WITH_EDITOR
			UTextRenderComponent* TextComp = NewObject<UTextRenderComponent>(Marker);
			TextComp->SetText(FText::FromString(
				FString::Printf(TEXT("[%s] %s"), *Objective.Type, *Objective.Description)));
			TextComp->SetWorldSize(20.0f);
			TextComp->SetHorizontalAlignment(EHTA_Center);
			TextComp->RegisterComponent();
			Marker->SetRootComponent(TextComp);
			Marker->SetActorLocation(Gameplay.PlayerStart.ToVector() + FVector(0, 0, 200 + i * 40));
#endif

			GeneratedActors.Add(Marker);
		}
	}
}

float UWorldBuilder::GeneratePerlinNoise(float X, float Y, int32 Octaves, float Scale) const
{
	float Total = 0.0f;
	float Frequency = Scale;
	float Amplitude = 1.0f;
	float MaxValue = 0.0f;

	for (int32 i = 0; i < Octaves; i++)
	{
		float SampleX = X * Frequency;
		float SampleY = Y * Frequency;

		// Simple hash-based noise approximation
		float N = FMath::Sin(SampleX * 127.1f + SampleY * 311.7f);
		N = FMath::Abs(FMath::Frac(N * 43758.5453f) * 2.0f - 1.0f);

		// Smooth interpolation
		float IntX = FMath::FloorToFloat(SampleX);
		float IntY = FMath::FloorToFloat(SampleY);
		float FracX = FMath::Frac(SampleX);
		float FracY = FMath::Frac(SampleY);

		float SmoothX = FracX * FracX * (3.0f - 2.0f * FracX);
		float SmoothY = FracY * FracY * (3.0f - 2.0f * FracY);

		float N00 = FMath::Frac(FMath::Sin(IntX * 127.1f + IntY * 311.7f) * 43758.5453f) * 2.0f - 1.0f;
		float N10 = FMath::Frac(FMath::Sin((IntX + 1) * 127.1f + IntY * 311.7f) * 43758.5453f) * 2.0f - 1.0f;
		float N01 = FMath::Frac(FMath::Sin(IntX * 127.1f + (IntY + 1) * 311.7f) * 43758.5453f) * 2.0f - 1.0f;
		float N11 = FMath::Frac(FMath::Sin((IntX + 1) * 127.1f + (IntY + 1) * 311.7f) * 43758.5453f) * 2.0f - 1.0f;

		float Lerp1 = FMath::Lerp(N00, N10, SmoothX);
		float Lerp2 = FMath::Lerp(N01, N11, SmoothX);
		float Value = FMath::Lerp(Lerp1, Lerp2, SmoothY);

		Total += Value * Amplitude;
		MaxValue += Amplitude;

		Amplitude *= 0.5f;
		Frequency *= 2.0f;
	}

	return Total / MaxValue;
}
