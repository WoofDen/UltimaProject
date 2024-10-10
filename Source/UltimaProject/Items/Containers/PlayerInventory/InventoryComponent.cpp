#include "InventoryComponent.h"

#include "UltimaProject/Framework/UPPlayerState.h"

void UInventoryComponent::ServerTryPickupItem_Implementation(AItem* Item)
{
	if (!IsValid(Item) || !CanPickItem(Item))
	{
		return;
	}

	if (AddItem(Item->GetItemData()))
	{
		// Reference on UItemData will be stored in the container now
		// So it's safe to destroy actor from the world now
		Item->RemoveFromWorld();
	}
}

UInventoryComponent::UInventoryComponent()
{
	if (!HasAnyFlags(RF_ClassDefaultObject))
	{
		OwnerCharacter = Cast<AUPCharacter>(GetOwner());
		check(OwnerCharacter.IsValid());
	}
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
