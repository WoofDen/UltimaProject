// Fill out your copyright notice in the Description page of Project Settings.

#include "HeartbeatItemProcessor.h"
#include "UltimaProject/Common/Macro.h"

TMap<const UClass*, UHeartbeatItemProcessor*> UHeartbeatItemProcessor::InstancePool;

bool UHeartbeatItemProcessor::HeartbeatInternal(FItemData& ItemData, bool bWorldItem, int64 CurrentTime, int32 TimePassed)
{
	if (bWorldItem)
	{
		{
			// Temperature
			const uint8 Target = UP::ItemProperty::Temperature::Default;
			const uint8 Current = ItemData.GetIntProp(EItemIntProperty::Temperature);

			if (Target != Current)
			{
				const uint8 Result = CalculateProp(Current, Target, TemperatureChangePerMinute, TimePassed);
				ItemData.SetProp(EItemIntProperty::Temperature, Result);
			}
		}

		{
			// Humidity
			// TODO ... a rain... one day
			const uint8 Target = UP::ItemProperty::Humidity::Default;
			const uint8 Current = ItemData.GetIntProp(EItemIntProperty::Humidity);

			if (Target == Current)
			{
				const uint8 Result = CalculateProp(Current, Target, HumidityChangePerMinute, TimePassed);
				ItemData.SetProp(EItemIntProperty::Humidity, Result);
			}
		}
	}

	return true;
}

void UHeartbeatItemProcessor::BeginDestroy()
{
	if (UObject* Instance = GetInstance(GetClass()); IsValid(Instance))
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

// Add default functionality here for any IHeartbeatProcessorInterface functions that are not pure virtual.
bool UHeartbeatItemProcessor::ProcessHeartbeat(AItem* WorldItem, int64 CurrentTime, int32 TimePassed)
{
	NULLCHECK_RETURN(WorldItem, false);

	FItemData& ItemData = WorldItem->GetItemDataMutable();
	return HeartbeatInternal(ItemData, true, CurrentTime, TimePassed);
}

bool UHeartbeatItemProcessor::ProcessHeartbeat(FContainerItemData& ItemData, int64 CurrentTime, int32 TimePassed)
{
	return HeartbeatInternal(ItemData.GetItemDataMutable(), false, CurrentTime, TimePassed);
}

UHeartbeatItemProcessor* UHeartbeatItemProcessor::GetInstance(const UClass* InstanceClass)
{
	NULLCHECK_RETURN(InstanceClass, nullptr);

	if (InstancePool.Contains(InstanceClass))
	{
		return InstancePool[InstanceClass];
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
