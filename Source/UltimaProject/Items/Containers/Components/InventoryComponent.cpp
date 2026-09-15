#include "InventoryComponent.h"
#include "UltimaProject/Characters/UPCharacter.h"
#include "UltimaProject/Common/Macro.h"

UInventoryComponent::UInventoryComponent()
{
	Category = EContainerCategory::Inventory;
}

void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerCharacter = Cast<AUPCharacter>(GetOwner());

	// Own inventory should be always replicated to the player
	SetIsReplicated(true);
}

bool UInventoryComponent::IsAccessible(const AController* Instigator) const
{
	NULLCHECK_RETURN(Instigator, false);

	// Inventory can be accessed only by its owner
	if (GetOwner() != Instigator->GetPawn())
	{
		return false;
	}

	return Super::IsAccessible(Instigator);
}
