#pragma once

#include "UltimaProject/Characters/UPCharacter.h"
#include "UltimaProject/Items/Containers/Container.h"
#include "InventoryComponent.generated.h"

UCLASS()
class ULTIMAPROJECT_API UInventoryComponent : public UContainer
{
	GENERATED_BODY()

	UFUNCTION(Server, Reliable)
	void ServerTryPickupItem(AItem* Item);

protected:
	TWeakObjectPtr<APawn> OwnerCharacter;

public:
	UInventoryComponent();

	bool CanPickItem(const AItem* Item) const;

	// Performs pickup action, considering all external conditions are met ( radius/visibility/etc )
	bool TryPickupItem(AItem* Item);
};
