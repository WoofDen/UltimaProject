// Fill out your copyright notice in the Description page of Project Settings.

#include "HeartbeatItemProcessor.h"
#include "UltimaProject/Common/Macro.h"

TMap<const UClass*, UHeartbeatItemProcessor*> UHeartbeatItemProcessor::InstancePool;

bool UHeartbeatItemProcessor::HeartbeatInternal_Implementation(FItemData& ItemData, const FItemDataLocation& ItemDataLocation, int64 CurrentTime, int32 TimePassed)
{
	bool bChanged = false;

	bChanged |= ProcessTemperature(ItemData, ItemDataLocation, TimePassed);
	bChanged |= ProcessHumidity(ItemData, ItemDataLocation, TimePassed);

	if (bChanged)
	{
		if (ItemDataLocation.ContainerComponent.IsValid())
		{
			ItemDataLocation.ContainerComponent->OnExternalItemChange(ItemDataLocation.ContainerItemHandle);
		}
	}

	return true;
}

void UHeartbeatItemProcessor::BeginDestroy()
{
	UClass* Class = GetClass();
	InstancePool.Remove(Class);

	Super::BeginDestroy();
}

float UHeartbeatItemProcessor::CalculateProp(float Current, float Target, float SpeedPerMinute, uint64 TimePassed)
{
	if (Target == Current)
	{
		return Target;
	}

	const float Delta = static_cast<float>(TimePassed) / 60.f * SpeedPerMinute;

	if (Target > Current)
	{
		return FMath::Min(Current + Delta, Target);
	}

	return FMath::Max(Current - Delta, Target);
}

float UHeartbeatItemProcessor::CalculateTemperature(float EnvironmentTemperature, float ItemTemperature, float ThermalCoefficient, float DeltaTime) const
{
	const float Alpha = 1.0f - FMath::Exp(-ThermalCoefficient * DeltaTime);
	return FMath::Lerp(ItemTemperature, EnvironmentTemperature, Alpha);
}

bool UHeartbeatItemProcessor::ProcessTemperature(FItemData& ItemData, const FItemDataLocation& ItemDataLocation, int32 TimePassed)
{
	// Temperature
	const float CurrentTemperature = ItemData.GetNumericProp(EItemNumericProperty::Temperature);
	const float NewTemperature = CalculateTemperature(AmbientTemperature, CurrentTemperature, ThermalExchangeCoefficient, TimePassed);
	ItemData.SetNumericProp(EItemNumericProperty::Temperature, NewTemperature);

	return true;
}

bool UHeartbeatItemProcessor::ProcessHumidity(FItemData& ItemData, const FItemDataLocation& ItemLocation, int32 TimePassed)
{
	const float CurrentHumidityLevel = ItemData.GetNumericProp(EItemNumericProperty::Humidity);

	// Humidity grows if there is a liquid
	// Even with Humidity 1.0f the liquid will be consumed to sustain this level
	if (ItemLocation.ContainerComponent.IsValid()
		&& ItemLocation.ContainerComponent->CanStoreLiquids()
		&& ItemLocation.ContainerComponent->GetLiquidCapacityUsed(false) > 0)
	{
		const float SoakCapacity = ItemData.GetAmount() * HumidityConsumptionRate;
		const float SoakAmountReal = FMath::Min(ItemLocation.ContainerComponent->GetLiquidCapacityUsed(false), SoakCapacity);

		TMap<FLiquidDescriptor, float> ExtractedLiquids = ItemLocation.ContainerComponent->ExtractLiquid(SoakAmountReal);

		if (!ExtractedLiquids.IsEmpty())
		{
			const float SoakResult = SoakAmountReal / SoakCapacity;
			const float HumidityDelta = HumidityGrowRate * SoakResult * TimePassed;
			const float HumidityNewValue = FMath::Clamp(CurrentHumidityLevel + HumidityDelta, 0, 1.f);

			ItemData.SetNumericProp(EItemNumericProperty::Humidity, HumidityNewValue);

			return true;
		}
	}

	// A world item or a dry container
	// Humidity loss
	if (CurrentHumidityLevel > 0)
	{
		const float HumidityDelta = HumidityGrowRate * TimePassed;
		const float HumidityNewValue = FMath::Clamp(CurrentHumidityLevel - HumidityDelta, 0, 1.f);

		ItemData.SetNumericProp(EItemNumericProperty::Humidity, HumidityNewValue);

		return true;
	}

	return false;
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
		ensureAlways(IsValid(InstancePool[InstanceClass]));
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

	NULLCHECK_RETURN(InstanceClass, nullptr);

	InstancePool.Add(InstanceClass, NewInstance);
	return NewInstance;
}
