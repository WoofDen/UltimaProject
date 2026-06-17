#pragma once

#include "UltimaProject/Items/Common/Item.h"
#include "ContainerInterface.generated.h"

// Interface for actors that represents a container or has some container logic ( chests, shelfs )
UINTERFACE(Blueprintable)
class UContainerInterface : public UInterface
{
	GENERATED_BODY()
};

DECLARE_MULTICAST_DELEGATE_OneParam(FOnContainerAccessibilityUpdated, class IContainerInterface*);

class IContainerInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	UContainerComponent* GetContainerComponent() const;

	virtual bool CanBeOpened(const class AUPPlayerController* Controller);

	// Event called when the container actor ( a player, a chest, a shelf ) has died/destroyed/moved or altered any other variables that is important for container viewers
	// Server only ( could be called predictively on the client in future )
	virtual FOnContainerAccessibilityUpdated GetAccessibilityChangedDelegate() const;
};
