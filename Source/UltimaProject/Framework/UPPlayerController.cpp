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

	// Initialize Gameplay HUD
	if (ensureAlways(IsValid(GameplayHUDWidgetClass))
		&& IsLocalController())
	{
		GameplayHUDWidgetInstance = CreateWidget<UGameplayHUDWidget>(this, GameplayHUDWidgetClass);
		check(GameplayHUDWidgetInstance);

		GameplayHUDWidgetInstance->AddToViewport();
	}
}

// TODO this one shouldn't be there
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
	const IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(GetPlayerState<APlayerState>());

	if (!ControlledPawn || !ASI)
	{
		return;
	}

	UUPAbilitySystemComponent* ASC = Cast<UUPAbilitySystemComponent>(ASI->GetAbilitySystemComponent());
	UInventoryComponent* InventoryComponent = ControlledPawn->FindComponentByClass<UInventoryComponent>();
	if (!InventoryComponent || !ASC)
	{
		return;
	}

	FGameplayAbilitySpec* PickupAbilitySpec = ASC->FindAbilityByTag(ASC->GetPickupAbilityTag());
	if (!PickupAbilitySpec || PickupAbilitySpec->IsActive())
	{
		return;
	}

	UGameplayAbility_Interaction* PickupAbilityInstance = Cast<UGameplayAbility_Interaction>(
		PickupAbilitySpec->GetPrimaryInstance());
	if (!ensureAlways(PickupAbilityInstance))
	{
		return;
	}

	// TODO an extra server check might be needed when we ensure the player actually run a interaction ability before to call UInventoryComponent::TryPickupItem
	ASC->TryActivateAbility(PickupAbilitySpec->Handle, true);
}
