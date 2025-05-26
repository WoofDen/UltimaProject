// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UltimaProject/Items/Containers/Container.h"
#include "ChestComponent.generated.h"

// Base for any in-world container
UCLASS(Blueprintable)
class ULTIMAPROJECT_API UChestComponent : public UContainer
{
	GENERATED_BODY()

public:
	UChestComponent();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;
};
