// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "HeartbeatInterface.generated.h"

UINTERFACE(Blueprintable)
class UHeartbeatInterface : public UInterface
{
	GENERATED_BODY()
};

class ULTIMAPROJECT_API IHeartbeatInterface
{
	GENERATED_BODY()

public:
	UFUNCTION()
	virtual void Register();

	UFUNCTION()
	virtual void Unregister();

	UFUNCTION(BlueprintNativeEvent)
	void Heartbeat(int64 CurrentTime, int32 TimePassed);
};
