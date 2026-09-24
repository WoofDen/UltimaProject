// Fill out your copyright notice in the Description page of Project Settings.

#include "CropsHeartbeatItemProcessor.h"

bool UCropsHeartbeatItemProcessor::HeartbeatInternal(FItemData& ItemData, bool bWorldItem, int64 CurrentTime, int32 TimePassed)
{
	if (!Super::HeartbeatInternal(ItemData, bWorldItem, CurrentTime, TimePassed))
	{
		return false;
	}

	// Grow seeds if they are soaked
	float HumidityNormalized = ItemData.GetIntPropNormalized(EItemIntProperty::Humidity);
	float TemperatureNormalized = ItemData.GetIntPropNormalized(EItemIntProperty::Temperature);
	if (HumidityNormalized > .5f && TemperatureNormalized <= .7f)
	{
		uint8 GrowCurrent = ItemData.GetIntProp(EItemIntProperty::Progress);
		uint8 GrowTarget = UINT8_MAX;

		uint8 GrowResult = CalculateProp(GrowCurrent, GrowTarget, GerminationPerMinute, TimePassed);
		ItemData.SetProp(EItemIntProperty::Progress, GrowResult);
	}

	return true;
}
