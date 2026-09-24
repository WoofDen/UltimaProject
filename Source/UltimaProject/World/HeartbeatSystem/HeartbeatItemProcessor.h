// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UltimaProject/Items/Containers/ContainerComponent.h"
#include "UObject/Interface.h"
#include "HeartbeatItemProcessor.generated.h"

/**
 * Server-only objects that externally implement heartbeat logic for certain item categories
 */
UCLASS(Blueprintable)
class ULTIMAPROJECT_API UHeartbeatItemProcessor : public UObject
{
	GENERATED_BODY()

	static TMap<const UClass*, UHeartbeatItemProcessor*> InstancePool;
	
protected:
	uint8 HumidityChangePerMinute = 50;
	uint8 TemperatureChangePerMinute = 70;
	
	uint8 CalculateProp(uint8 Current, uint8 Target, uint8 SpeedPerMinute, uint64 TimePassed);
	virtual bool HeartbeatInternal(FItemData& ItemData, bool bWorldItem, int64 CurrentTime, int32 TimePassed);
	
	virtual void BeginDestroy() override;

public:
	virtual bool ProcessHeartbeat(AItem* WorldItem, int64 CurrentTime, int32 TimePassed);
	virtual bool ProcessHeartbeat(FContainerItemData& ItemData, int64 CurrentTime, int32 TimePassed);

	static UHeartbeatItemProcessor* GetInstance(const UClass* InstanceClass);
};
