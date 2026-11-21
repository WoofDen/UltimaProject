// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ItemData.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/Actor.h"
#include "UltimaProject/World/HeartbeatSystem/HeartbeatInterface.h"
#include "Item.generated.h"

/**
 * In-world representation of every item
 */
UCLASS(Abstract, BlueprintType)
class ULTIMAPROJECT_API AItem :
	public AActor,
	public IHeartbeatInterface
{
	GENERATED_BODY()

private:
	UFUNCTION()
	void OnRep_ItemData(FItemData PrevValue);

protected:
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<USphereComponent> SphereComponent;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly)
	TObjectPtr<UStaticMeshComponent> StaticMeshComponent;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	TObjectPtr<UWidgetComponent> HoverWidget;

	// Data object represents current item.
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Replicated, Category="Runtime data",
		ReplicatedUsing=OnRep_ItemData)
	FItemData ItemData = FItemData::InvalidItemData;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Item settings")
	FItemDataDefinition ItemDefault;

	virtual void PostInitializeComponents() override;

	UFUNCTION(BlueprintImplementableEvent)
	void OnItemDataChanged();

public:
	AItem();

	// AActor
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual void OnConstruction(const FTransform& Transform) override;
	// AActor

	void RemoveFromWorld();

	bool SetItemData(FItemData&& NewData);

	UFUNCTION(BlueprintCallable)
	virtual void SetStaticData(const UItemDataAsset* ItemDataAsset);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void Tick(float DeltaTime) override;

	FItemData& GetItemDataMutable()
	{
		// self-defense against modifying ItemData somewhere else than server
		check(HasAuthority());
		return ItemData;
	}

	const UItemDataAsset* GetItemStaticData() const { return ItemData.GetStaticData(); }
	const FItemData& GetItemDataConst() const { return ItemData; }

	// IHeartbeatInterface
	virtual bool Heartbeat_Implementation(int64 CurrentTime, int32 TimePassed) override;
	// ~IHeartbeatInterface
};
