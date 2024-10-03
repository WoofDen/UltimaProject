// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ItemData.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/Actor.h"
#include "Item.generated.h"

/**
 * In-world representation of every item
 */
UCLASS(Abstract, BlueprintType)
class ULTIMAPROJECT_API AItem : public AActor
{
	GENERATED_BODY()

protected:
	UPROPERTY()
	TObjectPtr<USphereComponent> SphereComponent;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly)
	TObjectPtr<UStaticMeshComponent> StaticMeshComponent;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	TObjectPtr<UWidgetComponent> HoverWidget;

	// Data object represents current item.
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UItemData> ItemData;

	// Static data for item initialization
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<const UItemDataAsset> DefaultStaticData;

	// Dynamic data for item initialization
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FItemInstanceData DefaultInstanceData;

public:
	AItem();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	bool SetItemData(UItemData* NewData = nullptr);

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;
};
