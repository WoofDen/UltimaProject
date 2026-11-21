#pragma once


#include "IDetailCustomNodeBuilder.h"
#include "IPropertyTypeCustomization.h"

class IPropertyHandle;

class FLiquidMetaFlagsBuilder : public IDetailCustomNodeBuilder, public TSharedFromThis<FLiquidMetaFlagsBuilder>
{
public:
	FLiquidMetaFlagsBuilder(TSharedRef<IPropertyHandle> InMetaFlagsHandle, TSharedRef<IPropertyHandle> InMetaEnumHandle)
		: MetaFlagsHandle(InMetaFlagsHandle)
		  , MetaEnumHandle(InMetaEnumHandle)
	{
	}

	TSharedRef<SWidget> MakePickerWidget();

	virtual void SetOnRebuildChildren(FSimpleDelegate InOnRegenerateChildren) override
	{
		OnRebuildChildren = InOnRegenerateChildren;
	}

	virtual FName GetName() const override { return TEXT("LiquidMetaFlags"); }
	virtual bool InitiallyCollapsed() const override { return true; }

	virtual void GenerateHeaderRowContent(FDetailWidgetRow& NodeRow) override;
	virtual void GenerateChildContent(IDetailChildrenBuilder& ChildrenBuilder) override;

	virtual void Tick(float DeltaTime) override
	{
	}

	virtual bool RequiresTick() const override { return false; }

private:
	const UEnum* GetCurrentEnum() const
	{
		UObject* Object = nullptr;
		if (MetaEnumHandle->GetValue(Object) == FPropertyAccess::Success)
		{
			return Cast<UEnum>(Object);
		}
		return nullptr;
	}

	FText GetCurrentEnumText() const;

	FText GetHeaderText() const;

	void RebuildOptions();

	TSharedPtr<FString> FindCurrentOption() const;

	void OnEnumPicked(TSharedPtr<FString> Item, ESelectInfo::Type SelectInfo);

	ECheckBoxState GetBitState(uint64 Bit) const;

	void OnBitChanged(ECheckBoxState NewState, uint64 Bit);

private:
	TSharedRef<IPropertyHandle> MetaFlagsHandle;
	TSharedRef<IPropertyHandle> MetaEnumHandle;

	TArray<TSharedPtr<FString>> Options;
	TMap<FString, TWeakObjectPtr<UEnum>> OptionToEnum;

	FSimpleDelegate OnRebuildChildren;
};

class FLiquidDescriptorCustomization : public IPropertyTypeCustomization
{
public:
	static TSharedRef<IPropertyTypeCustomization> MakeInstance();

	static void Register();
	static void Unregister();

	virtual void CustomizeHeader(TSharedRef<IPropertyHandle> StructHandle,
	                             FDetailWidgetRow& HeaderRow,
	                             IPropertyTypeCustomizationUtils& Utils) override;

	virtual void CustomizeChildren(TSharedRef<IPropertyHandle> StructHandle,
	                               IDetailChildrenBuilder& ChildBuilder,
	                               IPropertyTypeCustomizationUtils& Utils) override;

private:
	TSharedPtr<IPropertyHandle> TypeHandle;
	TSharedPtr<IPropertyHandle> MetaFlagsHandle;
	TSharedPtr<IPropertyHandle> MetaEnumHandle;
};
