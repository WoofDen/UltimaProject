// Fill out your copyright notice in the Description page of Project Settings.


#include "UPPlayerController.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "GameFramework/Character.h"
#include "InputAction.h"
#include "UltimaProject/Characters/UPCharacter.h"

#define DBGPRINT(x) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green, TEXT(x));
#define DBGSPHERE(l, c) DrawDebugSphere(GetWorld(), l, 10.f, 6, c, false, 3.f);

AUPPlayerController::AUPPlayerController()
{
	SetShowMouseCursor(true);
	PathFollowingComponent = CreateDefaultSubobject<UUPPathFollowingComponent>("PathFollowingComponent");
}

void AUPPlayerController::BeginPlay()
{
	Super::BeginPlay();
}

void AUPPlayerController::MoveToCursor(const FInputActionInstance& Instance)
{
	FHitResult HitResult;

	// copy of GetHitResultUnderCursor
	ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(Player);
	bool bHit = false;
	if (LocalPlayer && LocalPlayer->ViewportClient)
	{
		FVector2D MousePosition;
		if (LocalPlayer->ViewportClient->GetMousePosition(MousePosition))
		{
			bHit = GetHitResultAtScreenPosition(MousePosition, ECC_WorldStatic, false, HitResult);
		}
	}

	// 
	if (!bHit) 
	{
		return;
	}

	// Seek for nav bounds
	DBGSPHERE(HitResult.Location, FColor::Red);

	// Will create path following component for PC
	UAIBlueprintHelperLibrary::SimpleMoveToLocation(
		GetCharacter()->GetController(),
		HitResult.Location
	);
}
