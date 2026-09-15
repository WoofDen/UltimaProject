// Fill out your copyright notice in the Description page of Project Settings.

#include "HeartbeatInterface.h"

#include "WorldHeartbeatSubsystem.h"
#include "UltimaProject/Common/Macro.h"

void IHeartbeatInterface::Register()
{
	UObject* This = Cast<UObject>(this);
	NULLCHECK(This);

	UWorld* World = This->GetWorld();
	NULLCHECK(World);

	if (UWorldHeartbeatSubsystem* Subsystem = World->GetSubsystem<UWorldHeartbeatSubsystem>())
	{
		Subsystem->AddHeart(This);
	}
}

void IHeartbeatInterface::Unregister()
{
	UObject* This = Cast<UObject>(this);
	NULLCHECK(This);

	UWorld* World = This->GetWorld();
	NULLCHECK(World);

	if (UWorldHeartbeatSubsystem* Subsystem = World->GetSubsystem<UWorldHeartbeatSubsystem>())
	{
		Subsystem->RemoveHeart(This);
	}
}
