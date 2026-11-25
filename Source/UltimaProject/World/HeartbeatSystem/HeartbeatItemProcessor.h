// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

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
	// How many of ml is soaked per second by a single item
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(ClampMin = 0.f))
	float HumidityConsumptionRate = 0.f;

	// % how much humidity is grown per s
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(ClampMin = 0.f, ClampMax = 1.f))
	float HumidityGrowRate = 0.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float AmbientTemperature = 20.f;

	// How fast the whole item loses temperature per second
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float ThermalExchangeCoefficient = 0.01f;

	float CalculateProp(float Current, float Target, float SpeedPerMinute, uint64 TimePassed);

	float CalculateTemperature(float EnvironmentTemperature, float ItemTemperature, float ThermalCoefficient, float DeltaTime) const;

	// Update Temperature prop. Returns true if the prop was changed
	bool ProcessTemperature(FItemData& ItemData, const FItemDataLocation& ItemLocation, int32 TimePassed);

	// Update Humidity prop. Returns true if the prop was changed
	bool ProcessHumidity(FItemData& ItemData, const FItemDataLocation& ItemLocation, int32 TimePassed);

	UFUNCTION(BlueprintNativeEvent)
	bool HeartbeatInternal(UPARAM(ref) FItemData& ItemData, UPARAM(ref) const FItemDataLocation& ItemLocation, int64 CurrentTime, int32 TimePassed);

	// ~UObject
	virtual void BeginDestroy() override;
	// ~UObject end

public:
	virtual bool ProcessHeartbeat(AItem* WorldItem, int64 CurrentTime, int32 TimePassed);
	virtual bool ProcessHeartbeat(FContainerItemData& ItemData, int64 CurrentTime, int32 TimePassed);

	static UHeartbeatItemProcessor* GetInstance(const UClass* InstanceClass, bool bCreateIfMissing = true);
};
