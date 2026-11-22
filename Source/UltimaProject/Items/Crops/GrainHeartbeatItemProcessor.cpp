// Fill out your copyright notice in the Description page of Project Settings.

#include "GrainHeartbeatItemProcessor.h"

bool UGrainHeartbeatItemProcessor::HeartbeatInternal_Implementation(FItemData& ItemData, const FItemDataLocation& ItemLocation, int64 CurrentTime, int32 TimePassed)
{
	if (!Super::HeartbeatInternal_Implementation(ItemData, ItemLocation, CurrentTime, TimePassed))
	{
		return false;
	}

	// Grow seeds if they are soaked
	float HumidityNormalized = ItemData.GetIntPropNormalized(EItemIntProperty::Humidity);
	float TemperatureNormalized = ItemData.GetIntPropNormalized(EItemIntProperty::Temperature);
	if (HumidityNormalized > .5f && TemperatureNormalized <= .7f)
	{
		uint8 GrowCurrent = ItemData.GetIntProp(EItemIntProperty::Grow);
		uint8 GrowTarget = UINT8_MAX;

		uint8 GrowResult = CalculateProp(GrowCurrent, GrowTarget, GerminationPerMinute, TimePassed);
		ItemData.SetProp(EItemIntProperty::Grow, GrowResult);
	}

	return true;
}
