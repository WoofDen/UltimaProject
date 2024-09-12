#pragma once

#include "Interfaces\ContainerInterface.h"
#include "Container.generated.h"

USTRUCT(BlueprintType)
struct FContainerItemData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	UItemData* ItemData = nullptr;

	TWeakObjectPtr<UContainer> Container = nullptr;

	bool operator==(const FContainerItemData& Other) const;
};

USTRUCT(BlueprintType)
struct FItemTransactionResult
{
	GENERATED_BODY()
};

UCLASS(Abstract)
class UContainer : public UActorComponent, public IContainerInterface
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FContainerItemData> Items;

	UPROPERTY(VisibleAnywhere)
	int64 WeightCapacity = 0;

	UPROPERTY(VisibleAnywhere)
	int64 ItemsCapacity = 0;

	// Find position nearby where we can safely drop an item
	bool FindDropTransform(FContainerItemData& Item, FTransform& Result) const;

public:
	inline static int64 MaxItemsCapacity = MAX_int64;
	inline static int64 MaxWeightCapacity = MAX_int64;

	FORCEINLINE int64 GetWeightCapacity() const
	{
		return WeightCapacity;
	}

	FORCEINLINE int64 GetItemsCapacity() const
	{
		return ItemsCapacity;
	}

	virtual void SetItemsCapacity(const int64 NewValue);
	virtual void SetWeightCapacity(const int64 NewValue);

	UFUNCTION(BlueprintCallable)
	virtual bool AddItem(UPARAM(ref) FContainerItemData& ItemData, FItemTransactionResult& Result);

	virtual bool AddItem(FContainerItemData& ItemData);

	UFUNCTION(BlueprintCallable)
	virtual bool MoveItem(FContainerItemData ItemData, FItemTransactionResult& Result);

	UFUNCTION(BlueprintCallable)
	virtual bool RemoveItem(FContainerItemData ItemData);

	UFUNCTION(BlueprintCallable)
	bool GetRandomItem(FContainerItemData& Item) const;

#pragma region ContainerInterface

public:
	UFUNCTION(BlueprintCallable)
	virtual AItem* DropItem(UPARAM(ref) FContainerItemData& Item) override;

	virtual TArray<FContainerItemData> GetItems() override;
#pragma endregion
};
