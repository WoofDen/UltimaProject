// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "UPGameMode.generated.h"

/**
 * 
 */
UCLASS()
class ULTIMAPROJECT_API AUPGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
	AUPGameMode();

	virtual void BeginPlay() override;
};
