#pragma once

#include "UltimaProject/Items/Containers/Container.h"
#include "InventoryComponent.generated.h"

UCLASS(Blueprintable)
class ULTIMAPROJECT_API UInventoryComponent : public UContainer
{
	GENERATED_BODY()

	UFUNCTION(Server, Reliable)
	void ServerTryPickupItem(AItem* Item);

	UFUNCTION(Server, Reliable)
	void ServerTryDropItem(const FContainerItemData& Item);
	
	UFUNCTION(Server, Reliable)
	void ServerTrySplitItem(const FContainerItemData& Item, const int64 SplitAmount);
protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	TWeakObjectPtr<APawn> OwnerCharacter;

public:
	UInventoryComponent();

	virtual void BeginPlay() override;

	bool CanPickItem(const AItem* Item) const;

	// Performs pickup action, considering all external conditions are met ( radius/visibility/etc )
	// Client only
	bool TryPickupItem(AItem* Item);
	
	// Client only
	UFUNCTION(BlueprintCallable)
	bool TrySplitItem(UPARAM(ref) const FContainerItemData& Item, const int64 SplitAmount);

	// Client only
	UFUNCTION(BlueprintCallable)
	bool TryDropItem(UPARAM(ref) const FContainerItemData& Item);
};
