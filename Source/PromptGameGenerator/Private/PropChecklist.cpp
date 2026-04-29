// Copyright 2024 PromptGameGenerator Contributors. All Rights Reserved.

#include "PropChecklist.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/Texture.h"

UPropScanner::UPropScanner()
{
}

void UPropScanner::ScanProjectAssets(const FString& SearchPath)
{
	ScannedProps.Empty();

	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	TArray<FAssetData> AllAssets;
	AssetRegistry.GetAssetsByPath(FName(*SearchPath), AllAssets, true);

	for (const FAssetData& AssetData : AllAssets)
	{
		EPropCategory Category = ClassifyAsset(AssetData);
		if (Category == EPropCategory::Other)
		{
			continue;
		}

		FPropItem Item;
		Item.DisplayName = SanitizeAssetName(AssetData.AssetName.ToString());
		Item.AssetPath = AssetData.GetSoftObjectPath().ToString();
		Item.PackageName = AssetData.PackageName.ToString();
		Item.Category = Category;
		Item.bSelected = false;

		ScannedProps.Add(Item);
	}

	ScannedProps.Sort([](const FPropItem& A, const FPropItem& B)
	{
		if (A.Category != B.Category)
			return static_cast<uint8>(A.Category) < static_cast<uint8>(B.Category);
		return A.DisplayName < B.DisplayName;
	});

	UE_LOG(LogTemp, Log, TEXT("PromptGameGenerator: Scanned %d usable assets from '%s'."), ScannedProps.Num(), *SearchPath);
}

void UPropScanner::ScanDirectory(const FString& DirectoryPath)
{
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	TArray<FAssetData> Assets;
	AssetRegistry.GetAssetsByPath(FName(*DirectoryPath), Assets, true);

	for (const FAssetData& AssetData : Assets)
	{
		EPropCategory Category = ClassifyAsset(AssetData);
		if (Category == EPropCategory::Other)
		{
			continue;
		}

		bool bAlreadyExists = false;
		for (const FPropItem& Existing : ScannedProps)
		{
			if (Existing.AssetPath == AssetData.GetSoftObjectPath().ToString())
			{
				bAlreadyExists = true;
				break;
			}
		}
		if (bAlreadyExists) continue;

		FPropItem Item;
		Item.DisplayName = SanitizeAssetName(AssetData.AssetName.ToString());
		Item.AssetPath = AssetData.GetSoftObjectPath().ToString();
		Item.PackageName = AssetData.PackageName.ToString();
		Item.Category = Category;
		Item.bSelected = false;

		ScannedProps.Add(Item);
	}
}

EPropCategory UPropScanner::ClassifyAsset(const FAssetData& AssetData) const
{
	FName AssetClass = AssetData.AssetClassPath.GetAssetName();

	if (AssetClass == UStaticMesh::StaticClass()->GetFName())
		return EPropCategory::StaticMesh;
	if (AssetClass == USkeletalMesh::StaticClass()->GetFName())
		return EPropCategory::SkeletalMesh;
	if (AssetClass == UMaterial::StaticClass()->GetFName() ||
		AssetClass == FName("MaterialInstance") ||
		AssetClass == FName("MaterialInstanceConstant"))
		return EPropCategory::Material;
	if (AssetClass == UBlueprint::StaticClass()->GetFName())
		return EPropCategory::Blueprint;
	if (AssetClass == USoundBase::StaticClass()->GetFName() ||
		AssetClass == FName("SoundWave") ||
		AssetClass == FName("SoundCue"))
		return EPropCategory::Sound;
	if (AssetClass == UParticleSystem::StaticClass()->GetFName() ||
		AssetClass == FName("NiagaraSystem"))
		return EPropCategory::Particle;
	if (AssetClass == UTexture::StaticClass()->GetFName() ||
		AssetClass == FName("Texture2D"))
		return EPropCategory::Texture;

	return EPropCategory::Other;
}

TArray<FPropItem> UPropScanner::GetSelectedProps() const
{
	TArray<FPropItem> Selected;
	for (const FPropItem& Item : ScannedProps)
	{
		if (Item.bSelected)
		{
			Selected.Add(Item);
		}
	}
	return Selected;
}

TArray<FPropItem> UPropScanner::GetPropsByCategory(EPropCategory Category) const
{
	TArray<FPropItem> Result;
	for (const FPropItem& Item : ScannedProps)
	{
		if (Item.Category == Category)
		{
			Result.Add(Item);
		}
	}
	return Result;
}

TArray<EPropCategory> UPropScanner::GetAvailableCategories() const
{
	TArray<EPropCategory> Categories;
	for (const FPropItem& Item : ScannedProps)
	{
		Categories.AddUnique(Item.Category);
	}
	return Categories;
}

void UPropScanner::SelectAll()
{
	for (FPropItem& Item : ScannedProps)
	{
		Item.bSelected = true;
	}
}

void UPropScanner::DeselectAll()
{
	for (FPropItem& Item : ScannedProps)
	{
		Item.bSelected = false;
	}
}

void UPropScanner::SelectCategory(EPropCategory Category, bool bSelect)
{
	for (FPropItem& Item : ScannedProps)
	{
		if (Item.Category == Category)
		{
			Item.bSelected = bSelect;
		}
	}
}

void UPropScanner::SetPropSelected(int32 Index, bool bSelected)
{
	if (ScannedProps.IsValidIndex(Index))
	{
		ScannedProps[Index].bSelected = bSelected;
	}
}

int32 UPropScanner::GetSelectedCount() const
{
	int32 Count = 0;
	for (const FPropItem& Item : ScannedProps)
	{
		if (Item.bSelected) Count++;
	}
	return Count;
}

FString UPropScanner::BuildPropsContextForLLM() const
{
	TArray<FPropItem> Selected = GetSelectedProps();
	if (Selected.Num() == 0)
	{
		return TEXT("");
	}

	FString Context = TEXT("\n\nThe user has selected the following project assets to be used in the level. ");
	Context += TEXT("When generating actors, prefer using these assets by referencing their asset_path in a \"custom_mesh\" field. ");
	Context += TEXT("For each actor that should use a project asset, add: \"custom_mesh\": \"<asset_path>\" to the actor spec.\n\n");
	Context += TEXT("Available project assets:\n");

	EPropCategory LastCategory = EPropCategory::Other;
	for (const FPropItem& Item : Selected)
	{
		if (Item.Category != LastCategory)
		{
			Context += FString::Printf(TEXT("\n[%s]\n"), *Item.GetCategoryName());
			LastCategory = Item.Category;
		}
		Context += FString::Printf(TEXT("  - \"%s\" (path: %s)\n"), *Item.DisplayName, *Item.AssetPath);
	}

	Context += TEXT("\nUse these assets where thematically appropriate. You may still use basic shapes (cube, sphere, etc.) for simple structural elements, ");
	Context += TEXT("but prefer the user's project assets for detailed props and decorations.\n");

	return Context;
}

FString UPropScanner::SanitizeAssetName(const FString& Name) const
{
	FString Clean = Name;
	Clean.ReplaceInline(TEXT("SM_"), TEXT(""));
	Clean.ReplaceInline(TEXT("SK_"), TEXT(""));
	Clean.ReplaceInline(TEXT("M_"), TEXT(""));
	Clean.ReplaceInline(TEXT("MI_"), TEXT(""));
	Clean.ReplaceInline(TEXT("T_"), TEXT(""));
	Clean.ReplaceInline(TEXT("BP_"), TEXT(""));
	Clean.ReplaceInline(TEXT("S_"), TEXT(""));
	Clean.ReplaceInline(TEXT("_"), TEXT(" "));
	return Clean;
}
