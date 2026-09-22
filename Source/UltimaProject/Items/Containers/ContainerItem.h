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

UCLASS()
class ULTIMAPROJECT_API UContainerItem : public UUserWidget
{
	GENERATED_BODY()

	UObject* GetTemperatureIcon(uint8 Temperature) const;
	void AddStatusIcon(UObject* ResourceObject);

protected:
	// Item data

	UPROPERTY(BlueprintReadOnly)
	FItemDataDefinition CurrentItem;

	UPROPERTY(BlueprintReadOnly)
	int32 ContainerItemHandle = FContainerItemData::InvalidHandle;

	// Icons
	// Icons - Humidity
	UPROPERTY(EditDefaultsOnly, Category="Status icons")
	UObject* IconStatusWet;

	UPROPERTY(EditDefaultsOnly, Category="Status icons")
	UObject* IconStatusHumid;

	// Icons - Temperature
	UPROPERTY(EditDefaultsOnly, Category="Status icons")
	UObject* IconStatusHot;

	UPROPERTY(EditDefaultsOnly, Category="Status icons")
	UObject* IconStatusBurning;

	UPROPERTY(EditDefaultsOnly, Category="Status icons")
	UObject* IconStatusCold;

	UPROPERTY(EditDefaultsOnly, Category="Status icons")
	UObject* IconStatusFrozen;

	// Icons - Condition
	UPROPERTY(EditDefaultsOnly, Category="Status icons")
	UObject* IconStatusDamaged;

	UPROPERTY(EditDefaultsOnly, Category="Status icons")
	UObject* IconStatusDestroyed;

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
	
	UFUNCTION(BlueprintCallable)
	UContainerWidget* GetContainerWidget() const;
	
	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool IsEmpty() const;
};

inline bool UContainerItem::IsEmpty() const
{
	return !CurrentItem.IsValid();
}
