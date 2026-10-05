#pragma once

#include "UltimaProject/Items/Common/Item.h"
#include "ContainerOwnerInterface.generated.h"

struct FContainerItemData;
// Interface for actors that represents a container or has some container logic ( chests, shelfs )
UINTERFACE(Blueprintable, BlueprintType)
class UContainerOwnerInterface : public UInterface
{
	GENERATED_BODY()
};

class IContainerOwnerInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	UContainerComponent* GetMainContainerComponent() const;

	// Server only ( could be called predictively on the client in future )
	AActor* GetOwningActor() const;
};
