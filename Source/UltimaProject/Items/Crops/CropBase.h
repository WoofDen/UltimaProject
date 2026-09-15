// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "UltimaProject/World/HeartbeatSystem/HeartbeatInterface.h"
#include "CropBase.generated.h"

UCLASS(Abstract, Blueprintable, BlueprintType)
class ULTIMAPROJECT_API ACropBase : public AActor, public IHeartbeatInterface
{
	GENERATED_BODY()

public:
	ACropBase();
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float GrowTime = 10.f;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
};
