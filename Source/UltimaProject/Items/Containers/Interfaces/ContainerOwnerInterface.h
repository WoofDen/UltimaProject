#pragma once

#include "UltimaProject/Items/Common/Item.h"
#include "ContainerOwnerInterface.generated.h"

struct FContainerItemData;
// Interface for actors that represents a container or has some container logic ( chests, shelfs )
UINTERFACE(Blueprintable)
class UContainerOwnerInterface : public UInterface
{
	GENERATED_BODY()
};

DECLARE_MULTICAST_DELEGATE_OneParam(FOnContainerAccessibilityUpdated, class IContainerOwnerInterface*);

class IContainerOwnerInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	UContainerComponent* GetMainContainerComponent() const;

	virtual bool CanBeOpened(const class AUPPlayerController* Controller) const;

	// Event called when the container actor ( a player, a chest, a shelf ) has died/destroyed/moved or altered any other variables that is important for container viewers
	// Server only ( could be called predictively on the client in future )
	virtual FOnContainerAccessibilityUpdated GetAccessibilityChangedDelegate() const;
	AActor* GetOwningActor() const;
};
