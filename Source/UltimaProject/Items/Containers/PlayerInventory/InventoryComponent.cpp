#include "InventoryComponent.h"
#include "UltimaProject/Characters/UPCharacter.h"
#include "UltimaProject/Framework/UPPlayerState.h"

void UInventoryComponent::ServerTryPickupItem_Implementation(AItem* Item)
{
	// Third pickup check - server check
	if (!IsValid(Item) || !CanStoreItem(Item))
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
	SplitItem(const_cast<FContainerItemData&>(Item), SplitAmount);
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

bool UInventoryComponent::CanStoreItem(const AItem* Item) const
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
	// Expect this to be called from a local client
	ensure(!GetOwner()->HasAuthority());

	// Send the request to the server
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
