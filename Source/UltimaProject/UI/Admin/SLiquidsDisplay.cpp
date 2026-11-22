#include "SLiquidsDisplay.h"

#include "Brushes/SlateNoResource.h"
#include "Fonts/SlateFontInfo.h"
#include "Styling/CoreStyle.h"
#include "UltimaProject/Items/Containers/ContainerComponent.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Notifications/SProgressBar.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"

namespace
{
	constexpr float PanelPadding = 8.f;   // spacing between the border and the content
	constexpr float RowSpacing = 4.f;     // spacing between rows
	constexpr float NameColumnWidth = 64.f;
	constexpr float DetailsColumnWidth = 20.f;
	constexpr float BarHeight = 18.f;

	const FLinearColor BarTrackColor(0.62f, 0.62f, 0.62f);

	FText MakePercentText(float Fraction)
	{
		const int32 Percent = FMath::RoundToInt(Fraction * 100.f);
		if (Percent == 0 && Fraction > 0.f)
		{
			return INVTEXT("<1%");
		}
		return FText::Format(INVTEXT("{0}%"), FText::AsNumber(Percent));
	}

	/** Display names of all enum entries whose index is set in Flags, comma separated. */
	FText BuildFlagsText(const UEnum* Enum, uint64 Flags)
	{
		TArray<FString> Names;
		if (Enum)
		{
			for (int32 i = 0; i < Enum->NumEnums(); ++i)
			{
				if (Enum->HasMetaData(TEXT("Hidden"), i))
				{
					continue;
				}
				if (i == Enum->NumEnums() - 1 && Enum->GetNameStringByIndex(i).EndsWith(TEXT("_MAX")))
				{
					continue;
				}

				const int64 Value = Enum->GetValueByIndex(i);
				if (Value >= 0 && Value < 64 && ((Flags >> Value) & 1ull))
				{
					Names.Add(Enum->GetDisplayNameTextByIndex(i).ToString());
				}
			}
		}
		return FText::FromString(FString::Join(Names, TEXT(", ")));
	}
}

FLinearColor SLiquidsDisplay::GetSubstanceColor(ESubstance Type)
{
	switch (Type)
	{
	case ESubstance::Water:
		return FLinearColor(0.45f, 0.75f, 1.f); // light blue
	default:
		return FLinearColor::Black;
	}
}

FText SLiquidsDisplay::GetSubstanceName(ESubstance Type)
{
	// GetNameStringByValue already strips the "ESubstance::" prefix
	UEnum* Enum = StaticEnum<ESubstance>();
	FString Name = Enum->GetNameStringByValue(static_cast<int64>(Type));
	if (Name.IsEmpty())
	{
		Name = TEXT("unknown");
	}
	return FText::FromString(Name.ToLower());
}

void SLiquidsDisplay::Construct(const FArguments& InArgs)
{
	BarStyle = FProgressBarStyle()
	           .SetBackgroundImage(FSlateColorBrush(BarTrackColor))
	           .SetFillImage(FSlateColorBrush(FLinearColor::White)) // tinted by FillColorAndOpacity
	           .SetEnableFillAnimation(false);

	// "+"/"-" button: no background, no borders
	FlatButtonStyle = FButtonStyle()
	                  .SetNormal(FSlateNoResource())
	                  .SetHovered(FSlateNoResource())
	                  .SetPressed(FSlateNoResource())
	                  .SetNormalPadding(FMargin(0.f))
	                  .SetPressedPadding(FMargin(0.f));

	ChildSlot
	[
		SNew(SBox)
		.WidthOverride(InArgs._Width)
		[
			SNew(SBorder)
			.BorderImage(&BackgroundBrush)
			.Padding(PanelPadding)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight()
				[
					SAssignNew(RowsBox, SVerticalBox)
				]
				+ SVerticalBox::Slot().AutoHeight()
				[
					SAssignNew(DetailsBox, SVerticalBox)
				]
			]
		]
	];

	Refresh(true);
}

void SLiquidsDisplay::SetContainer(UContainerComponent* InContainer)
{
	Container = InContainer;
	ExpandedKeys.Reset();
	Refresh(true);
}

void SLiquidsDisplay::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);
	Refresh(false);
}

void SLiquidsDisplay::Refresh(bool bForce)
{
	TArray<FLiquidRow> NewRows;
	float NewCapacity = 0.f;
	float NewUsed = 0.f;
	float NewUsedNormalized = 0.f;

	if (UContainerComponent* C = Container.Get())
	{
		NewCapacity = C->GetLiquidCapacity();
		NewUsed = C->GetLiquidCapacityUsed(false);
		NewUsedNormalized = C->GetLiquidCapacityUsed(true);

		for (const FContainerLiquidData& Liquid : C->GetLiquids())
		{
			FLiquidRow Row;
			Row.Key = GetTypeHash(Liquid.Descriptor);
			Row.Type = Liquid.Descriptor.Type;
			Row.Flags = Liquid.Descriptor.MetaFlags;
			Row.Enum = Liquid.Descriptor.MetaFlagsEnum;
			
			float Fraction = Liquid.GetAmount() / NewCapacity;
			Row.Fraction = FMath::Clamp(Fraction, 0.f, 1.f);
			NewRows.Add(Row);
		}
	}

	if (!bForce && bHasData
		&& NewCapacity == Capacity && NewUsed == Used && NewRows == Rows)
	{
		return;
	}

	Rows = MoveTemp(NewRows);
	Capacity = NewCapacity;
	Used = NewUsed;
	UsedNormalized = NewUsedNormalized;
	bHasData = true;

	RebuildRows();
}

void SLiquidsDisplay::RebuildRows()
{
	RowsBox->ClearChildren();

	// Forget expanded state of rows that no longer have details
	TSet<uint32> ValidKeys;
	for (const FLiquidRow& Row : Rows)
	{
		if (Row.HasDetails())
		{
			ValidKeys.Add(Row.Key);
		}
	}
	ExpandedKeys = ExpandedKeys.Intersect(ValidKeys);

	// --- Total row (always first) ---
	{
		FNumberFormattingOptions CapacityFormat;
		CapacityFormat.MaximumFractionalDigits = 2;

		const FText TotalText = FText::Format(
			INVTEXT("{0} / {1}"),
			FText::AsNumber(Used, &CapacityFormat),
			FText::AsNumber(Capacity, &CapacityFormat));

		RowsBox->AddSlot().AutoHeight().Padding(0.f, 0.f, 0.f, RowSpacing)
		[
			MakeRow(INVTEXT("Total"), FLinearColor::Black, FLinearColor::Black,
			        FMath::Clamp(UsedNormalized, 0.f, 1.f), TotalText, nullptr)
		];
	}

	// --- Liquid rows ---
	for (const FLiquidRow& Row : Rows)
	{
		RowsBox->AddSlot().AutoHeight().Padding(0.f, 0.f, 0.f, RowSpacing)
		[
			MakeRow(GetSubstanceName(Row.Type), FLinearColor::Black, GetSubstanceColor(Row.Type),
			        Row.Fraction, MakePercentText(Row.Fraction), &Row)
		];
	}

	RebuildDetails();
}

void SLiquidsDisplay::RebuildDetails()
{
	DetailsBox->ClearChildren();

	for (const FLiquidRow& Row : Rows)
	{
		if (!Row.HasDetails() || !ExpandedKeys.Contains(Row.Key))
		{
			continue;
		}

		DetailsBox->AddSlot().AutoHeight().Padding(0.f, 2.f, 0.f, 0.f)
		[
			SNew(STextBlock)
			.AutoWrapText(true)
			.Font(FCoreStyle::GetDefaultFontStyle("Regular", 9))
			.ColorAndOpacity(FLinearColor::Black)
			.Text(FText::Format(INVTEXT("{0}: {1}"), GetSubstanceName(Row.Type),
			                    BuildFlagsText(Row.Enum.Get(), Row.Flags)))
		];
	}
}

TSharedRef<SWidget> SLiquidsDisplay::MakeRow(const FText& Name, const FLinearColor& NameColor,
                                             const FLinearColor& BarColor, float Fraction,
                                             const FText& BarText, const FLiquidRow* DetailsRow)
{
	// White text with a black outline stays readable on both the filled and the empty part of the bar
	FSlateFontInfo BarFont = FCoreStyle::GetDefaultFontStyle("Bold", 9);
	BarFont.OutlineSettings.OutlineSize = 1;
	BarFont.OutlineSettings.OutlineColor = FLinearColor::Black;

	TSharedRef<SWidget> DetailsWidget = SNullWidget::NullWidget;
	if (DetailsRow && DetailsRow->HasDetails())
	{
		const uint32 Key = DetailsRow->Key;
		DetailsWidget = SNew(SButton)
			.ButtonStyle(&FlatButtonStyle)
			.ContentPadding(FMargin(0.f))
			.ToolTipText(INVTEXT("Details"))
			.OnClicked(FOnClicked::CreateSP(this, &SLiquidsDisplay::OnDetailsClicked, Key))
			[
				SNew(STextBlock)
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
				.ColorAndOpacity(FLinearColor::Black)
				.Text_Lambda([this, Key]()
				{
					return ExpandedKeys.Contains(Key) ? INVTEXT("-") : INVTEXT("+");
				})
			];
	}

	return SNew(SHorizontalBox)
		// name
		+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
		[
			SNew(SBox).WidthOverride(NameColumnWidth)
			[
				SNew(STextBlock)
				.Font(FCoreStyle::GetDefaultFontStyle("Regular", 9))
				.ColorAndOpacity(NameColor)
				.Text(Name)
			]
		]
		// size
		+ SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center).Padding(4.f, 0.f)
		[
			SNew(SBox).HeightOverride(BarHeight)
			[
				SNew(SOverlay)
				+ SOverlay::Slot()
				[
					SNew(SProgressBar)
					.Style(&BarStyle)
					.BorderPadding(FVector2D::ZeroVector)
					.Percent(TOptional<float>(Fraction))
					.FillColorAndOpacity(FSlateColor(BarColor))
				]
				+ SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Font(BarFont)
					.ColorAndOpacity(FLinearColor::White)
					.Text(BarText)
				]
			]
		]
		// details (column is always reserved so the bars stay aligned)
		+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
		[
			SNew(SBox).WidthOverride(DetailsColumnWidth).HAlign(HAlign_Center)
			[
				DetailsWidget
			]
		];
}

FReply SLiquidsDisplay::OnDetailsClicked(uint32 Key)
{
	if (ExpandedKeys.Contains(Key))
	{
		ExpandedKeys.Remove(Key);
	}
	else
	{
		ExpandedKeys.Add(Key);
	}

	// Only the bottom block is rebuilt, so the clicked button stays alive
	RebuildDetails();
	return FReply::Handled();
}
