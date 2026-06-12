#pragma once

#include "UltimaProject/Items/Containers/ContainerComponent.h"
#include "InventoryComponent.generated.h"

UCLASS(Blueprintable)
class ULTIMAPROJECT_API UInventoryComponent : public UContainerComponent
{
	GENERATED_BODY()

protected:
	TWeakObjectPtr<APawn> OwnerCharacter;

public:
	UInventoryComponent();

	virtual void BeginPlay() override;
};
