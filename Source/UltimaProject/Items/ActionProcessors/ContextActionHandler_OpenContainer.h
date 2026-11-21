// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ContextActionHandler.h"
#include "ContextActionHandler_OpenContainer.generated.h"

/**
 * 
 */
UCLASS()
class ULTIMAPROJECT_API UContextActionHandler_OpenContainer : public UContextActionHandler
{
	GENERATED_BODY()
	
public:
	//~UContextActionHandler
	virtual bool HandleAction(AUPPlayerController* PC, AItem* Item) override;
	virtual bool IsAvailable(AUPPlayerController* PC, AItem* Item) override;
	//~UContextActionHandler End
};
