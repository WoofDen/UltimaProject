// Fill out your copyright notice in the Description page of Project Settings.


// Game includes
#include "UPPlayerController.h"
#include "UPPlayerState.h"
#include "UltimaProject/Characters/UPCharacter.h"
#include "UltimaProject/Common/InputHelpers.h"
#include "UltimaProject/Common/Macro.h"
#include "UltimaProject/Items/Containers/ContainerComponent.h"
#include "UltimaProject/Items/Containers/Components/ProxyContainerComponent.h"
#include "UltimaProject/Items/Containers/Interfaces/ContainerInterface.h"

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

bool AUPPlayerController::IsContainerOpened(IContainerInterface* ContainerInterface) const
{
	return OpenedContainers.Contains(ContainerInterface);
}

void AUPPlayerController::TryOpenContainer(IContainerInterface* ContainerInterface, EContainerRelationType Relation)
{
	NULLCHECK(ContainerInterface);

	// Already opened
	if (IsContainerOpened(ContainerInterface))
	{
		return;
	}

	if (!ContainerInterface->CanBeOpened(this))
	{
		return;
	}

	UContainerComponent* ContainerComponent = IContainerInterface::Execute_GetContainerComponent(
		ContainerInterface->_getUObject());
	NULLCHECK(ContainerComponent);

	switch (Relation)
	{
	case EContainerRelationType::Inventory:
		// Inventory is already replicated
		GameplayHUDWidgetInstance->AddContainerWidget(ContainerComponent);
		break;
	case EContainerRelationType::InWorldContainer:
		ServerOpenProxyContainer(ContainerInterface->_getUObject());
		break;
	case EContainerRelationType::Invalid:
		UE_LOG(LogController, Error, TEXT("Invalid container type"));
		return;
	}

	OpenedContainers.Add(ContainerInterface);
}

void AUPPlayerController::TryCloseContainer(IContainerInterface* ContainerInterface)
{
	NULLCHECK(ContainerInterface);
	if (!IsContainerOpened(ContainerInterface))
	{
		return;
	}

	if (GameplayHUDWidgetInstance)
	{
		UContainerComponent* ContainerComponent = IContainerInterface::Execute_GetContainerComponent(
			ContainerInterface->_getUObject());
		GameplayHUDWidgetInstance->CloseContainerWidget(ContainerComponent);
	}

	OpenedContainers.Remove(ContainerInterface);
}

void AUPPlayerController::ServerOpenProxyContainer_Implementation(UObject* ContainerInterfaceObject)
{
	UProxyContainerComponent* ProxyContainer = NewObject<UProxyContainerComponent>(this);
	UContainerComponent* ContainerComponent = IContainerInterface::Execute_GetContainerComponent(ContainerInterfaceObject);
	
	ProxyContainer->InitializeServer(this, ContainerComponent);
	ProxyContainer->RegisterComponent();
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

	IContainerInterface* InventoryInterface = Cast<IContainerInterface>(CursorItem);
	NULLCHECK(CursorItem);

	if (IsContainerOpened(InventoryInterface))
	{
		TryCloseContainer(InventoryInterface);
	}
	else
	{
		TryOpenContainer(InventoryInterface, EContainerRelationType::InWorldContainer);
	}
}

void AUPPlayerController::HandleInventoryToggle()
{
	IContainerInterface* InventoryInterface = Cast<IContainerInterface>(GetPawn());
	NULLCHECK(InventoryInterface);

	if (IsContainerOpened(InventoryInterface))
	{
		TryCloseContainer(InventoryInterface);
	}
	else
	{
		TryOpenContainer(InventoryInterface, EContainerRelationType::Inventory);
	}
}
