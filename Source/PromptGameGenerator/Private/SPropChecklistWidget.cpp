// Copyright 2024 PromptGameGenerator Contributors. All Rights Reserved.

#include "SPropChecklistWidget.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SSearchBox.h"
#include "Widgets/Views/STableRow.h"
#include "DesktopPlatformModule.h"
#include "EditorStyleSet.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"

#define LOCTEXT_NAMESPACE "PromptGameGenerator"

void SPropChecklistWidget::Construct(const FArguments& InArgs)
{
	PropScanner = InArgs._PropScanner;
	OnConfirmed = InArgs._OnConfirmed;
	OnCancelled = InArgs._OnCancelled;

	if (PropScanner && PropScanner->GetTotalCount() == 0)
	{
		PropScanner->ScanProjectAssets();
	}

	RefreshFilteredList();

	ChildSlot
	[
		SNew(SVerticalBox)

		// Header
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(8)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(STextBlock)
				.Text(LOCTEXT("ChecklistTitle", "Select Props & Assets"))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 14))
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 4, 0, 0)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("ChecklistSubtitle", "Choose which project assets to include in your generated game. The AI will use selected assets as props and decorations in the level."))
				.AutoWrapText(true)
				.Font(FCoreStyle::GetDefaultFontStyle("Regular", 9))
				.ColorAndOpacity(FSlateColor(FLinearColor(0.6f, 0.6f, 0.6f)))
			]
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(8, 0)
		[
			SNew(SSeparator)
		]

		// Toolbar
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(8, 4)
		[
			BuildToolbar()
		]

		// Category sections and asset list
		+ SVerticalBox::Slot()
		.FillHeight(1.0f)
		.Padding(8, 0)
		[
			BuildAssetList()
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(8, 4)
		[
			SNew(SSeparator)
		]

		// Footer with confirm/cancel
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(8, 4, 8, 8)
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			.VAlign(VAlign_Center)
			[
				SAssignNew(SelectionCountText, STextBlock)
				.Text(this, &SPropChecklistWidget::GetSelectionCountText)
				.Font(FCoreStyle::GetDefaultFontStyle("Regular", 10))
			]

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(4, 0)
			[
				SNew(SButton)
				.OnClicked(this, &SPropChecklistWidget::OnConfirmClicked)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("ConfirmProps", "Confirm & Generate"))
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 11))
				]
			]

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(4, 0, 0, 0)
			[
				SNew(SButton)
				.OnClicked(this, &SPropChecklistWidget::OnCancelClicked)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("SkipProps", "Skip (No Props)"))
				]
			]
		]
	];
}

TSharedRef<SWidget> SPropChecklistWidget::BuildToolbar()
{
	return SNew(SHorizontalBox)

		// Search box
		+ SHorizontalBox::Slot()
		.FillWidth(1.0f)
		.Padding(0, 0, 4, 0)
		[
			SNew(SSearchBox)
			.HintText(LOCTEXT("SearchProps", "Search assets..."))
			.OnTextChanged(this, &SPropChecklistWidget::OnSearchTextChanged)
		]

		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(2, 0)
		[
			SNew(SButton)
			.OnClicked(this, &SPropChecklistWidget::OnSelectAllClicked)
			.ToolTipText(LOCTEXT("SelectAllTip", "Select all visible assets"))
			[
				SNew(STextBlock)
				.Text(LOCTEXT("SelectAll", "All"))
				.Font(FCoreStyle::GetDefaultFontStyle("Regular", 9))
			]
		]

		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(2, 0)
		[
			SNew(SButton)
			.OnClicked(this, &SPropChecklistWidget::OnDeselectAllClicked)
			.ToolTipText(LOCTEXT("DeselectAllTip", "Deselect all assets"))
			[
				SNew(STextBlock)
				.Text(LOCTEXT("DeselectAll", "None"))
				.Font(FCoreStyle::GetDefaultFontStyle("Regular", 9))
			]
		]

		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(2, 0)
		[
			SNew(SButton)
			.OnClicked(this, &SPropChecklistWidget::OnScanClicked)
			.ToolTipText(LOCTEXT("RescanTip", "Rescan project for assets"))
			[
				SNew(STextBlock)
				.Text(LOCTEXT("Rescan", "Rescan"))
				.Font(FCoreStyle::GetDefaultFontStyle("Regular", 9))
			]
		]

		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(2, 0, 0, 0)
		[
			SNew(SButton)
			.OnClicked(this, &SPropChecklistWidget::OnScanDirectoryClicked)
			.ToolTipText(LOCTEXT("ScanDirTip", "Scan a specific content directory"))
			[
				SNew(STextBlock)
				.Text(LOCTEXT("ScanDir", "+ Folder"))
				.Font(FCoreStyle::GetDefaultFontStyle("Regular", 9))
			]
		];
}

TSharedRef<SWidget> SPropChecklistWidget::BuildAssetList()
{
	return SNew(SScrollBox)

		// Category header checkboxes
		+ SScrollBox::Slot()
		.Padding(0, 4)
		[
			SAssignNew(CategoryContainer, SVerticalBox)
		]

		// Full asset list
		+ SScrollBox::Slot()
		[
			SAssignNew(PropListView, SListView<TSharedPtr<int32>>)
			.ListItemsSource(&FilteredIndices)
			.OnGenerateRow(this, &SPropChecklistWidget::OnGenerateRow)
			.SelectionMode(ESelectionMode::None)
		];
}

TSharedRef<ITableRow> SPropChecklistWidget::OnGenerateRow(TSharedPtr<int32> InIndex, const TSharedRef<STableViewBase>& OwnerTable)
{
	int32 Idx = *InIndex;
	TArray<FPropItem>& Props = PropScanner->GetScannedPropsRef();

	if (!Props.IsValidIndex(Idx))
	{
		return SNew(STableRow<TSharedPtr<int32>>, OwnerTable);
	}

	FPropItem& Prop = Props[Idx];

	return SNew(STableRow<TSharedPtr<int32>>, OwnerTable)
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(4, 2)
			.VAlign(VAlign_Center)
			[
				SNew(SCheckBox)
				.IsChecked_Lambda([this, Idx]()
				{
					TArray<FPropItem>& P = PropScanner->GetScannedPropsRef();
					return P.IsValidIndex(Idx) && P[Idx].bSelected ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
				})
				.OnCheckStateChanged_Lambda([this, Idx](ECheckBoxState NewState)
				{
					PropScanner->SetPropSelected(Idx, NewState == ECheckBoxState::Checked);
				})
			]

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(4, 2)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(FText::FromString(FString::Printf(TEXT("[%s]"), *Prop.GetCategoryName())))
				.Font(FCoreStyle::GetDefaultFontStyle("Regular", 8))
				.ColorAndOpacity(FSlateColor(FLinearColor(0.4f, 0.6f, 0.9f)))
			]

			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			.Padding(4, 2)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(FText::FromString(Prop.DisplayName))
				.Font(FCoreStyle::GetDefaultFontStyle("Regular", 10))
			]

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(4, 2)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(FText::FromString(Prop.PackageName))
				.Font(FCoreStyle::GetDefaultFontStyle("Regular", 8))
				.ColorAndOpacity(FSlateColor(FLinearColor(0.5f, 0.5f, 0.5f)))
			]
		];
}

void SPropChecklistWidget::OnSearchTextChanged(const FText& NewText)
{
	SearchFilter = NewText.ToString();
	RefreshFilteredList();
}

void SPropChecklistWidget::OnCategoryCheckChanged(ECheckBoxState NewState, EPropCategory Category)
{
	PropScanner->SelectCategory(Category, NewState == ECheckBoxState::Checked);
	RefreshFilteredList();
}

FReply SPropChecklistWidget::OnSelectAllClicked()
{
	if (!PropScanner) return FReply::Handled();

	if (SearchFilter.IsEmpty())
	{
		PropScanner->SelectAll();
	}
	else
	{
		for (const TSharedPtr<int32>& IdxPtr : FilteredIndices)
		{
			PropScanner->SetPropSelected(*IdxPtr, true);
		}
	}
	RefreshFilteredList();
	return FReply::Handled();
}

FReply SPropChecklistWidget::OnDeselectAllClicked()
{
	if (!PropScanner) return FReply::Handled();

	if (SearchFilter.IsEmpty())
	{
		PropScanner->DeselectAll();
	}
	else
	{
		for (const TSharedPtr<int32>& IdxPtr : FilteredIndices)
		{
			PropScanner->SetPropSelected(*IdxPtr, false);
		}
	}
	RefreshFilteredList();
	return FReply::Handled();
}

FReply SPropChecklistWidget::OnConfirmClicked()
{
	OnConfirmed.ExecuteIfBound();
	return FReply::Handled();
}

FReply SPropChecklistWidget::OnCancelClicked()
{
	OnCancelled.ExecuteIfBound();
	return FReply::Handled();
}

FReply SPropChecklistWidget::OnScanClicked()
{
	if (PropScanner)
	{
		PropScanner->ScanProjectAssets();
		RefreshFilteredList();

		FNotificationInfo Info(FText::FromString(
			FString::Printf(TEXT("Found %d assets"), PropScanner->GetTotalCount())));
		Info.ExpireDuration = 3.0f;
		FSlateNotificationManager::Get().AddNotification(Info);
	}
	return FReply::Handled();
}

FReply SPropChecklistWidget::OnScanDirectoryClicked()
{
	if (!PropScanner) return FReply::Handled();

	// Scan common additional directories
	TArray<FString> ExtraPaths = {
		TEXT("/Game/Props"),
		TEXT("/Game/Assets"),
		TEXT("/Game/Meshes"),
		TEXT("/Game/Models"),
		TEXT("/Game/Environment"),
		TEXT("/Game/Characters"),
		TEXT("/Game/Materials"),
		TEXT("/Game/Blueprints")
	};

	int32 PrevCount = PropScanner->GetTotalCount();
	for (const FString& Path : ExtraPaths)
	{
		PropScanner->ScanDirectory(Path);
	}

	RefreshFilteredList();

	int32 NewAssets = PropScanner->GetTotalCount() - PrevCount;
	FNotificationInfo Info(FText::FromString(
		FString::Printf(TEXT("Found %d additional assets"), FMath::Max(0, NewAssets))));
	Info.ExpireDuration = 3.0f;
	FSlateNotificationManager::Get().AddNotification(Info);

	return FReply::Handled();
}

void SPropChecklistWidget::RefreshFilteredList()
{
	FilteredIndices.Empty();

	if (!PropScanner) return;

	const TArray<FPropItem>& Props = PropScanner->GetScannedProps();
	for (int32 i = 0; i < Props.Num(); i++)
	{
		if (!SearchFilter.IsEmpty())
		{
			if (!Props[i].DisplayName.Contains(SearchFilter, ESearchCase::IgnoreCase) &&
				!Props[i].AssetPath.Contains(SearchFilter, ESearchCase::IgnoreCase) &&
				!Props[i].GetCategoryName().Contains(SearchFilter, ESearchCase::IgnoreCase))
			{
				continue;
			}
		}
		FilteredIndices.Add(MakeShareable(new int32(i)));
	}

	// Rebuild category headers
	if (CategoryContainer.IsValid())
	{
		CategoryContainer->ClearChildren();

		TArray<EPropCategory> Categories = PropScanner->GetAvailableCategories();
		for (EPropCategory Cat : Categories)
		{
			CategoryContainer->AddSlot()
			.AutoHeight()
			.Padding(0, 2)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(4, 0)
				.VAlign(VAlign_Center)
				[
					SNew(SCheckBox)
					.IsChecked_Lambda([this, Cat]()
					{
						return IsCategoryFullySelected(Cat) ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
					})
					.OnCheckStateChanged(this, &SPropChecklistWidget::OnCategoryCheckChanged, Cat)
				]
				+ SHorizontalBox::Slot()
				.FillWidth(1.0f)
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text(this, &SPropChecklistWidget::GetCategoryCountText, Cat)
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 10))
				]
			];
		}
	}

	if (PropListView.IsValid())
	{
		PropListView->RequestListRefresh();
	}
}

FText SPropChecklistWidget::GetSelectionCountText() const
{
	if (!PropScanner) return FText::GetEmpty();
	return FText::FromString(FString::Printf(TEXT("%d / %d assets selected"),
		PropScanner->GetSelectedCount(), PropScanner->GetTotalCount()));
}

FText SPropChecklistWidget::GetCategoryCountText(EPropCategory Category) const
{
	if (!PropScanner) return FText::GetEmpty();
	TArray<FPropItem> CatItems = PropScanner->GetPropsByCategory(Category);
	int32 SelectedInCat = 0;
	for (const FPropItem& Item : CatItems)
	{
		if (Item.bSelected) SelectedInCat++;
	}

	FString CatName;
	if (CatItems.Num() > 0)
	{
		CatName = CatItems[0].GetCategoryName();
	}
	return FText::FromString(FString::Printf(TEXT("%s (%d/%d)"), *CatName, SelectedInCat, CatItems.Num()));
}

bool SPropChecklistWidget::IsCategoryFullySelected(EPropCategory Category) const
{
	if (!PropScanner) return false;
	TArray<FPropItem> CatItems = PropScanner->GetPropsByCategory(Category);
	if (CatItems.Num() == 0) return false;
	for (const FPropItem& Item : CatItems)
	{
		if (!Item.bSelected) return false;
	}
	return true;
}

#undef LOCTEXT_NAMESPACE
