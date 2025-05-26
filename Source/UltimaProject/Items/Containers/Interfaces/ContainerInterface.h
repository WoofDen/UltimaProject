#pragma once

#include "UltimaProject/Items/Common/Item.h"
#include "ContainerInterface.generated.h"

struct FContainerItemData;

// TODO maybe remove
UINTERFACE(MinimalAPI, Blueprintable)
class UContainerInterface : public UInterface
{
	GENERATED_BODY()
};

class IContainerInterface
{
	GENERATED_BODY()

public:
	virtual TArray<FContainerItemData> GetItems() = 0;
};
