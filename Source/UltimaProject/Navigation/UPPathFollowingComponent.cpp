// Fill out your copyright notice in the Description page of Project Settings.

// Game includes
#include "UPPathFollowingComponent.h"
#include "UltimaProject/Characters/UPCharacter.h"
#include "UltimaProject/Framework/UPPlayerState.h"

FAIRequestID UUPPathFollowingComponent::RequestMove(const FAIMoveRequest& RequestData, FNavPathSharedPtr InPath)
{
	FAIRequestID Result = Super::RequestMove(RequestData, InPath);

	if (InPath.IsValid() && InPath->GetPathPoints().Num() > 0)
	{
		PathStartedAt = InPath->GetPathPoints()[0].Location;
	}

	return Result;
}

void UUPPathFollowingComponent::OnPathFinished(const FPathFollowingResult& Result)
{
	PathFinishedAt = GetCurrentNavLocation();
	OnMovementCompleted();

	// will reset state
	Super::OnPathFinished(Result);
}

void UUPPathFollowingComponent::OnMovementCompleted() const
{
	const AController* Controller = CastChecked<AController>(GetOwner());
	const AUPPlayerState* PS = Controller->GetPlayerState<AUPPlayerState>();

	const float Length = FMath::Max(0, (PathFinishedAt - PathStartedAt).Length());

	PS->GetSkillSystemComponent()->AddSkillExp(ESkill::Walking, Length / 1000);
}
