// Fill out your copyright notice in the Description page of Project Settings.

#include "ProxyContainerComponent.h"

#include "Engine/ActorChannel.h"
#include "Net/UnrealNetwork.h"
#include "UltimaProject/Common/Macro.h"

void UProxyContainerComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	{
		FDoRepLifetimeParams Params;
		Params.bIsPushBased = true;

		// As proxies belong to PC, it replicates only to the owner already
		Params.Condition = COND_None;

		DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, ProxyContainerItems, Params);
	}

	DOREPLIFETIME_CONDITION(ThisClass, OriginContainer, COND_InitialOnly);
	DOREPLIFETIME_CONDITION(ThisClass, OwnerController, COND_InitialOnly);
}

void UProxyContainerComponent::OnRep_OriginContainer()
{
	InitializeClient();
}

void UProxyContainerComponent::OnRep_OwnerController()
{
	InitializeClient();
}

void UProxyContainerComponent::OnRep_ProxyContainerItems()
{
	NotifyContainerItemsChanged();
}

void UProxyContainerComponent::OnOriginContainerItemsChanged()
{
	NULLCHECK_SP(OriginContainer);

	// Both server & Client

	AActor* Owner = GetOwner();
	NULLCHECK(Owner);

	// Origin container items have been changed
	// Multiply players can simultaneously change the container so per-item update looks complicated for now
	// Update the whole proxy array and push it to clients
	if (Owner->HasAuthority())
	{
		ProxyContainerItems.Items = OriginContainer->ContainerItems.Items;
		ProxyContainerItems.MarkArrayDirty(); // Handles push model state

		// For client it takes time to get ProxyContainerItems's new value replicated
		// The update will be called by OnReo_ function
	}
}

UProxyContainerComponent::UProxyContainerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	// Replication will be enabled once the container is properly initialized
	SetIsReplicatedByDefault(false);
}

void UProxyContainerComponent::InitializeServer(AUPPlayerController* InOwner, UContainerComponent* InContainerComponent)
{
	if (bInitialized)
	{
		return;
	}

	// The component designed exclusively for PC
	check(GetOwner() == InOwner);
	check(GetOwner()->HasAuthority()); // Server only, all the essential data will be replicated

	VALIDCHECK_LOG(InOwner, Error, "Proxy container - invalid controller");
	VALIDCHECK_LOG(InContainerComponent, Error, "Proxy container - invalid controller");

	OwnerController = InOwner;
	OriginContainer = InContainerComponent;

	// Do a copy of items into proxy. Items are still referred to its original container and as items are UObjects, it doesn't inflict duplicate
	ProxyContainerItems.ContainerComponent = this;
	ProxyContainerItems.Items = OriginContainer->ContainerItems.Items;
	ProxyContainerItems.MarkArrayDirty();

	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, ProxyContainerItems, this);

	// Track origin container changes and reflect them to the proxy
	OriginContainer->OnContainerItemsChanged.AddDynamic(this, &ThisClass::OnOriginContainerItemsChanged);

	// Send to client
	SetIsReplicated(true);
	bInitialized = true;
}

void UProxyContainerComponent::InitializeClient()
{
	if (bInitialized)
	{
		return;
	}

	NULLCHECK_SP(OriginContainer);
	NULLCHECK_SP(OwnerController);

	UGameplayHUDWidget* HUD = OwnerController->GetGameplayHUD();
	NULLCHECK(HUD);

	// Show proxy container UI to the client
	HUD->AddContainerWidget(this);

	OriginContainer->OnContainerItemsChanged.AddDynamic(this, &ThisClass::OnOriginContainerItemsChanged);

	ProxyContainerItems.ContainerComponent = this;

	SetIsReplicated(true);
	bInitialized = true;
}

void UProxyContainerComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UProxyContainerComponent::BeginDestroy()
{
	Super::BeginDestroy();

	if (GetOwner() && GetOwner()->HasAuthority())
	{
		// Clear the update delegate on server
		if (OriginContainer.Get())
		{
			OriginContainer->OnContainerItemsChanged.RemoveDynamic(this, &ThisClass::OnOriginContainerItemsChanged);
		}
	}
}

bool UProxyContainerComponent::ReplicateSubobjects(class UActorChannel* Channel, class FOutBunch* Bunch,
                                                   FReplicationFlags* RepFlags)
{
	// Skip parent as it replicates the UContainerComponent::ContainerItems - proxy has always this value empty
	bool WroteSomething = UActorComponent::ReplicateSubobjects(Channel, Bunch, RepFlags);

	for (auto& Item : ProxyContainerItems.Items)
	{
		//WroteSomething |= Channel->ReplicateSubobject(Item.ItemData, *Bunch, *RepFlags);
	}

	return WroteSomething;
}

TArray<FContainerItemData> UProxyContainerComponent::GetItemsForDisplay(AController* InstigatorController)
{
	return ProxyContainerItems.Items;
}

UContainerComponent* UProxyContainerComponent::GetOriginContainer()
{
	return OriginContainer.Get();
}

TSubclassOf<UContainerWidget> UProxyContainerComponent::GetContainerWidgetClass() const
{
	NULLCHECK_SP_RETURN(OriginContainer, nullptr);
	return OriginContainer->GetContainerWidgetClass();
}

FItemTransactionResult UProxyContainerComponent::MoveItem(AItem* WorldItem)
{
	NULLCHECK_SP_RETURN(OriginContainer, GItemTransactionResult_Error);
	return OriginContainer->MoveItem(WorldItem);
}

FItemTransactionResult UProxyContainerComponent::MoveItem(FContainerItemData& SourceItem)
{
	NULLCHECK_SP_RETURN(OriginContainer, GItemTransactionResult_Error);
	return OriginContainer->MoveItem(SourceItem);
}

FItemTransactionResult UProxyContainerComponent::MoveItem(FContainerItemData& Item, AItem* OutItem)
{
	NULLCHECK_SP_RETURN(OriginContainer, GItemTransactionResult_Error);
	return OriginContainer->MoveItem(Item, OutItem);
}

FItemTransactionResult UProxyContainerComponent::SplitItem(FContainerItemData& Data, const int32 SplitAmount)
{
	NULLCHECK_SP_RETURN(OriginContainer, GItemTransactionResult_Error);
	return OriginContainer->SplitItem(Data, SplitAmount);
}

bool UProxyContainerComponent::HasItem(const FContainerItemData& ItemData) const
{
	NULLCHECK_SP_RETURN(OriginContainer, false);

	// Clients origin container doesn't replicate, thus its items will be empty
	// Rely on the proxy copy for client checks
	if (GetNetMode() == NM_Client)
	{
		return ProxyContainerItems.Items.Contains(ItemData);
	}

	return OriginContainer->HasItem(ItemData);
}
