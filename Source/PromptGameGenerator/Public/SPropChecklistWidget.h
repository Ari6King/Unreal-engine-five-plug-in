// Copyright 2024 PromptGameGenerator Contributors. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/Input/SSearchBox.h"
#include "PropChecklist.h"

DECLARE_DELEGATE(FOnPropSelectionConfirmed);
DECLARE_DELEGATE(FOnPropSelectionCancelled);

class PROMPTGAMEGENERATOR_API SPropChecklistWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SPropChecklistWidget) {}
		SLATE_ARGUMENT(UPropScanner*, PropScanner)
		SLATE_EVENT(FOnPropSelectionConfirmed, OnConfirmed)
		SLATE_EVENT(FOnPropSelectionCancelled, OnCancelled)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

private:
	TSharedRef<SWidget> BuildCategorySection(EPropCategory Category);
	TSharedRef<SWidget> BuildToolbar();
	TSharedRef<SWidget> BuildAssetList();

	TSharedRef<ITableRow> OnGenerateRow(TSharedPtr<int32> InIndex, const TSharedRef<STableViewBase>& OwnerTable);

	void OnSearchTextChanged(const FText& NewText);
	void OnCategoryCheckChanged(ECheckBoxState NewState, EPropCategory Category);
	FReply OnSelectAllClicked();
	FReply OnDeselectAllClicked();
	FReply OnConfirmClicked();
	FReply OnCancelClicked();
	FReply OnScanClicked();
	FReply OnScanDirectoryClicked();

	void RefreshFilteredList();
	FText GetSelectionCountText() const;
	FText GetCategoryCountText(EPropCategory Category) const;
	bool IsCategoryFullySelected(EPropCategory Category) const;

	UPropScanner* PropScanner = nullptr;
	FOnPropSelectionConfirmed OnConfirmed;
	FOnPropSelectionCancelled OnCancelled;

	FString SearchFilter;
	TArray<TSharedPtr<int32>> FilteredIndices;
	TSharedPtr<SListView<TSharedPtr<int32>>> PropListView;
	TSharedPtr<STextBlock> SelectionCountText;
	TSharedPtr<SVerticalBox> CategoryContainer;
};
