#pragma once

#include "UltimaProject/Items/Common/Item.h"
#include "ContainerInterface.generated.h"

// Interface for actors that represents a container or has some container logic ( chests, shelfs )
UINTERFACE(Blueprintable)
class UContainerInterface : public UInterface
{
	GENERATED_BODY()
};

class IContainerInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	UContainerComponent* GetContainerComponent() const;
	
	virtual bool CanBeOpened(const class AUPPlayerController* Controller);
	
	// TODO container gets inaccessible delegate 
};
