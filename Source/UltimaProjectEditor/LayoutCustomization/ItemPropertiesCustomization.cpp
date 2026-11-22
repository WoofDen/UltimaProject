#if WITH_EDITOR

#include "ItemPropertiesCustomization.h"
#include "DetailLayoutBuilder.h"
#include "UltimaProject/Items/Common/ItemData.h"
#include "DetailWidgetRow.h"
#include "IDetailChildrenBuilder.h"
#include "IDetailCustomNodeBuilder.h"
#include "IPropertyUtilities.h"
#include "PropertyHandle.h"
#include "ScopedTransaction.h"
#include "Widgets/Input/SNumericEntryBox.h"
#include "Widgets/Input/SSlider.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "ItemPropertiesCustomization"

// ---------------------------------------------------------------------------
// Node builder: one slider + numeric entry per enabled item property
// ---------------------------------------------------------------------------
class FItemPropertiesNodeBuilder : public IDetailCustomNodeBuilder, public TSharedFromThis<FItemPropertiesNodeBuilder>
{
public:
	FItemPropertiesNodeBuilder(TSharedRef<IPropertyHandle> InStructHandle, TSharedPtr<IPropertyUtilities> InUtils)
		: StructHandle(InStructHandle)
		, Utils(InUtils)
	{
	}

	virtual void SetOnRebuildChildren(FSimpleDelegate InOnRebuild) override { OnRebuild = InOnRebuild; }
	virtual bool InitiallyCollapsed() const override { return false; }
	virtual FName GetName() const override { return TEXT("ItemIntProperties"); }
	virtual bool RequiresTick() const override { return true; }

	// Rebuild rows when the enabled set changes (EnabledProperties edited, data asset swapped, asset loaded...)
	virtual void Tick(float) override
	{
		TArray<EItemIntProperty> Now;
		const ESource NowSource = ResolveKeys(Now);
		if (NowSource != DisplayedSource || Now != DisplayedKeys)
		{
			DisplayedSource = NowSource;
			DisplayedKeys = Now;
			OnRebuild.ExecuteIfBound();
		}
	}

	virtual void GenerateHeaderRowContent(FDetailWidgetRow& NodeRow) override
	{
		NodeRow.NameContent()
		[
			SNew(STextBlock)
			.Text(LOCTEXT("IntProps", "Int Properties"))
			.Font(IDetailLayoutBuilder::GetDetailFont())
		];
	}

	virtual void GenerateChildContent(IDetailChildrenBuilder& ChildrenBuilder) override
	{
		DisplayedSource = ResolveKeys(DisplayedKeys);

		if (DisplayedSource == ESource::NoAsset || DisplayedKeys.IsEmpty())
		{
			const FText Hint = DisplayedSource == ESource::NoAsset
				? LOCTEXT("NoAsset", "Assign Static Data to edit properties")
				: LOCTEXT("NoProps", "No enabled properties");

			ChildrenBuilder.AddCustomRow(Hint)
			[
				SNew(STextBlock)
				.Text(Hint)
				.Font(IDetailLayoutBuilder::GetDetailFontItalic())
			];
			return;
		}

		const UEnum* Enum = StaticEnum<EItemIntProperty>();

		for (const EItemIntProperty Key : DisplayedKeys)
		{
			if (Key == EItemIntProperty::MAX)
			{
				continue;
			}

			const FText Label = Enum->GetDisplayNameTextByValue(static_cast<int64>(Key));

			ChildrenBuilder.AddCustomRow(Label)
			.NameContent()
			[
				SNew(STextBlock)
				.Text(Label)
				.Font(IDetailLayoutBuilder::GetDetailFont())
			]
			.ValueContent()
			.MinDesiredWidth(260.f)
			.MaxDesiredWidth(500.f)
			[
				SNew(SHorizontalBox)

				+ SHorizontalBox::Slot()
				.FillWidth(1.f)
				.VAlign(VAlign_Center)
				[
					SNew(SSlider)
					.MinValue(0.f)
					.MaxValue(1.f)
					.StepSize(1.f / UINT8_MAX)
					.Value(TAttribute<float>::CreateSP(this, &FItemPropertiesNodeBuilder::GetNormalized, Key))
					.OnMouseCaptureBegin(FSimpleDelegate::CreateSP(this, &FItemPropertiesNodeBuilder::OnSliderBegin))
					.OnMouseCaptureEnd(FSimpleDelegate::CreateSP(this, &FItemPropertiesNodeBuilder::OnSliderEnd))
					.OnValueChanged(FOnFloatValueChanged::CreateSP(this, &FItemPropertiesNodeBuilder::OnSliderChanged, Key))
					.IsEnabled(TAttribute<bool>::CreateSP(this, &FItemPropertiesNodeBuilder::IsEditable))
				]

				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(6.f, 0.f, 0.f, 0.f)
				.VAlign(VAlign_Center)
				[
					SNew(SBox)
					.WidthOverride(64.f)
					[
						SNew(SNumericEntryBox<float>)
						.AllowSpin(false)
						.MinValue(0.f)
						.MaxValue(1.f)
						.MaxFractionalDigits(3)
						.Font(IDetailLayoutBuilder::GetDetailFont())
						.Value(TAttribute<TOptional<float>>::CreateSP(this, &FItemPropertiesNodeBuilder::GetNormalizedOptional, Key))
						.OnValueCommitted(SNumericEntryBox<float>::FOnValueCommitted::CreateSP(this, &FItemPropertiesNodeBuilder::OnEntryCommitted, Key))
						.IsEnabled(TAttribute<bool>::CreateSP(this, &FItemPropertiesNodeBuilder::IsEditable))
					]
				]
			];
		}
	}

private:
	enum class ESource : uint8
	{
		NoContext, // no data asset can be determined -> show keys already stored
		NoAsset,   // context found, but the asset is not assigned
		Asset      // keys come from UItemDataAsset::EnabledProperties
	};

	static uint8 ToByte(float Normalized)
	{
		return static_cast<uint8>(FMath::Clamp(FMath::RoundToInt(Normalized * UINT8_MAX), 0, static_cast<int32>(UINT8_MAX)));
	}

	// ----- Context resolution ------------------------------------------------

	const UItemDataAsset* FindDataAsset(bool& bContextFound) const
	{
		bContextFound = false;

		// 1. FItemData / FItemDataDefinition above us: sibling StaticDataSoftPtr
		for (TSharedPtr<IPropertyHandle> Parent = StructHandle->GetParentHandle(); Parent.IsValid(); Parent = Parent->GetParentHandle())
		{
			const TSharedPtr<IPropertyHandle> Soft = Parent->GetChildHandle(TEXT("StaticDataSoftPtr"), /*bRecurse*/ false);
			if (!Soft.IsValid())
			{
				continue;
			}

			bContextFound = true;

			TArray<void*> Raw;
			Soft->AccessRawData(Raw);
			if (Raw.IsEmpty() || !Raw[0])
			{
				return nullptr;
			}

			const auto& SoftPtr = *static_cast<const TSoftObjectPtr<const UItemDataAsset>*>(Raw[0]);
			if (const UItemDataAsset* Loaded = SoftPtr.Get())
			{
				return Loaded;
			}
			return SoftPtr.IsNull() ? nullptr : SoftPtr.LoadSynchronous();
		}

		// 2. Struct owned directly by the data asset (DefaultProperties)
		TArray<UObject*> Outers;
		StructHandle->GetOuterObjects(Outers);
		if (!Outers.IsEmpty())
		{
			if (const UItemDataAsset* Asset = Cast<UItemDataAsset>(Outers[0]))
			{
				bContextFound = true;
				return Asset;
			}
		}

		return nullptr;
	}

	ESource ResolveKeys(TArray<EItemIntProperty>& OutKeys) const
	{
		OutKeys.Reset();

		bool bContext = false;
		const UItemDataAsset* Asset = FindDataAsset(bContext);

		if (!bContext)
		{
			if (const FItemProperties* Props = GetProps())
			{
				OutKeys = Props->PropertyKeys;
			}
			return ESource::NoContext;
		}

		if (!Asset)
		{
			return ESource::NoAsset;
		}

		for (const EItemIntProperty Key : Asset->EnabledProperties)
		{
			OutKeys.AddUnique(Key);
		}
		return ESource::Asset;
	}

	// ----- Data access -------------------------------------------------------

	const FItemProperties* GetProps() const
	{
		TArray<void*> Raw;
		StructHandle->AccessRawData(Raw);
		// With multi-selection and differing values, the first object wins
		return (!Raw.IsEmpty() && Raw[0]) ? static_cast<const FItemProperties*>(Raw[0]) : nullptr;
	}

	bool IsEditable() const
	{
		const bool bUtilsAllow = !Utils.IsValid() || Utils->IsPropertyEditingEnabled();
		return bUtilsAllow && !StructHandle->IsEditConst();
	}

	float GetNormalized(EItemIntProperty Key) const
	{
		const FItemProperties* Props = GetProps();
		if (!Props)
		{
			return 0.f;
		}

		const int32 Idx = Props->PropertyKeys.Find(Key);
		return Props->PropertyValues.IsValidIndex(Idx) ? static_cast<float>(Props->PropertyValues[Idx]) / UINT8_MAX : 0.f;
	}

	TOptional<float> GetNormalizedOptional(EItemIntProperty Key) const
	{
		return GetNormalized(Key);
	}

	void WriteNormalized(EItemIntProperty Key, float Normalized)
	{
		const uint8 Byte = ToByte(Normalized);

		TArray<void*> Raw;
		StructHandle->AccessRawData(Raw);
		for (void* Ptr : Raw)
		{
			if (!Ptr)
			{
				continue;
			}

			FItemProperties& Props = *static_cast<FItemProperties*>(Ptr);

			// Same contract as FItemData::SetProp: keys and values stay parallel
			int32 Idx = Props.PropertyKeys.Find(Key);
			if (Idx == INDEX_NONE)
			{
				Idx = Props.PropertyKeys.Add(Key);
			}
			if (Props.PropertyValues.Num() < Props.PropertyKeys.Num())
			{
				Props.PropertyValues.SetNumZeroed(Props.PropertyKeys.Num());
			}
			Props.PropertyValues[Idx] = Byte;
		}
	}

	// ----- Slider / entry handlers --------------------------------------------

	void OnSliderBegin()
	{
		bDragging = true;
		Transaction = MakeUnique<FScopedTransaction>(LOCTEXT("EditItemProp", "Edit Item Property"));
		StructHandle->NotifyPreChange();
	}

	void OnSliderChanged(float Value, EItemIntProperty Key)
	{
		if (!IsEditable())
		{
			return;
		}

		if (bDragging)
		{
			WriteNormalized(Key, Value);
			StructHandle->NotifyPostChange(EPropertyChangeType::Interactive);
		}
		else
		{
			// Keyboard / non-drag change: one self-contained transaction
			const FScopedTransaction Scoped(LOCTEXT("EditItemProp", "Edit Item Property"));
			StructHandle->NotifyPreChange();
			WriteNormalized(Key, Value);
			StructHandle->NotifyPostChange(EPropertyChangeType::ValueSet);
		}
	}

	void OnSliderEnd()
	{
		if (bDragging)
		{
			StructHandle->NotifyPostChange(EPropertyChangeType::ValueSet);
			Transaction.Reset();
			bDragging = false;
		}
	}

	void OnEntryCommitted(float Value, ETextCommit::Type, EItemIntProperty Key)
	{
		// Losing focus without a real change must not dirty the asset
		if (!IsEditable() || ToByte(Value) == ToByte(GetNormalized(Key)))
		{
			return;
		}

		OnSliderChanged(Value, Key);
	}

	TSharedRef<IPropertyHandle> StructHandle;
	TSharedPtr<IPropertyUtilities> Utils;
	FSimpleDelegate OnRebuild;

	TArray<EItemIntProperty> DisplayedKeys;
	ESource DisplayedSource = ESource::NoContext;

	bool bDragging = false;
	TUniquePtr<FScopedTransaction> Transaction;
};

// ---------------------------------------------------------------------------
// Struct customization
// ---------------------------------------------------------------------------
void FItemPropertiesCustomization::CustomizeHeader(TSharedRef<IPropertyHandle> StructHandle,
	FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& Utils)
{
	HeaderRow.NameContent()[ StructHandle->CreatePropertyNameWidget() ];
}

void FItemPropertiesCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> StructHandle,
	IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& Utils)
{
	// PropertyKeys / PropertyValues have no edit flags, so they never show up here
	ChildBuilder.AddCustomBuilder(MakeShared<FItemPropertiesNodeBuilder>(StructHandle, Utils.GetPropertyUtilities()));

	uint32 NumChildren = 0;
	StructHandle->GetNumChildren(NumChildren);
	for (uint32 i = 0; i < NumChildren; ++i)
	{
		if (TSharedPtr<IPropertyHandle> Child = StructHandle->GetChildHandle(i))
		{
			ChildBuilder.AddProperty(Child.ToSharedRef()); // SolidSubstance etc.
		}
	}
}

#undef LOCTEXT_NAMESPACE

#endif