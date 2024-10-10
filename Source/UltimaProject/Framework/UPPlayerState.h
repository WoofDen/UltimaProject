// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "UPPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class ULTIMAPROJECT_API AUPPlayerState : public APlayerState
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, meta=(ClampMin=0))
	float PickupRadius;
	
public:
	FORCEINLINE float GetPickupRadius() const
	{
		return PickupRadius;
	}
};
