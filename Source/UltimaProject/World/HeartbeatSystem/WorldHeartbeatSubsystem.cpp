// Fill out your copyright notice in the Description page of Project Settings.

#include "WorldHeartbeatSubsystem.h"
#include "UltimaProject/Common/Macro.h"

DEFINE_LOG_CATEGORY(LogHeartbeat);

int64 UWorldHeartbeatSubsystem::GetHeartbeatTimestamp() const
{
	return FDateTime::UtcNow().ToUnixTimestamp();
}

void UWorldHeartbeatSubsystem::AddHeart(UObject* Heart)
{
	if (Heart && Heart->Implements<UHeartbeatInterface>())
	{
		Hearts.Add(Heart, GetHeartbeatTimestamp());
	}
}

void UWorldHeartbeatSubsystem::RemoveHeart(UObject* Heart)
{
	Hearts.Remove(Heart);
}

void UWorldHeartbeatSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UWorld* World = GetWorld();
	NULLCHECK(World);

	LastHeartbeatTimestamp = FDateTime::UtcNow().ToUnixTimestamp();

	World->GetTimerManager().SetTimer(HeartbeatTimerHandle, [this]
	{
		AsyncTask(ENamedThreads::GameThread, [this]
		{
			Heartbeat();
		});
	}, HeartbeatInterval, true);
}

void UWorldHeartbeatSubsystem::Deinitialize()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(HeartbeatTimerHandle);
	}

	Super::Deinitialize();
}

void UWorldHeartbeatSubsystem::Heartbeat()
{
	int64 CurrentTimestamp = FDateTime::UtcNow().ToUnixTimestamp();
	for (auto It = Hearts.CreateIterator(); It; ++It)
	{
		const int32 TimePassed = CurrentTimestamp - It->Value;

		if (!It->Key.IsValid())
		{
			It.RemoveCurrent();
			continue;
		}

		IHeartbeatInterface::Execute_Heartbeat(Cast<UObject>(It->Key.Get()), CurrentTimestamp, TimePassed);
		It->Value = CurrentTimestamp;
	}
}
