// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "HeartbeatInterface.h"
#include "Subsystems/WorldSubsystem.h"
#include "WorldHeartbeatSubsystem.generated.h"

/**
 * Async. run logic for tick-based objects ( like growing crops )
 */

DECLARE_LOG_CATEGORY_EXTERN(LogHeartbeat, Log, All);

UCLASS(BlueprintType)
class ULTIMAPROJECT_API UWorldHeartbeatSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

	FTimerHandle HeartbeatTimerHandle;

	// TODO need to save it somewhere
	int64 LastHeartbeatTimestamp;

	void Heartbeat();

#if !UE_BUILD_SHIPPING
	float HeartbeatInterval = 1.f;
#else
	float HeartbeatInterval = 60.f;
#endif

	TMap<TWeakObjectPtr<UObject>, int64> Hearts;

	inline int64 GetHeartbeatTimestamp() const;

public:
	UFUNCTION(BlueprintCallable)
	void AddHeart(UObject* Heart);

	UFUNCTION(BlueprintCallable)
	void RemoveHeart(UObject* Heart);

	// USubsystem
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	// ~USubsystem
};
