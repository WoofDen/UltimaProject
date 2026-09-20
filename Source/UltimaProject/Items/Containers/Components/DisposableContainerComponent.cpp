// Fill out your copyright notice in the Description page of Project Settings.

#include "DisposableContainerComponent.h"

#include "Net/UnrealNetwork.h"
#include "UltimaProject/Common/Macro.h"
#include "UltimaProject/Framework/UPPlayerController.h"
#include "UltimaProject/UI/ContainerWidget.h"

void UDisposableContainerComponent::OnLifetimeExpired()
{
	// Server & client
	DisposeContainer();
}

void UDisposableContainerComponent::DisposeContainer()
{
	// Server & client

	// Client: close container UI
	if (APawn* PawnOwner = GetOwner<APawn>(); PawnOwner->GetNetMode() == NM_Client)
	{
		AUPPlayerController* PC = PawnOwner->GetController<AUPPlayerController>();
		NULLCHECK(PC);

		IContainerOwnerInterface* ContainerOwnerInterface = GetOwnerInterface();
		NULLCHECK(ContainerOwnerInterface);

		PC->TryCloseContainer(this);
	}

	DestroyComponent();
}

UDisposableContainerComponent::UDisposableContainerComponent()
{
	SetIsReplicatedByDefault(false);
}

void UDisposableContainerComponent::BeginPlay()
{
	Super::BeginPlay();

	// Supposed to be attached to the player character itself
	ensureAlways(GetOwner() && GetOwner()->IsA<APawn>() && GetOwner()->GetLocalRole() != ROLE_SimulatedProxy);

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(LifetimeTimerHandle,
		                                  FTimerDelegate::CreateUObject(this, &ThisClass::OnLifetimeExpired),
		                                  MaxLifetime, false);
	}

	if (AActor* PawnOwner = GetOwner<APawn>(); PawnOwner && PawnOwner->HasAuthority())
	{
		ContainerOrigin = PawnOwner->GetActorLocation();
	}
}

void UDisposableContainerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(LifetimeTimerHandle);
	}

	Super::EndPlay(EndPlayReason);
}

TArray<FContainerItemData> UDisposableContainerComponent::GetItemsForDisplay(AController* InstigatorController)
{
	if (!InstigatorController || InstigatorController->GetPawn() != GetOwner())
	{
		UE_LOG(LogUPContainers, Error, TEXT("Unexpected request"));
		return {};
	}

	return Super::GetItemsForDisplay(InstigatorController);
}

void UDisposableContainerComponent::OnClientContainerClosed(AUPPlayerController* Instigator)
{
	Super::OnClientContainerClosed(Instigator);

	DisposeContainer();
}

void UDisposableContainerComponent::OnClientReady()
{
	Super::OnClientReady();

	/*
	 * Dynamic components seems doesn't send the replicated properties with initial bunch. 
	 * Thus WidgetClass won't be available in BeginPlay
	 * Wait for the property to be updated on Client and open the container
	 * TODO how to handle a possible package loss? no widget class no container
	 */

	if (APawn* PawnOwner = GetOwner<APawn>(); PawnOwner && PawnOwner->GetNetMode() == NM_Client)
	{
		ContainerOrigin = PawnOwner->GetActorLocation();

		if (AUPPlayerController* PC = PawnOwner->GetController<AUPPlayerController>(); PC)
		{
			PC->TryOpenContainer(this, EContainerRelationType::Disposable);
		}
	}
}

bool UDisposableContainerComponent::CanStoreItem(const AController* Instigator, const AItem* Item) const
{
	return false;
}

FVector UDisposableContainerComponent::GetContainerOrigin() const
{
	return ContainerOrigin;
}

UDisposableContainerComponent* UDisposableContainerComponent::CreateDisposableContainer(
	APlayerController* OwnerController,
	FString Name,
	EContainerCategory ContainerCategory,
	TArray<FItemDataDefinition> Items)
{
	NULLCHECK_RETURN(OwnerController, nullptr);

	// Server only
	if (!OwnerController->HasAuthority())
	{
		return nullptr;
	}

	APawn* OwnerPawn = OwnerController->GetPawn();
	NULLCHECK_RETURN(OwnerController, nullptr);

	EObjectFlags ContainerFlags = RF_Transient;
	UDisposableContainerComponent* ContainerComponent = NewObject<UDisposableContainerComponent>(OwnerPawn, UDisposableContainerComponent::StaticClass(), FName(Name), ContainerFlags);
	NULLCHECK_RETURN(ContainerComponent, nullptr);

	ContainerComponent->SetCategory(ContainerCategory);
	ContainerComponent->Rename(*Name);

	// Populate Items
	uint32 Capacity = 0;
	for (const auto& ItemDefinition : Items)
	{
		// Increase capacity
		Capacity += ItemDefinition.GetStaticData()->Slots;
		ContainerComponent->ItemSlotsCapacity = Capacity;

		// Spawn the item
		uint32 ResultHandle = ContainerComponent->SpawnItem(ItemDefinition);
		ensureAlways(ResultHandle != FContainerItemData::InvalidHandle);
	}

	// Ready to replicate
	ContainerComponent->RegisterComponent();
	ContainerComponent->SetIsReplicated(true);

	return ContainerComponent;
}
