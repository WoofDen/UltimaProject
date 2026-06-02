// Fill out your copyright notice in the Description page of Project Settings.


// Game includes
#include "UPPlayerController.h"
#include "UPPlayerState.h"
#include "UltimaProject/Characters/UPCharacter.h"
#include "UltimaProject/Common/InputHelpers.h"
#include "UltimaProject/Common/Macro.h"
#include "UltimaProject/Items/Containers/ContainerComponent.h"
#include "UltimaProject/Items/Containers/Interfaces/ContainerInterface.h"

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

void AUPPlayerController::TryOpenContainer(IContainerInterface* ContainerInterface)
{
	NULLCHECK(ContainerInterface);
	if (!ContainerInterface->CanBeOpened(this))
	{
		return;
	}
	
	UContainerComponent* ContainerComponent = IContainerInterface::Execute_GetContainerComponent(ContainerInterface->_getUObject());
	NULLCHECK(ContainerComponent);
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
	ControlledPawn->AddMovementInput(Direction); // won't work on a server :o

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

	NULLCHECK(ControlledPawn);
	NULLCHECK(ASI);

	UUPAbilitySystemComponent* ASC = Cast<UUPAbilitySystemComponent>(ASI->GetAbilitySystemComponent());
	NULLCHECK(ASC);

	FGameplayAbilitySpec* PickupAbilitySpec = ASC->FindAbilityByTag(ASC->GetPickupAbilityTag());

	// Prevent parallel pickups
	if (!PickupAbilitySpec || PickupAbilitySpec->IsActive())
	{
		return;
	}

	FGameplayEventData EventData;
	EventData.Instigator = this;
	EventData.Target = InputHelpers::GetActorUnderCursor(this);

	if (EventData.Target)
	{
		ASC->HandleGameplayEvent(ASC->GetPickupAbilityTag(), &EventData);
	}
}

void AUPPlayerController::HandleActivateAction()
{
	AActor* CursorItem = InputHelpers::GetActorUnderCursor(this);
	NULLCHECK(CursorItem);
	NULLCHECK_LOG(GameplayHUDWidgetInstance, Error, "PC Invalid HUD value");

	if (IContainerInterface* ContainerInterface = Cast<IContainerInterface>(CursorItem))
	{
		TryOpenContainer(ContainerInterface);
	}
}
