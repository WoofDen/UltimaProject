// Fill out your copyright notice in the Description page of Project Settings.

#include "CropBase.h"

#include "Net/UnrealNetwork.h"

ACropBase::ACropBase()
{
	PrimaryActorTick.bCanEverTick = false;
	NetPriority = .1f;

	// Grass, who cares
	SetNetUpdateFrequency(1.f);
	bReplicates = true;
}

void ACropBase::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		IHeartbeatInterface::Register();
	}

	CurrentCycleTime = 0.f;
}

void ACropBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (HasAuthority())
	{
		IHeartbeatInterface::Unregister();
	}

	Super::EndPlay(EndPlayReason);
}

void ACropBase::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ACropBase, GrowProgress);
}

void ACropBase::Heartbeat_Implementation(int64 CurrentTime, int32 TimePassed)
{
	IHeartbeatInterface::Heartbeat_Implementation(CurrentTime, TimePassed);

	CurrentCycleTime += TimePassed;
	SetProgress(static_cast<float>(CurrentCycleTime) / GrowTime);
}

void ACropBase::OnRep_GrowProgress()
{
	ensureAlways(true);
	OnProgressUpdated(GrowProgress);
}

void ACropBase::SetProgress(float NewProgress)
{
	ensureAlways(HasAuthority());
	GrowProgress = FMath::Clamp(NewProgress, 0.f, 1.f);

	OnProgressUpdated(GrowProgress);
}
