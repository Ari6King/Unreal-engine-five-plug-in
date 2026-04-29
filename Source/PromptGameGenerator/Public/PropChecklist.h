// Copyright 2024 PromptGameGenerator Contributors. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/StaticMesh.h"
#include "Materials/Material.h"
#include "Engine/Blueprint.h"
#include "Sound/SoundBase.h"
#include "Particles/ParticleSystem.h"
#include "PropChecklist.generated.h"

UENUM(BlueprintType)
enum class EPropCategory : uint8
{
	StaticMesh		UMETA(DisplayName = "Static Meshes"),
	SkeletalMesh	UMETA(DisplayName = "Skeletal Meshes"),
	Material		UMETA(DisplayName = "Materials"),
	Blueprint		UMETA(DisplayName = "Blueprints"),
	Sound			UMETA(DisplayName = "Sounds"),
	Particle		UMETA(DisplayName = "Particles/Niagara"),
	Texture			UMETA(DisplayName = "Textures"),
	Other			UMETA(DisplayName = "Other")
};

USTRUCT(BlueprintType)
struct FPropItem
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString AssetPath;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EPropCategory Category = EPropCategory::Other;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bSelected = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PackageName;

	FString GetCategoryName() const
	{
		switch (Category)
		{
		case EPropCategory::StaticMesh: return TEXT("Static Meshes");
		case EPropCategory::SkeletalMesh: return TEXT("Skeletal Meshes");
		case EPropCategory::Material: return TEXT("Materials");
		case EPropCategory::Blueprint: return TEXT("Blueprints");
		case EPropCategory::Sound: return TEXT("Sounds");
		case EPropCategory::Particle: return TEXT("Particles");
		case EPropCategory::Texture: return TEXT("Textures");
		default: return TEXT("Other");
		}
	}
};

UCLASS()
class PROMPTGAMEGENERATOR_API UPropScanner : public UObject
{
	GENERATED_BODY()

public:
	UPropScanner();

	void ScanProjectAssets(const FString& SearchPath = TEXT("/Game"));
	void ScanDirectory(const FString& DirectoryPath);

	const TArray<FPropItem>& GetScannedProps() const { return ScannedProps; }
	TArray<FPropItem>& GetScannedPropsRef() { return ScannedProps; }

	TArray<FPropItem> GetSelectedProps() const;
	TArray<FPropItem> GetPropsByCategory(EPropCategory Category) const;
	TArray<EPropCategory> GetAvailableCategories() const;

	void SelectAll();
	void DeselectAll();
	void SelectCategory(EPropCategory Category, bool bSelect);
	void SetPropSelected(int32 Index, bool bSelected);

	FString BuildPropsContextForLLM() const;

	int32 GetTotalCount() const { return ScannedProps.Num(); }
	int32 GetSelectedCount() const;

private:
	EPropCategory ClassifyAsset(const FAssetData& AssetData) const;
	FString SanitizeAssetName(const FString& Name) const;

	UPROPERTY()
	TArray<FPropItem> ScannedProps;
};
