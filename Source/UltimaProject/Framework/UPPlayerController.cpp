// Fill out your copyright notice in the Description page of Project Settings.


#include "UPPlayerController.h"

#include "UPPlayerState.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "GameFramework/Character.h"
#include "UltimaProject/Characters/UPCharacter.h"
#include "UltimaProject/Items/Containers/PlayerInventory/InventoryComponent.h"

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

void AUPPlayerController::MoveToCursor()
{
	APawn* ControlledPawn = GetPawn();
	if (!ControlledPawn)
	{
		return;
	}

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

	// DBGSPHERE(HitResult.Location, FColor::Red);

	FVector Direction = (HitResult.Location - ControlledPawn->GetActorLocation()).GetSafeNormal();

	ensure(!HasAuthority());
	ControlledPawn->AddMovementInput(Direction); // won't work on server :o

	/*
	// has issues with rotation replication 
	UAIBlueprintHelperLibrary::SimpleMoveToLocation(
		this,
		Location
	);
	*/
}

void AUPPlayerController::HandlePickupAction() const
{
	APawn* ControlledPawn = GetPawn();
	if (!ControlledPawn)
	{
		return;
	}

	UInventoryComponent* InventoryComponent = ControlledPawn->FindComponentByClass<UInventoryComponent>();
	if (!InventoryComponent)
	{
		return;
	}

	// find actor under cursor
	ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(Player);
	FHitResult HitResult;
	if (LocalPlayer && LocalPlayer->ViewportClient)
	{
		FVector2D MousePosition;
		if (LocalPlayer->ViewportClient->GetMousePosition(MousePosition))
		{
			TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
			ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_WorldDynamic));

			GetHitResultAtScreenPosition(
				MousePosition,
				ObjectTypes,
				false,
				HitResult);
		}
	}

	if (!HitResult.bBlockingHit)
	{
		return;
	}

	AItem* Item = Cast<AItem>(HitResult.GetActor());

	if (!Item)
	{
		return;
	}

	InventoryComponent->TryPickupItem(Item);
}
