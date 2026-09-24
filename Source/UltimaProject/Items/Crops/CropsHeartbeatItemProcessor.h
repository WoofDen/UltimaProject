// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UltimaProject/World/HeartbeatSystem/HeartbeatItemProcessor.h"
#include "CropsHeartbeatItemProcessor.generated.h"

/**
 * 
 */
UCLASS()
class ULTIMAPROJECT_API UCropsHeartbeatItemProcessor : public UHeartbeatItemProcessor
{
	GENERATED_BODY()

	float GerminationPerMinute = 50.f;

	// UHeartbeatItemProcessor
	virtual bool HeartbeatInternal(FItemData& ItemData, bool bWorldItem, int64 CurrentTime, int32 TimePassed) override;
	// ~UHeartbeatItemProcessor
};
