// Fill out your copyright notice in the Description page of Project Settings.

#include "HeartbeatItemProcessor.h"
#include "UltimaProject/Common/Macro.h"

TMap<const UClass*, UHeartbeatItemProcessor*> UHeartbeatItemProcessor::InstancePool;

bool UHeartbeatItemProcessor::HeartbeatInternal_Implementation(FItemData& ItemData, const FItemDataLocation& ItemDataLocation, int64 CurrentTime, int32 TimePassed)
{
	if (!ItemDataLocation.Item.IsValid())
	{
		return false;
	}

	ProcessTemperature(ItemData, ItemDataLocation, TimePassed);
	ProcessHumidity(ItemData, ItemDataLocation, TimePassed);

	return true;
}

void UHeartbeatItemProcessor::BeginDestroy()
{
	if (UObject* Instance = GetInstance(GetClass(), false); IsValid(Instance))
	{
		InstancePool.Remove(GetClass());;
		Instance->MarkAsGarbage();
	}

	Super::BeginDestroy();
}

uint8 UHeartbeatItemProcessor::CalculateProp(uint8 Current, uint8 Target, uint8 SpeedPerMinute, uint64 TimePassed)
{
	if (Target == Current)
	{
		return Target;
	}

	uint8 Delta = FMath::Min((TimePassed / 60) * SpeedPerMinute, static_cast<uint64>(UINT8_MAX));

	if (Target > Current)
	{
		return static_cast<uint8>(
			FMath::Min<uint64>(Current + Delta, Target)
		);
	}

	return static_cast<uint8>(
		FMath::Max<int64>(
			static_cast<int64>(Current) - static_cast<int64>(Delta),
			Target
		)
	);
}

float UHeartbeatItemProcessor::CalculateTemperature(float EnvironmentTemperature, float ItemTemperature, float ThermalCoefficient, float DeltaTime) const
{
	const float Alpha = 1.0f - FMath::Exp(-ThermalCoefficient * DeltaTime);
	return FMath::Lerp(ItemTemperature, EnvironmentTemperature, Alpha);
}

void UHeartbeatItemProcessor::ProcessTemperature(FItemData& ItemData, const FItemDataLocation& ItemDataLocation, int32 TimePassed)
{
	// Temperature
	const float TargetTemperatureReal = UP::ItemProperty::Temperature::Default;
	const uint8 CurrentTemperatureProp = ItemData.GetIntProp(EItemIntProperty::Temperature);

	const float CurrentTemperatureReal = UP::ItemProperty::Temperature::Limits.Y * (static_cast<float>(CurrentTemperatureProp) / UINT8_MAX);

	float NewTemperatureReal = CalculateTemperature(TargetTemperatureReal, CurrentTemperatureReal, ThermalExchangeCoefficient, TimePassed);

	uint8 NewTemperatureProp = NewTemperatureReal / UP::ItemProperty::Temperature::Limits.Y * UINT8_MAX;
	ItemData.SetProp(EItemIntProperty::Temperature, NewTemperatureProp);
}

void UHeartbeatItemProcessor::ProcessHumidity(FItemData& ItemData, const FItemDataLocation& ItemLocation, int32 TimePassed)
{
	float CurrentHumidityLevel = ItemData.GetIntPropNormalized(EItemIntProperty::Humidity);

	if (ItemLocation.ContainerComponent.IsValid()
		&& ItemLocation.ContainerComponent->CanStoreLiquids()
		&& ItemLocation.ContainerComponent->GetLiquidCapacityUsed())
	{
		// Humidity grows
		const float SoakCapacity = ItemData.GetAmount() * HumidityConsumptionRate;
		const float SoakAmountReal = FMath::Min(ItemLocation.ContainerComponent->GetLiquidCapacityUsed(), SoakCapacity);

		TMap<FLiquidDescriptor, float> ExtractedLiquids = ItemLocation.ContainerComponent->ExtractLiquid(SoakAmountReal);

		if (!ExtractedLiquids.IsEmpty())
		{
			const float SoakResult = SoakAmountReal / SoakCapacity;
			const float HumidityDelta = HumidityGrowRate * SoakResult * TimePassed;

			uint8 NewValue = FMath::Lerp(0,UINT8_MAX, CurrentHumidityLevel + HumidityDelta);
			
			ItemData.SetProp(EItemIntProperty::Humidity, NewValue);
		}
	}
	else if (CurrentHumidityLevel > 0)
	{
		// A world item or a dry container
		// Humidity loss
	}
}

// Add default functionality here for any IHeartbeatProcessorInterface functions that are not pure virtual.
bool UHeartbeatItemProcessor::ProcessHeartbeat(AItem* WorldItem, int64 CurrentTime, int32 TimePassed)
{
	NULLCHECK_RETURN(WorldItem, false);

	FItemDataLocation ItemLocation;
	ItemLocation.Item = WorldItem;

	FItemData& ItemData = WorldItem->GetItemDataMutable();
	return HeartbeatInternal(ItemData, ItemLocation, CurrentTime, TimePassed);
}

bool UHeartbeatItemProcessor::ProcessHeartbeat(FContainerItemData& ItemData, int64 CurrentTime, int32 TimePassed)
{
	FItemDataLocation ItemLocation;
	ItemLocation.ContainerComponent = ItemData.GetContainerComponent();
	ItemLocation.ContainerItemHandle = ItemData.GetHandle();

	return HeartbeatInternal(ItemData.GetItemDataMutable(), ItemLocation, CurrentTime, TimePassed);
}

UHeartbeatItemProcessor* UHeartbeatItemProcessor::GetInstance(const UClass* InstanceClass, bool bCreateIfMissing)
{
	NULLCHECK_RETURN(InstanceClass, nullptr);

	if (InstancePool.Contains(InstanceClass))
	{
		return InstancePool[InstanceClass];
	}

	if (!bCreateIfMissing)
	{
		return nullptr;
	}

	UHeartbeatItemProcessor* NewInstance = NewObject<UHeartbeatItemProcessor>(
		GetTransientPackage(),
		InstanceClass,
		InstanceClass->GetFName(),
		RF_Transient
	);

	NewInstance->AddToRoot();

	NULLCHECK_RETURN(InstanceClass, nullptr);

	InstancePool.Add(InstanceClass, NewInstance);
	return NewInstance;
}
