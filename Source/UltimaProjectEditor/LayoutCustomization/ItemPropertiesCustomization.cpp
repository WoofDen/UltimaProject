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
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SNumericEntryBox.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "ItemPropertiesCustomization"

// ---------------------------------------------------------------------------
// Node builder: one numeric entry per enabled item property
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
	virtual FName GetName() const override { return TEXT("ItemNumericProperties"); }
	virtual bool RequiresTick() const override { return true; }

	// Rebuild rows when the enabled set changes or the data asset changes.
	virtual void Tick(float) override
	{
		TArray<EItemNumericProperty> Now;
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
			.Text(LOCTEXT("NumericProps", "Numeric Properties"))
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

		const UEnum* Enum = StaticEnum<EItemNumericProperty>();

		for (const EItemNumericProperty Key : DisplayedKeys)
		{
			if (Key == EItemNumericProperty::MAX)
			{
				continue;
			}

			const FText Label = Enum->GetDisplayNameTextByValue(static_cast<int64>(Key));

			FDetailWidgetRow& Row = ChildrenBuilder.AddCustomRow(Label);
			if (DisplayedSource == ESource::AssetDefaults)
			{
				Row.NameContent()
				[
					SNew(SCheckBox)
					.IsChecked(this, &FItemPropertiesNodeBuilder::GetKeyCheckState, Key)
					.OnCheckStateChanged(this, &FItemPropertiesNodeBuilder::OnKeyCheckStateChanged, Key)
					.IsEnabled(this, &FItemPropertiesNodeBuilder::IsEditable)
					[
						SNew(STextBlock).Text(Label).Font(IDetailLayoutBuilder::GetDetailFont())
					]
				];
			}
			else
			{
				Row.NameContent()
				[
					SNew(STextBlock).Text(Label).Font(IDetailLayoutBuilder::GetDetailFont())
				];
			}

			Row.ValueContent()
			.MinDesiredWidth(260.f)
			.MaxDesiredWidth(500.f)
			[
				SNew(SNumericEntryBox<float>)
				.AllowSpin(false)
				.MaxFractionalDigits(3)
				.Font(IDetailLayoutBuilder::GetDetailFont())
				.Value(TAttribute<TOptional<float>>::CreateSP(this, &FItemPropertiesNodeBuilder::GetNumericValueOptional, Key))
				.OnValueCommitted(SNumericEntryBox<float>::FOnValueCommitted::CreateSP(this, &FItemPropertiesNodeBuilder::OnEntryCommitted, Key))
				.IsEnabled(TAttribute<bool>::CreateSP(this, &FItemPropertiesNodeBuilder::IsNumericEditable, Key))
			];
		}
	}

private:
	enum class ESource : uint8
	{
		NoContext, // no data asset can be determined -> show keys already stored
		NoAsset,   // context found, but the asset is not assigned
		Asset,     // keys come from UItemDataAsset::DefaultProperties
		AssetDefaults // allow selecting keys on the data asset itself
	};


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

	ESource ResolveKeys(TArray<EItemNumericProperty>& OutKeys) const
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

		if (GetProps() == &Asset->DefaultProperties)
		{
			const UEnum* Enum = StaticEnum<EItemNumericProperty>();
			for (int32 Index = 0; Index < Enum->NumEnums(); ++Index)
			{
				if (!Enum->HasMetaData(TEXT("Hidden"), Index))
				{
					OutKeys.Add(static_cast<EItemNumericProperty>(Enum->GetValueByIndex(Index)));
				}
			}
			return ESource::AssetDefaults;
		}
		OutKeys = Asset->DefaultProperties.PropertyKeys;
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

	ECheckBoxState GetKeyCheckState(EItemNumericProperty Key) const
	{
		TArray<void*> Raw;
		StructHandle->AccessRawData(Raw);
		TOptional<bool> Enabled;
		for (const void* Ptr : Raw)
		{
			if (!Ptr) continue;
			const bool bEnabled = static_cast<const FItemProperties*>(Ptr)->PropertyKeys.Contains(Key);
			if (Enabled.IsSet() && Enabled.GetValue() != bEnabled) return ECheckBoxState::Undetermined;
			Enabled = bEnabled;
		}
		return Enabled.Get(false) ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
	}

	bool IsNumericEditable(EItemNumericProperty Key) const
	{
		return IsEditable() && (DisplayedSource != ESource::AssetDefaults || GetKeyCheckState(Key) == ECheckBoxState::Checked);
	}

	void OnKeyCheckStateChanged(ECheckBoxState State, EItemNumericProperty Key)
	{
		if (!IsEditable() || State == ECheckBoxState::Undetermined) return;
		const FScopedTransaction Scoped(LOCTEXT("EditEnabledItemProp", "Edit Supported Item Property"));
		StructHandle->NotifyPreChange();
		TArray<void*> Raw;
		StructHandle->AccessRawData(Raw);
		for (void* Ptr : Raw)
		{
			if (!Ptr) continue;
			FItemProperties& Props = *static_cast<FItemProperties*>(Ptr);
			Props.PropertyValues.SetNum(Props.PropertyKeys.Num());
			if (State == ECheckBoxState::Checked)
			{
				if (!Props.PropertyKeys.Contains(Key))
				{
					Props.PropertyKeys.Add(Key);
					Props.PropertyValues.Add(0.f);
				}
			}
			else
			{
				for (int32 Index = Props.PropertyKeys.Num() - 1; Index >= 0; --Index)
				{
					if (Props.PropertyKeys[Index] == Key)
					{
						Props.PropertyKeys.RemoveAt(Index);
						Props.PropertyValues.RemoveAt(Index);
					}
				}
			}
		}
		StructHandle->NotifyPostChange(EPropertyChangeType::ValueSet);
	}

	TOptional<float> GetNumericValueOptional(EItemNumericProperty Key) const
	{
		TArray<void*> Raw;
		StructHandle->AccessRawData(Raw);
		TOptional<float> Result;
		for (const void* Ptr : Raw)
		{
			if (!Ptr) continue;
			const FItemProperties& Props = *static_cast<const FItemProperties*>(Ptr);
			const int32 Idx = Props.PropertyKeys.Find(Key);
			const float Value = Props.PropertyValues.IsValidIndex(Idx) ? Props.PropertyValues[Idx] : 0.f;
			if (Result.IsSet() && !FMath::IsNearlyEqual(Result.GetValue(), Value)) return {};
			Result = Value;
		}
		return Result;
	}

	void WriteNumericValue(EItemNumericProperty Key, float Value)
	{
		TArray<void*> Raw;
		StructHandle->AccessRawData(Raw);
		for (void* Ptr : Raw)
		{
			if (!Ptr)
			{
				continue;
			}

			FItemProperties& Props = *static_cast<FItemProperties*>(Ptr);

			// Keep keys and values parallel, as FItemData::SetNumericProp does
			int32 Idx = Props.PropertyKeys.Find(Key);
			if (Idx == INDEX_NONE)
			{
				Idx = Props.PropertyKeys.Add(Key);
			}
			if (Props.PropertyValues.Num() < Props.PropertyKeys.Num())
			{
				Props.PropertyValues.SetNumZeroed(Props.PropertyKeys.Num());
			}
			Props.PropertyValues[Idx] = Value;
		}
	}

	void OnEntryCommitted(float Value, ETextCommit::Type, EItemNumericProperty Key)
	{
		// Losing focus without a real change must not dirty the asset
		if (!IsNumericEditable(Key))
		{
			return;
		}
		TArray<void*> Raw;
		StructHandle->AccessRawData(Raw);
		bool bNeedsWrite = false;
		for (const void* Ptr : Raw)
		{
			if (!Ptr) continue;
			const FItemProperties& Props = *static_cast<const FItemProperties*>(Ptr);
			const int32 Idx = Props.PropertyKeys.Find(Key);
			if (!Props.PropertyValues.IsValidIndex(Idx) || !FMath::IsNearlyEqual(Value, Props.PropertyValues[Idx]))
			{
				bNeedsWrite = true;
				break;
			}
		}
		if (!bNeedsWrite) return;

		const FScopedTransaction Scoped(LOCTEXT("EditItemProp", "Edit Item Property"));
		StructHandle->NotifyPreChange();
		WriteNumericValue(Key, Value);
		StructHandle->NotifyPostChange(EPropertyChangeType::ValueSet);
	}

	TSharedRef<IPropertyHandle> StructHandle;
	TSharedPtr<IPropertyUtilities> Utils;
	FSimpleDelegate OnRebuild;

	TArray<EItemNumericProperty> DisplayedKeys;
	ESource DisplayedSource = ESource::NoContext;

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