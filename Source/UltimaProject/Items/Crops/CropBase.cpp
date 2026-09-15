// Fill out your copyright notice in the Description page of Project Settings.

#include "CropBase.h"

ACropBase::ACropBase()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ACropBase::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		IHeartbeatInterface::Register();
	}
}

void ACropBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (HasAuthority())
	{
		IHeartbeatInterface::Unregister();
	}

	Super::EndPlay(EndPlayReason);
}
