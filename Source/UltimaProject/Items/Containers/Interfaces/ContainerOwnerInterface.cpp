// Game includes
#include "ContainerOwnerInterface.h"

#include "UltimaProject/Common/Macro.h"
#include "UltimaProject/Items/Containers/ContainerComponent.h"

bool IContainerOwnerInterface::CanBeOpened(const class AUPPlayerController* Controller) const
{
	return IsValid(Controller) && IsValid(GetMainContainerComponent());
}

FOnContainerAccessibilityUpdated IContainerOwnerInterface::GetAccessibilityChangedDelegate() const
{
	checkNoEntry(); // Has to be reimplemented
	return {};
}

AActor* IContainerOwnerInterface::GetOwningActor() const
{
	IContainerOwnerInterface* Interface = const_cast<IContainerOwnerInterface*>(this);
	return Cast<AActor>(Interface);
}

