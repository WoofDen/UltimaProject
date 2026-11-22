// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "UltimaProject/World/HeartbeatSystem/HeartbeatItemProcessor.h"
#include "GrainHeartbeatItemProcessor.generated.h"

/**
 * 
 */
UCLASS(Blueprintable)
class ULTIMAPROJECT_API UGrainHeartbeatItemProcessor : public UHeartbeatItemProcessor
{
	GENERATED_BODY()

	float GerminationPerMinute = 50.f;

	// UHeartbeatItemProcessor
	virtual bool HeartbeatInternal_Implementation(FItemData& ItemData, const FItemDataLocation& ItemLocation, int64 CurrentTime, int32 TimePassed) override;
	// ~UHeartbeatItemProcessor
};
