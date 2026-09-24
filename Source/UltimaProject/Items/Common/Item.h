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
	void OnRep_ItemData();

protected:
	UPROPERTY(EditAnywhere)
	TObjectPtr<USphereComponent> SphereComponent;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly)
	TObjectPtr<UStaticMeshComponent> StaticMeshComponent;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	TObjectPtr<UWidgetComponent> HoverWidget;

	// Data object represents current item.
	UPROPERTY(BlueprintReadOnly, Replicated, VisibleInstanceOnly, Category="Runtime data",
		ReplicatedUsing=OnRep_ItemData)
	FItemData ItemData = FItemData::EmptyItem;

	// Static data for item initialization
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<const UItemDataAsset> DefaultStaticData;

	// Dynamic data for item initialization
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FItemInstanceData DefaultInstanceData;

	virtual void PostInitializeComponents() override;

	UFUNCTION(BlueprintImplementableEvent)
	void OnItemDataChanged();

public:
	AItem();
	
	// AActor
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	// AActor

	void RemoveFromWorld();

	bool SetItemData(FItemData&& NewData);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void Tick(float DeltaTime) override;

	FItemData& GetItemDataMutable()
	{
		// self-defence against modifying ItemData somewhere else than server
		check(HasAuthority());
		return ItemData;
	}

	const FItemData GetItemDataConst() const { return ItemData; }
	
	// IHeartbeatInterface
	virtual bool Heartbeat_Implementation(int64 CurrentTime, int32 TimePassed) override;
	// ~IHeartbeatInterface
};
