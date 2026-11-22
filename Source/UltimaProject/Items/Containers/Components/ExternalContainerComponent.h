// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

// Game includes
#include "UltimaProject/Items/Containers/ContainerComponent.h"

// Generated include
#include "ExternalContainerComponent.generated.h"

/**
 * External container component (chests, shelves, pockets)
 */
UCLASS(ClassGroup=(Containers), meta=(BlueprintSpawnableComponent))
class ULTIMAPROJECT_API UExternalContainerComponent	: public UContainerComponent, public IHeartbeatInterface
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere)
	TArray<FItemDataDefinition> DefaultItems;

	// Default liquids. Value is percentage of the capacity

	UPROPERTY(EditAnywhere)
	TArray<FLiquidDescriptor> Test;

	UPROPERTY(EditAnywhere, meta=(ClampMin=0, ClampMax=1))
	TMap<FLiquidDescriptor, float> DefaultLiquids;

	// An empty container flag
	UPROPERTY(VisibleInstanceOnly, Replicated)
	bool bIsExplicitlyEmpty = false;

public:
	UExternalContainerComponent();


	// UActorComponent
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	// ~UActorComponent

	// UContainerComponent
	virtual void NotifyContainerItemsChanged_Implementation() override;
	virtual bool IsEmpty() const override;
	// ~UContainerComponent
	
	// IHeartbeatInterface
	virtual bool Heartbeat_Implementation(int64 CurrentTime, int32 TimePassed) override;
	// ~IHeartbeatInterface
};
