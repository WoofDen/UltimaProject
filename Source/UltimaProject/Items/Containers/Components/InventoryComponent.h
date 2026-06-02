#pragma once

#include "UltimaProject/Items/Containers/ContainerComponent.h"
#include "InventoryComponent.generated.h"

UCLASS(Blueprintable)
class ULTIMAPROJECT_API UInventoryComponent : public UContainerComponent
{
	GENERATED_BODY()

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

	bool CanStoreItem(const AItem* Item) const;

	void TryStoreItem(AItem* Item);

	// Client only
	UFUNCTION(BlueprintCallable)
	bool TrySplitItem(UPARAM(ref) const FContainerItemData& Item, const int64 SplitAmount);

	// Client only
	UFUNCTION(BlueprintCallable)
	bool TryDropItem(UPARAM(ref) const FContainerItemData& Item);
};
