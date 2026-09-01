// Game includes
#include "ContainerInterface.h"

#include "UltimaProject/Common/Macro.h"
#include "UltimaProject/Items/Containers/ContainerComponent.h"

bool IContainerInterface::CanBeOpened(const class AUPPlayerController* Controller) const
{
	return IsValid(Controller) && IsValid(GetContainerComponent());
}

FOnContainerAccessibilityUpdated IContainerInterface::GetAccessibilityChangedDelegate() const
{
	checkNoEntry(); // Has to be reimplemented
	return {};
}

AActor* IContainerInterface::GetOwningActor() const
{
	return Cast<AActor>(_getUObject());
}

bool IContainerInterface::CanStoreItem(AController* Instigator, const FContainerItemData& ContainerItemData) const
{
	AUPPlayerController* PlayerController = Cast<AUPPlayerController>(Instigator);
	NULLCHECK_RETURN(PlayerController, false);

	UContainerComponent* SourceContainerComponent = ContainerItemData.GetContainerComponent();
	NULLCHECK_RETURN(SourceContainerComponent, false);

	IContainerInterface* SourceContainer = SourceContainerComponent->GetOwnerInterface();
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
		SourceContainer);
	const bool bTargetContainerIsOpened = GetOwningActor() == Pawn || PlayerController->IsContainerOpened(
		this);

	if (!bSourceContainerOpened ||
		!bTargetContainerIsOpened)
	{
		return false;
	}

	return true;
}

void IContainerInterface::StoreItemImpl(AController* InstigatorController, const FContainerItemData& ItemData)
{
	NULLCHECK(InstigatorController);
	check(InstigatorController->HasAuthority()); // Server only

	if (!CanStoreItem(InstigatorController, ItemData))
	{
		return;
	}
	
	UContainerComponent* OriginContainerComponent = IContainerInterface::Execute_GetContainerComponent(_getUObject());
	NULLCHECK(OriginContainerComponent);
	
	OriginContainerComponent->ServerTryStoreItem(InstigatorController, ItemData);
}
