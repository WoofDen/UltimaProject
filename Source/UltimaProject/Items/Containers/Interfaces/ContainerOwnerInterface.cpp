// Game includes
#include "ContainerOwnerInterface.h"

AActor* IContainerOwnerInterface::GetOwningActor() const
{
	IContainerOwnerInterface* Interface = const_cast<IContainerOwnerInterface*>(this);
	return Cast<AActor>(Interface);
}

