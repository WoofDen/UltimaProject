#include "LiquidDescriptorCustomization.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "IDetailChildrenBuilder.h"
#include "PropertyEditorModule.h"
#include "PropertyHandle.h"
#include "ScopedTransaction.h"
#include "Modules/ModuleManager.h"
#include "UltimaProject/Items/Containers/ContainerTypes.h"
#include "UObject/UObjectIterator.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SSearchableComboBox.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "LiquidDescriptorCustomization"

namespace LiquidDescriptorEditor
{
	static const TCHAR* NoneLabel = TEXT("(None)");

	static bool IsSelectableEntry(const UEnum* Enum, int32 Index)
	{
		if (Enum->HasMetaData(TEXT("Hidden"), Index))
		{
			return false;
		}
		if (Index == Enum->NumEnums() - 1 && Enum->GetNameStringByIndex(Index).EndsWith(TEXT("_MAX")))
		{
			return false;
		}
		return true;
	}

	/** Include enum if all its value fits in 64 mask [0-63]. */
	static bool IsEligibleEnum(const UEnum* Enum)
	{
		if (!Enum || Enum->IsTemplate())
		{
			return false;
		}

		int32 Count = 0;
		for (int32 i = 0; i < Enum->NumEnums(); ++i)
		{
			if (!IsSelectableEntry(Enum, i))
			{
				continue;
			}
			const int64 Value = Enum->GetValueByIndex(i);
			if (Value < 0 || Value >= 64)
			{
				return false;
			}
			++Count;
		}
		return Count > 0;
	}
}

TSharedRef<SWidget> FLiquidMetaFlagsBuilder::MakePickerWidget()
{
	RebuildOptions();

	return SNew(SSearchableComboBox)
		.OptionsSource(&Options)
		.InitiallySelectedItem(FindCurrentOption())
		.OnGenerateWidget_Lambda([](TSharedPtr<FString> Item)
		{
			return SNew(STextBlock)
				.Font(IDetailLayoutBuilder::GetDetailFont())
				.Text(FText::FromString(*Item));
		})
		.OnSelectionChanged(SSearchableComboBox::FOnSelectionChanged::CreateSP(
			this, &FLiquidMetaFlagsBuilder::OnEnumPicked))
		[
			SNew(STextBlock)
			.Font(IDetailLayoutBuilder::GetDetailFont())
			.Text(MakeAttributeSP(this, &FLiquidMetaFlagsBuilder::GetCurrentEnumText))
		];
}

void FLiquidMetaFlagsBuilder::GenerateHeaderRowContent(FDetailWidgetRow& NodeRow)
{
	NodeRow.NameContent()
	[
		SNew(STextBlock)
		.Font(IDetailLayoutBuilder::GetDetailFont())
		.Text(MakeAttributeSP(this, &FLiquidMetaFlagsBuilder::GetHeaderText))
	];
}

void FLiquidMetaFlagsBuilder::GenerateChildContent(IDetailChildrenBuilder& ChildrenBuilder)
{
	IDetailCustomNodeBuilder::GenerateChildContent(ChildrenBuilder);

	const UEnum* Enum = GetCurrentEnum();
	if (!Enum)
	{
		return;
	}

	for (int32 i = 0; i < Enum->NumEnums(); ++i)
	{
		if (!LiquidDescriptorEditor::IsSelectableEntry(Enum, i))
		{
			continue;
		}

		const int64 Value = Enum->GetValueByIndex(i);
		if (Value < 0 || Value >= 64)
		{
			continue;
		}

		const uint64 Bit = uint64(1) << Value;
		const FText DisplayName = Enum->GetDisplayNameTextByIndex(i);

		ChildrenBuilder.AddCustomRow(DisplayName)
		               .NameContent()
			[
				SNew(STextBlock)
				.Font(IDetailLayoutBuilder::GetDetailFont())
				.Text(DisplayName)
				.ToolTipText(FText::Format(LOCTEXT("BitTooltip", "Bit {0}"), FText::AsNumber(Value)))
			]
			.ValueContent()
			[
				SNew(SCheckBox)
				.IsChecked(MakeAttributeSP(this, &FLiquidMetaFlagsBuilder::GetBitState, Bit))
				.OnCheckStateChanged(FOnCheckStateChanged::CreateSP(
					this, &FLiquidMetaFlagsBuilder::OnBitChanged, Bit))
			];
	}
}

FText FLiquidMetaFlagsBuilder::GetCurrentEnumText() const
{
	UObject* Object = nullptr;
	const FPropertyAccess::Result Result = MetaEnumHandle->GetValue(Object);
	if (Result == FPropertyAccess::MultipleValues)
	{
		return LOCTEXT("MultipleValues", "Multiple Values");
	}
	if (const UEnum* Enum = Cast<UEnum>(Object))
	{
		return FText::FromString(Enum->GetName());
	}
	return FText::FromString(LiquidDescriptorEditor::NoneLabel);
}

FText FLiquidMetaFlagsBuilder::GetHeaderText() const
{
	if (const UEnum* Enum = GetCurrentEnum())
	{
		return FText::Format(LOCTEXT("FlagsHeader", "Meta Flags ({0})"), FText::FromString(Enum->GetName()));
	}

	return LOCTEXT("FlagsHeaderNone", "Meta Flags (enum not selected)");
}

void FLiquidMetaFlagsBuilder::RebuildOptions()
{
	Options.Reset();
	OptionToEnum.Reset();
	Options.Add(MakeShared<FString>(LiquidDescriptorEditor::NoneLabel));

	TArray<UEnum*> Enums;
	for (TObjectIterator<UEnum> It; It; ++It)
	{
		if (LiquidDescriptorEditor::IsEligibleEnum(*It))
		{
			Enums.Add(*It);
		}
	}
	Enums.Sort([](const UEnum& A, const UEnum& B) { return A.GetName() < B.GetName(); });

	for (UEnum* Enum : Enums)
	{
		FString Key = Enum->GetName();
		if (OptionToEnum.Contains(Key))
		{
			Key = Enum->GetPathName();
		}
		OptionToEnum.Add(Key, Enum);
		Options.Add(MakeShared<FString>(Key));
	}
}

TSharedPtr<FString> FLiquidMetaFlagsBuilder::FindCurrentOption() const
{
	if (const UEnum* Current = GetCurrentEnum())
	{
		for (int32 i = 1; i < Options.Num(); ++i)
		{
			if (const TWeakObjectPtr<UEnum>* Found = OptionToEnum.Find(*Options[i]))
			{
				if (Found->Get() == Current)
				{
					return Options[i];
				}
			}
		}
	}

	return Options[0];
}

void FLiquidMetaFlagsBuilder::OnEnumPicked(TSharedPtr<FString> Item, ESelectInfo::Type SelectInfo)
{
	if (!Item.IsValid() || SelectInfo == ESelectInfo::Direct)
	{
		return;
	}

	const UObject* NewEnum = nullptr;
	if (const TWeakObjectPtr<UEnum>* Found = OptionToEnum.Find(*Item))
	{
		NewEnum = Found->Get();
	}

	MetaEnumHandle->SetValue(NewEnum);

	OnRebuildChildren.ExecuteIfBound();
}

ECheckBoxState FLiquidMetaFlagsBuilder::GetBitState(uint64 Bit) const
{
	int32 Num = 0;
	bool bAny = false;
	bool bAll = true;

	MetaFlagsHandle->EnumerateConstRawData([&](const void* RawData, const int32, const int32)
	{
		if (RawData)
		{
			const bool bSet = (*static_cast<const uint64*>(RawData) & Bit) != 0;
			bAny |= bSet;
			bAll &= bSet;
			++Num;
		}
		return true;
	});

	if (Num == 0)
	{
		return ECheckBoxState::Unchecked;
	}
	return bAll ? ECheckBoxState::Checked : (bAny ? ECheckBoxState::Undetermined : ECheckBoxState::Unchecked);
}

void FLiquidMetaFlagsBuilder::OnBitChanged(ECheckBoxState NewState, uint64 Bit)
{
	const bool bOn = (NewState == ECheckBoxState::Checked);

	const FScopedTransaction Transaction(LOCTEXT("ToggleFlag", "Toggle Liquid Meta Flag"));

	TArray<UObject*> OuterObjects;
	MetaFlagsHandle->GetOuterObjects(OuterObjects);
	for (UObject* Object : OuterObjects)
	{
		Object->Modify();
	}

	MetaFlagsHandle->NotifyPreChange();
	MetaFlagsHandle->EnumerateRawData([&](void* RawData, const int32, const int32)
	{
		if (RawData)
		{
			uint64& Flags = *static_cast<uint64*>(RawData);
			Flags = bOn ? (Flags | Bit) : (Flags & ~Bit);
		}
		return true;
	});
	MetaFlagsHandle->NotifyPostChange(EPropertyChangeType::ValueSet);
	MetaFlagsHandle->NotifyFinishedChangingProperties();
}

TSharedRef<IPropertyTypeCustomization> FLiquidDescriptorCustomization::MakeInstance()
{
	return MakeShared<FLiquidDescriptorCustomization>();
}

void FLiquidDescriptorCustomization::Register()
{
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyModule.RegisterCustomPropertyTypeLayout(
		FLiquidDescriptor::StaticStruct()->GetFName(),
		FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FLiquidDescriptorCustomization::MakeInstance));
}

void FLiquidDescriptorCustomization::Unregister()
{
	if (FModuleManager::Get().IsModuleLoaded("PropertyEditor"))
	{
		FPropertyEditorModule& PropertyModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
		PropertyModule.UnregisterCustomPropertyTypeLayout(FLiquidDescriptor::StaticStruct()->GetFName());
	}
}

void FLiquidDescriptorCustomization::CustomizeHeader(TSharedRef<IPropertyHandle> StructHandle,
                                                     FDetailWidgetRow& HeaderRow,
                                                     IPropertyTypeCustomizationUtils& Utils)
{
	TypeHandle = StructHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLiquidDescriptor, Type));
	MetaFlagsHandle = StructHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLiquidDescriptor, MetaFlags));
	MetaEnumHandle = StructHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLiquidDescriptor, MetaFlagsEnum));

	HeaderRow
		.NameContent()
		[
			StructHandle->CreatePropertyNameWidget()
		]
		.ValueContent()
		.MinDesiredWidth(200.f)
		[
			SNew(STextBlock)
			.Font(IDetailLayoutBuilder::GetDetailFont())
			.Text_Lambda([TypeH = TypeHandle, FlagsH = MetaFlagsHandle]()
			{
				FText TypeText;
				TypeH->GetValueAsDisplayText(TypeText);

				uint64 Flags = 0;
				if (FlagsH->GetValue(Flags) == FPropertyAccess::Success)
				{
					return FText::Format(LOCTEXT("Summary", "{0}  [{1} flags]"),
					                     TypeText, FText::AsNumber(FPlatformMath::CountBits64(Flags)));
				}
				return FText::Format(LOCTEXT("SummaryMulti", "{0}  [...]"), TypeText);
			})
		];
}

void FLiquidDescriptorCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> StructHandle,
                                                       IDetailChildrenBuilder& ChildBuilder,
                                                       IPropertyTypeCustomizationUtils& Utils)
{
	ChildBuilder.AddProperty(TypeHandle.ToSharedRef());

	TSharedRef<FLiquidMetaFlagsBuilder> FlagsBuilder = MakeShared<FLiquidMetaFlagsBuilder>(MetaFlagsHandle.ToSharedRef(), MetaEnumHandle.ToSharedRef());

	ChildBuilder.AddCustomRow(LOCTEXT("MetaEnumRow", "Meta Enum"))
	            .NameContent()
		[
			SNew(STextBlock)
			.Font(IDetailLayoutBuilder::GetDetailFont())
			.Text(LOCTEXT("MetaEnumLabel", "Meta Enum"))
			.ToolTipText(LOCTEXT("MetaEnumTip", "Enum flag value"))
		]
		.ValueContent()
		.MinDesiredWidth(200.f)
		[
			FlagsBuilder->MakePickerWidget()
		];

	ChildBuilder.AddCustomBuilder(FlagsBuilder);
}

#undef LOCTEXT_NAMESPACE
