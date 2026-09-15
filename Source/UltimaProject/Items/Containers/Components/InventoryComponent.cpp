#include "InventoryComponent.h"
#include "UltimaProject/Characters/UPCharacter.h"

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
