// Fill out your copyright notice in the Description page of Project Settings.

#include "GrainHeartbeatItemProcessor.h"

bool UGrainHeartbeatItemProcessor::HeartbeatInternal_Implementation(FItemData& ItemData, const FItemDataLocation& ItemLocation, int64 CurrentTime, int32 TimePassed)
{
	if (!Super::HeartbeatInternal_Implementation(ItemData, ItemLocation, CurrentTime, TimePassed))
	{
		return false;
	}

	// Grow seeds if they are soaked
	const float Humidity = ItemData.GetNumericProp(EItemNumericProperty::Humidity);
	const float Temperature = ItemData.GetNumericProp(EItemNumericProperty::Temperature);
	if (Humidity > .5f && Temperature <= 70.f)
	{
		const float GrowCurrent = ItemData.GetNumericProp(EItemNumericProperty::Grow);
		constexpr float GrowTarget = static_cast<float>(UINT8_MAX);

		const float GrowResult = CalculateProp(GrowCurrent, GrowTarget, GerminationPerMinute, TimePassed);
		ItemData.SetNumericProp(EItemNumericProperty::Grow, GrowResult);
	}

	return true;
}
