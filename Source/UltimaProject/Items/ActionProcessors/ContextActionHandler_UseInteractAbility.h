// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ContextActionHandler.h"
#include "ContextActionHandler_UseInteractAbility.generated.h"

/**
 * 
 */
UCLASS()
class ULTIMAPROJECT_API UContextActionHandler_UseInteractAbility : public UContextActionHandler
{
	GENERATED_BODY()
	
protected:
	UPROPERTY(EditDefaultsOnly)
	FGameplayTag AbilityTag;
	
public:
	//~UContextActionHandler
	virtual bool HandleAction(AUPPlayerController* PC, AItem* Item) override;
	//~UContextActionHandler end
};
