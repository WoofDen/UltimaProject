#pragma once

#include "Brushes/SlateColorBrush.h"
#include "Styling/SlateTypes.h"
#include "UltimaProject/Items/Containers/ContainerTypes.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/SCompoundWidget.h"

class SVerticalBox;
class UContainerComponent;
class UEnum;

/**
 * Pure Slate widget: light gray panel with a "Total" row and one row per liquid.
 * Row format:  name | progress bar (% of container capacity) | [+/-] details button
 * The "details" block (comma separated meta flag names) is shown at the bottom of the panel.
 *
 * The widget polls the container every tick and rebuilds itself only when the data changes.
 */
class SLiquidsDisplay : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SLiquidsDisplay)
			: _Width(280.f)
		{
		}

		SLATE_ARGUMENT(float, Width)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	void SetContainer(UContainerComponent* InContainer);

	/** Fill color of the progress bar for the given substance. Light blue for water, black by default. */
	static FLinearColor GetSubstanceColor(ESubstance Type);

	/** Enum value name without the "ESubstance::" prefix, lower-cased ("water"). */
	static FText GetSubstanceName(ESubstance Type);

	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

private:
	struct FLiquidRow
	{
		uint32 Key = 0;
		ESubstance Type = ESubstance::Water;
		uint64 Flags = 0;
		TWeakObjectPtr<UEnum> Enum;
		float Fraction = 0.f;

		bool HasDetails() const { return Enum.IsValid() && Flags != 0; }

		bool operator==(const FLiquidRow& Other) const
		{
			return Key == Other.Key && Type == Other.Type && Flags == Other.Flags
				&& Enum == Other.Enum && Fraction == Other.Fraction;
		}
	};

	void Refresh(bool bForce);
	void RebuildRows();
	void RebuildDetails();

	TSharedRef<SWidget> MakeRow(const FText& Name,
	                            const FLinearColor& NameColor,
	                            const FLinearColor& BarColor,
	                            float Fraction,
	                            const FText& BarText,
	                            const FLiquidRow* DetailsRow);

	FReply OnDetailsClicked(uint32 Key);

	TWeakObjectPtr<UContainerComponent> Container;

	TArray<FLiquidRow> Rows;
	float Capacity = 0.f;
	float Used = 0.f;
	float UsedNormalized = 0.f;
	bool bHasData = false;

	// Keys of the rows whose details are currently expanded
	TSet<uint32> ExpandedKeys;

	TSharedPtr<SVerticalBox> RowsBox;
	TSharedPtr<SVerticalBox> DetailsBox;

	// Styles must outlive the widgets that reference them
	FSlateColorBrush BackgroundBrush = FSlateColorBrush(FLinearColor(0.82f, 0.82f, 0.82f));
	FProgressBarStyle BarStyle;
	FButtonStyle FlatButtonStyle;
};
