// Game includes
#include "ContainerOwnerInterface.h"

#include "UltimaProject/Common/Macro.h"
#include "UltimaProject/Items/Containers/ContainerComponent.h"

bool IContainerOwnerInterface::CanBeOpened(const class AUPPlayerController* Controller) const
{
	return IsValid(Controller) && IsValid(GetContainerComponent());
}

FOnContainerAccessibilityUpdated IContainerOwnerInterface::GetAccessibilityChangedDelegate() const
{
	checkNoEntry(); // Has to be reimplemented
	return {};
}

AActor* IContainerOwnerInterface::GetOwningActor() const
{
	return Cast<AActor>(_getUObject());
}

bool IContainerOwnerInterface::CanStoreItem(AController* Instigator, const UContainerComponent* SourceContainerComponent, int32 ContainerItemHandle) const
{
	AUPPlayerController* PlayerController = Cast<AUPPlayerController>(Instigator);
	NULLCHECK_RETURN(PlayerController, false);

	IContainerOwnerInterface* SourceContainer = SourceContainerComponent->GetOwnerInterface();
	NULLCHECK_RETURN(SourceContainer, false);

	// Check both containers are accessible
	if (!CanBeOpened(PlayerController) ||
		!SourceContainer->CanBeOpened(PlayerController))
	{
		return false;
	}

	// Cheating check - containers have to be opened to move items between. Skip if you own the container
	// TODO shouldbe some other kind of check other than to pawn
	const APawn* Pawn = PlayerController->GetPawn();
	const bool bSourceContainerOpened = GetOwningActor() == Pawn || PlayerController->IsContainerOpened(
		GetContainerComponent());
	const bool bTargetContainerIsOpened = GetOwningActor() == Pawn || PlayerController->IsContainerOpened(
		GetContainerComponent());

	if (!bSourceContainerOpened ||
		!bTargetContainerIsOpened)
	{
		return false;
	}

	return true;
}

void IContainerOwnerInterface::StoreItemImpl(AController* InstigatorController, UContainerComponent* SourceContainerComponent, int32 ContainerItemHandle)
{
	check(InstigatorController->HasAuthority()); // Server only
	NULLCHECK(InstigatorController);

	if (!CanStoreItem(InstigatorController, SourceContainerComponent, ContainerItemHandle))
	{
		return;
	}
	
	UContainerComponent* OriginContainerComponent = IContainerOwnerInterface::Execute_GetContainerComponent(_getUObject());
	NULLCHECK(OriginContainerComponent);
	
	// OriginContainerComponent->ServerTryStoreItem(InstigatorController, ItemData);
}
