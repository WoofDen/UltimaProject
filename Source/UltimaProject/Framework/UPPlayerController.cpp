// Fill out your copyright notice in the Description page of Project Settings.


// Game includes
#include "UPPlayerController.h"
#include "UPPlayerState.h"
#include "UltimaProject/Characters/UPCharacter.h"
#include "UltimaProject/Common/GameplayTags.h"
#include "UltimaProject/Common/InputHelpersFunctionLibrary.h"
#include "UltimaProject/Common/Macro.h"
#include "UltimaProject/GAS/Abilities/Interactions/GameplayAbility_Drop.h"
#include "UltimaProject/GAS/Abilities/Interactions/GameplayAbility_Pickup.h"
#include "UltimaProject/GAS/Abilities/Interactions/GameplayAbility_Relocate.h"
#include "UltimaProject/Items/Containers/ContainerComponent.h"
#include "UltimaProject/Items/Containers/Components/ExternalContainerComponent.h"
#include "UltimaProject/Items/Containers/Components/ProxyContainerComponent.h"
#include "UltimaProject/Items/Containers/Interfaces/ContainerOwnerInterface.h"

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

bool AUPPlayerController::IsContainerOpened(const IContainerOwnerInterface* ContainerInterface) const
{
	return OpenedContainers.Contains(ContainerInterface);
}

void AUPPlayerController::TryOpenContainer(IContainerOwnerInterface* ContainerInterface,
                                           EContainerRelationType Relation)
{
	NULLCHECK(ContainerInterface);
	check(!HasAuthority()); // Client only

	// Already opened
	if (IsContainerOpened(ContainerInterface))
	{
		return;
	}

	if (!ContainerInterface->CanBeOpened(this))
	{
		return;
	}

	UContainerComponent* ContainerComponent = IContainerOwnerInterface::Execute_GetContainerComponent(
		ContainerInterface->_getUObject());
	NULLCHECK(ContainerComponent);
	ensureAlways(!ContainerComponent->IsA<UProxyContainerComponent>()); // Shouldn't ever occur :E

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

void AUPPlayerController::TryCloseContainer(IContainerOwnerInterface* ContainerInterface)
{
	NULLCHECK(ContainerInterface);
	check(!HasAuthority()); // Client only

	if (!IsContainerOpened(ContainerInterface))
	{
		return;
	}

	OpenedContainers.Remove(ContainerInterface);

	UContainerComponent* ContainerComponent = IContainerOwnerInterface::Execute_GetContainerComponent(
		ContainerInterface->_getUObject());
	NULLCHECK(ContainerComponent);

	if (GameplayHUDWidgetInstance)
	{
		GameplayHUDWidgetInstance->CloseContainerWidget(ContainerComponent);
	}

	if (ContainerComponent->IsA<UExternalContainerComponent>())
	{
		ServerCloseProxyContainer(ContainerInterface->_getUObject());
	}
}

void AUPPlayerController::OnOpenedContainerAccessibilityUpdated(IContainerOwnerInterface* ContainerInterface)
{
	NULLCHECK(ContainerInterface);
	check(HasAuthority()); // Server only

	if (ContainerInterface->CanBeOpened(this))
	{
		// Container is still accessible
		return;
	}

	UObject* ContainerInterfaceObject = ContainerInterface->_getUObject();

	// Container is no longer accessible
	OpenedContainers.Remove(ContainerInterface);

	UContainerComponent* ContainerComponent = IContainerOwnerInterface::Execute_GetContainerComponent(
		ContainerInterfaceObject);

	// External containers are accessible only through proxy containers created per-client runtime
	if (ContainerComponent->IsA<UExternalContainerComponent>())
	{
		UProxyContainerComponent* ProxyContainerComponent = nullptr;
		OpenedProxyContainers.RemoveAndCopyValue(ContainerComponent, ProxyContainerComponent);

		ProxyContainerComponent->DestroyComponent();
	}

	ClientForceCloseContainer(ContainerInterfaceObject);
}

void AUPPlayerController::ClientForceCloseContainer_Implementation(UObject* ContainerInterfaceObject)
{
	IContainerOwnerInterface* ContainerInterface = Cast<IContainerOwnerInterface>(ContainerInterfaceObject);
	NULLCHECK(ContainerInterface);

	UContainerComponent* ContainerComponent = IContainerOwnerInterface::Execute_GetContainerComponent(
		ContainerInterface->_getUObject());
	NULLCHECK(ContainerComponent);

	if (GameplayHUDWidgetInstance)
	{
		GameplayHUDWidgetInstance->CloseContainerWidget(ContainerComponent);
	}
}

void AUPPlayerController::ServerCloseProxyContainer_Implementation(UObject* ContainerInterfaceObject)
{
	OpenedContainers.Remove(ContainerInterfaceObject);

	UContainerComponent* ContainerComponent = IContainerOwnerInterface::Execute_GetContainerComponent(
		ContainerInterfaceObject);
	ensureAlways(ContainerComponent->IsA<UExternalContainerComponent>());

	UProxyContainerComponent* ProxyContainerComponent = nullptr;
	OpenedProxyContainers.RemoveAndCopyValue(ContainerComponent, ProxyContainerComponent);
	if (ProxyContainerComponent)
	{
		ProxyContainerComponent->DestroyComponent();
	}
}

void AUPPlayerController::ServerOpenProxyContainer_Implementation(UObject* ContainerInterfaceObject)
{
	UProxyContainerComponent* ProxyContainer = NewObject<UProxyContainerComponent>(this);
	UContainerComponent* ContainerComponent = IContainerOwnerInterface::Execute_GetContainerComponent(
		ContainerInterfaceObject);

	ProxyContainer->InitializeServer(this, ContainerComponent);
	ProxyContainer->RegisterComponent();

	OpenedContainers.Add(ContainerInterfaceObject);
	OpenedProxyContainers.Add(ContainerComponent, ProxyContainer);
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

void AUPPlayerController::HandleDropAction(UContainerComponent* SourceContainer, int32 ContainerItemHandle,
                                           int32 ItemAmount) const
{
	UUPAbilitySystemComponent* ASC = GetAbilitySystemComponent();

	NULLCHECK(SourceContainer);
	NULLCHECK(ASC);
	ensureAlways(IsValid(ASC));

	FGameplayAbilityTargetData_DropOperation* DropDataPtr = new FGameplayAbilityTargetData_DropOperation();
	DropDataPtr->ItemAmount = ItemAmount;
	DropDataPtr->SourceContainer = SourceContainer->GetOriginContainer();
	DropDataPtr->ContainerItemHandle = ContainerItemHandle;

	FGameplayEventData EventData;
	EventData.Instigator = this;
	EventData.TargetData = FGameplayAbilityTargetDataHandle(DropDataPtr);

	// All validation will be provided by ability itself
	ASC->HandleGameplayEvent(TAG_Ability_Container_Drop, &EventData);
}

void AUPPlayerController::HandlePickupAction(AItem* SourceItem, int32 ItemAmount,
                                             UContainerComponent* TargetContainer) const
{
	NULLCHECK(SourceItem);
	NULLCHECK(TargetContainer);

	UUPAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	NULLCHECK(ASC);

	FGameplayAbilityTargetData_PickupOperation* PickupDataPtr = new FGameplayAbilityTargetData_PickupOperation();
	PickupDataPtr->ItemAmount = ItemAmount;
	PickupDataPtr->SourceItem = SourceItem;
	PickupDataPtr->TargetContainer = TargetContainer->GetOriginContainer();

	FGameplayEventData EventData;
	EventData.Instigator = this;
	EventData.TargetData = FGameplayAbilityTargetDataHandle(PickupDataPtr);

	ASC->HandleGameplayEvent(TAG_Ability_Container_Pickup.GetTag(), &EventData);
}

void AUPPlayerController::HandleActivateAction()
{
	AActor* CursorItem = UInputHelpersFunctionLibrary::GetActorUnderCursor(this);
	NULLCHECK(CursorItem);
	NULLCHECK_LOG(GameplayHUDWidgetInstance, Error, "PC Invalid HUD value");

	IContainerOwnerInterface* CursorContainer = Cast<IContainerOwnerInterface>(CursorItem);
	NULLCHECK(CursorContainer);

	if (IsContainerOpened(CursorContainer))
	{
		TryCloseContainer(CursorContainer);
	}
	else
	{
		TryOpenContainer(CursorContainer, EContainerRelationType::InWorldContainer);
	}
}

void AUPPlayerController::HandleInventoryToggle()
{
	IContainerOwnerInterface* InventoryInterface = Cast<IContainerOwnerInterface>(GetPawn());
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

void AUPPlayerController::HandleRelocateItem(UContainerComponent* SourceContainer, int32 ContainerItemHandle,
                                             UContainerComponent* TargetContainer, int32 ItemAmount)
{
	NULLCHECK(SourceContainer);
	NULLCHECK(TargetContainer);

	UUPAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	NULLCHECK(ASC);

	FGameplayAbilityTargetData_RelocateOperation* SourceDataPtr = new FGameplayAbilityTargetData_RelocateOperation();
	SourceDataPtr->ItemAmount = ItemAmount;
	SourceDataPtr->ItemHandle = ContainerItemHandle;
	SourceDataPtr->SourceContainer = SourceContainer;
	SourceDataPtr->TargetContainer = TargetContainer->GetOriginContainer();

	FGameplayEventData EventData;
	EventData.Instigator = this;
	EventData.TargetData = FGameplayAbilityTargetDataHandle(SourceDataPtr);

	ASC->HandleGameplayEvent(TAG_Ability_Container_Relocate.GetTag(), &EventData);
}

UUPAbilitySystemComponent* AUPPlayerController::GetAbilitySystemComponent() const
{
	// TODO could be cached
	APawn* ControlledPawn = GetPawn();
	const IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(GetPlayerState<APlayerState>());

	NULLCHECK_RETURN(ControlledPawn, nullptr);
	NULLCHECK_RETURN(ASI, nullptr);

	UUPAbilitySystemComponent* ASC = Cast<UUPAbilitySystemComponent>(ASI->GetAbilitySystemComponent());
	NULLCHECK_RETURN(ASC, nullptr);

	return Cast<UUPAbilitySystemComponent>(ASC);
}
