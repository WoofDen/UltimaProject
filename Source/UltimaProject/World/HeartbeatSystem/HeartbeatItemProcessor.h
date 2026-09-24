// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "HeartbeatProcessor.generated.h"

/**
 * 
 */
UCLASS(Blueprintable)
class ULTIMAPROJECT_API UHeartbeatProcessor : public UObject
{
	GENERATED_BODY()

public:
	void HeartbeatExternal();
};
