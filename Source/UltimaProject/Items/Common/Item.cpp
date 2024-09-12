#include "Item.h"

AItem::AItem()
{
	PrimaryActorTick.bCanEverTick = false;

	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>("StaticMeshComponent");
	SetRootComponent(StaticMeshComponent);

	StaticMeshComponent->SetGenerateOverlapEvents(false);
	StaticMeshComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	StaticMeshComponent->SetCollisionObjectType(ECC_WorldDynamic);
	StaticMeshComponent->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
}

AItem::AItem(UItemData* ItemData)
{
	SetItemData(ItemData);
}

bool AItem::SetItemData(UItemData* NewData /*= nullptr*/)
{
	// It should be could once after actor creation ( so far )
	if (!ensureAlways(Data == nullptr))
	{
		return false;
	}

	Data = NewData;

	if (StaticMeshComponent)
	{
		UStaticMesh* Mesh = Data->GetStaticData()->WorldMesh.Get();
		StaticMeshComponent->SetStaticMesh(Mesh);
	}

	return true;
}

void AItem::BeginPlay()
{
	Super::BeginPlay();
}

void AItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
