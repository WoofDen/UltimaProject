// Fill out your copyright notice in the Description page of Project Settings.

#include "ChestComponent.h"

UChestComponent::UChestComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UChestComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UChestComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

