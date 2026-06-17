// Game includes
#include "ContainerInterface.h"
#include "UltimaProject/Items/Containers/ContainerComponent.h"

bool IContainerInterface::CanBeOpened(const class AUPPlayerController* Controller)
{
	return IsValid(Controller) && IsValid(GetContainerComponent());
}

FOnContainerAccessibilityUpdated IContainerInterface::GetAccessibilityChangedDelegate() const
{
	checkNoEntry(); // Has to be reimplemented
	return {};
}
