// Fill out your copyright notice in the Description page of Project Settings.


#include "UPGameMode.h"

#include "EngineUtils.h"
#include "UPPlayerController.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "NavMesh/RecastNavMesh.h"
#include "UltimaProject/Characters/UPCharacter.h"

AUPGameMode::AUPGameMode()
{
	PlayerControllerClass = AUPPlayerController::StaticClass();
	DefaultPawnClass = AUPCharacter::StaticClass();
}

void AUPGameMode::BeginPlay()
{
	Super::BeginPlay();

	/*
	if (GetLocalRole() == ROLE_Authority)
	{
		const ACharacter* DefaultCharacter = Cast<ACharacter>(DefaultPawnClass->GetDefaultObject());
		const UCharacterMovementComponent* DefaultMovement = Cast<UCharacterMovementComponent>(DefaultCharacter->GetMovementComponent());

		const float MovementSlope = DefaultMovement->GetWalkableFloorAngle();
		const float StepHeight = DefaultMovement->MaxStepHeight;
		const float Radius = DefaultCharacter->GetCapsuleComponent()->GetScaledCapsuleRadius();
		const float Height = DefaultCharacter->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() * 2;

		// ensure nav volumes has the same settings
		for( TActorIterator<ARecastNavMesh> It(GetWorld()); It; ++It)
		{
			// todo test runtime update
			ARecastNavMesh* Volume = *It;
			Volume->AgentMaxStepHeight = StepHeight;
			Volume->AgentMaxSlope = MovementSlope;
			Volume->AgentRadius = Radius;
			Volume->AgentHeight = Height;

			Volume->OnNavMeshUpdate.Broadcast();
		}
	}
	*/
}
