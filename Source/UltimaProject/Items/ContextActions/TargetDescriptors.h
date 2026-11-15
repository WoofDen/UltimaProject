// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameplayTagContainer.h"
#include "GameFramework/Actor.h"
#include "TargetDescriptors.generated.h"

USTRUCT()
struct FTargetDescriptor
{
	GENERATED_BODY()
	
	FTargetDescriptor() = default;
	virtual ~FTargetDescriptor();

	UPROPERTY(EditAnywhere)
	FText StartedMessage;

	UPROPERTY(EditAnywhere)
	FText CompletedMessage;

	UPROPERTY(EditAnywhere)
	FText CancelledMessage;
	
	UPROPERTY(EditAnywhere)
	FGameplayTag AbilityTriggerTag;
};

USTRUCT()
struct FTargetDescriptor_Actor : public FTargetDescriptor
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	TArray<TSubclassOf<AActor>> TargetClasses;
};
