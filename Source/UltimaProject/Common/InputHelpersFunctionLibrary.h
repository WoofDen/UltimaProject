// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "InputHelpersFunctionLibrary.generated.h"

UCLASS()
class UInputHelpersFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	UFUNCTION(Blueprintable, BlueprintPure)
	static AActor* GetActorUnderCursor(const APlayerController* PC);
};
