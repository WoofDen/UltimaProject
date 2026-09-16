// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "UltimaProject/World/HeartbeatSystem/HeartbeatInterface.h"
#include "CropBase.generated.h"

UCLASS(Abstract, Blueprintable, BlueprintType)
class ULTIMAPROJECT_API ACropBase : public AActor, public IHeartbeatInterface
{
	GENERATED_BODY()

	int64 CurrentCycleTime;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float GrowTime = 10.f;

	UPROPERTY(VisibleAnywhere, ReplicatedUsing=OnRep_GrowProgress)
	float GrowProgress = 0.f;
	
	UFUNCTION()
	void OnRep_GrowProgress();

	void SetProgress(float NewProgress);
	
	UFUNCTION(BlueprintImplementableEvent)
	void OnProgressUpdated(float Progress);

public:
	ACropBase();

	// ĄActor
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	// ~AActor
	
	// IHeartbeatInterface
	virtual void Heartbeat_Implementation(int64 CurrentTime, int32 TimePassed) override;
	// ~IHeartbeatInterface
};
