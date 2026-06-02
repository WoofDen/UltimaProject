#include "ContainerInterface.h"

bool IContainerInterface::CanBeOpened(const class AUPPlayerController* Controller)
{
	return true;
}

TSubclassOf<UContainerWidget> IContainerInterface::GetDisplayWidgetClass() const
{
	return nullptr;
}
