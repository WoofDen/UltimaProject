// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "UltimaProject/Items/Common/Item.h"
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

public:
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AItem> ItemBaseClass;
};
