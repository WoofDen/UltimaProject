#pragma once

#include "CoreMinimal.h"
#include "ContainerComponent.h"
#include "Blueprint/UserWidget.h"
#include "Components/HorizontalBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "ContainerItem.generated.h"

class UTextLayoutWidget;
class UImage;

USTRUCT(BlueprintType)
struct FItemPropIconSettings
{
	GENERATED_BODY()

	FItemPropIconSettings() = default;

	UPROPERTY(EditAnywhere, Category="UP", meta=(ClampMin = 0.f, ClampMax = 1.f))
	TArray<FVector2f> PropertyRanges;

	UPROPERTY(EditDefaultsOnly, Category="UP")
	TArray<TObjectPtr<UObject>> Icons;
};

UCLASS()
class ULTIMAPROJECT_API UContainerItem : public UUserWidget
{
	GENERATED_BODY()

	void AddStatusIcon(UObject* ResourceObject);
	void BuildPropertyIcons(const FItemData& ItemData);

protected:
	// Item data

	UPROPERTY(BlueprintReadOnly)
	FItemDataDefinition CurrentItem;

	UPROPERTY(BlueprintReadOnly)
	int32 ContainerItemHandle = FContainerItemData::InvalidHandle;

	UPROPERTY(EditDefaultsOnly)
	TMap<EItemIntProperty, FItemPropIconSettings> PropertyIcons;

	// Widgets
	UPROPERTY(EditDefaultsOnly, meta=(BindWidget))
	TObjectPtr<UImage> ItemIcon;

	UPROPERTY(EditDefaultsOnly, meta=(BindWidget))
	TObjectPtr<UTextBlock> ItemAmount;

	UPROPERTY(EditDefaultsOnly, meta=(BindWidget))
	TObjectPtr<UHorizontalBox> StatusList;

	UPROPERTY(EditDefaultsOnly)
	FSlateBrush StatusBrushSettings;

public:
	// UUserWidget
	virtual void NativePreConstruct() override;
	// ~UUserWidget

	UFUNCTION(BlueprintCallable)
	void SetItem(const FContainerItemData& Item);

	UFUNCTION(BlueprintCallable)
	void ResetItem();

	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool IsEmpty() const;
};

inline bool UContainerItem::IsEmpty() const
{
	return !CurrentItem.IsValid();
}
