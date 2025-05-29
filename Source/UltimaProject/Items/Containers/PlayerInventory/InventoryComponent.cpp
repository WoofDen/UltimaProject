#include "InventoryComponent.h"
#include "Net/UnrealNetwork.h"
#include "UltimaProject/Characters/UPCharacter.h"
#include "UltimaProject/Framework/UPPlayerState.h"

void UInventoryComponent::ServerTryPickupItem_Implementation(AItem* Item)
{
	if (!IsValid(Item) || !CanPickItem(Item))
	{
		return;
	}

	MoveItem(Item);
}

void UInventoryComponent::ServerTryDropItem_Implementation(const FContainerItemData& Item)
{
	ensureAlways(Item.IsValid() && Item.IsInContainer(this));

	if (ensure(HasItem(Item)))
	{
		AItem* Result = nullptr;
		MoveItem(const_cast<FContainerItemData&>(Item), Result);
	}
}

void UInventoryComponent::ServerTrySplitItem_Implementation(const FContainerItemData& Item, const int64 SplitAmount)
{
	SplitItem(Item, SplitAmount);
}

bool UInventoryComponent::TrySplitItem(const FContainerItemData& Item, const int64 SplitAmount)
{
	if(!ensureAlways(HasItem(Item)))
	{
		return false;
	}

	const UItemData* ItemData = Item.GetItemData();
	if(!ItemData || SplitAmount < 0 || SplitAmount >= ItemData->GetAmount())
	{
		return false;
	}

	ServerTrySplitItem(Item, SplitAmount);
	return true;
}

void UInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(UInventoryComponent, Items, COND_OwnerOnly);
}

UInventoryComponent::UInventoryComponent()
{
}

void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();
	
	OwnerCharacter = Cast<AUPCharacter>(GetOwner());
	SetIsReplicated(true);
}

bool UInventoryComponent::CanPickItem(const AItem* Item) const
{
	if (!OwnerCharacter.IsValid())
	{
		return false;
	}

	const AUPPlayerState* PlayerState = OwnerCharacter->GetPlayerState<AUPPlayerState>();
	if (!PlayerState)
	{
		return false;
	}

	// radius check
	float Distance = (OwnerCharacter->GetActorLocation() - Item->GetActorLocation()).Length();
	if (Distance > PlayerState->GetPickupRadius())
	{
		return false;
	}

	// visibility check
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(OwnerCharacter.Get());
	Params.AddIgnoredActor(Item);

	FHitResult Result;
	// move trace a bit up to avoid ground collision
	const FVector TraceEnd = Item->GetActorLocation() + FVector(0, 0, 1.f);
	const bool bHasObstacle = GetWorld()->LineTraceSingleByChannel(
		Result,
		OwnerCharacter->GetActorLocation(),
		TraceEnd,
		ECC_Visibility,
		Params
	);
	if (bHasObstacle)
	{
		return false;
	}

	return true;
}

bool UInventoryComponent::TryPickupItem(AItem* Item)
{
	// expect this to be called from a local client
	ensure(!GetOwner()->HasAuthority());

	// local checks

	// send request to server
	ServerTryPickupItem(Item);
	return true;
}

bool UInventoryComponent::TryDropItem(const FContainerItemData& Item)
{
	ensureAlways(!GetOwner()->HasAuthority());
	ensureAlways(Item.IsValid() && Item.IsInContainer(this));

	if (ensureAlways(HasItem(Item)))
	{
		ServerTryDropItem(Item);
	}

	return true;
}
