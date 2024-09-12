// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Navigation/PathFollowingComponent.h"
#include "UObject/Object.h"
#include "UPPathFollowingComponent.generated.h"

/**
 * 
 */
UCLASS()
class ULTIMAPROJECT_API UUPPathFollowingComponent : public UPathFollowingComponent
{
	GENERATED_BODY()

	FVector PathStartedAt;
	FVector PathFinishedAt;

	virtual FAIRequestID RequestMove(const FAIMoveRequest& RequestData, FNavPathSharedPtr InPath) override;
	virtual void OnPathFinished(const FPathFollowingResult& Result) override;

	void OnMovementCompleted() const;
};
