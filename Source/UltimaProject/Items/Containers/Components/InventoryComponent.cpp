#include "InventoryComponent.h"
#include "UltimaProject/Characters/UPCharacter.h"

UInventoryComponent::UInventoryComponent()
{
}

void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerCharacter = Cast<AUPCharacter>(GetOwner());
	
	// Own inventory should be always replicated to the player
	SetIsReplicated(true);
}
