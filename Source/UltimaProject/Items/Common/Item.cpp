#include "Item.h"

#include "UltimaProject/Framework/UPPlayerState.h"

AItem::AItem()
{
	PrimaryActorTick.bCanEverTick = true;

	SphereComponent = CreateDefaultSubobject<USphereComponent>("SphereCollisionComponent");
	SetRootComponent(SphereComponent);
	SphereComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SphereComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	SphereComponent->SetCollisionObjectType(ECC_WorldDynamic);
	SphereComponent->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);

	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>("StaticMeshComponent");
	StaticMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	StaticMeshComponent->SetupAttachment(RootComponent);

	HoverWidget = CreateDefaultSubobject<UWidgetComponent>("HoverWidgetComponent");
	HoverWidget->SetVisibility(false);
	HoverWidget->SetupAttachment(RootComponent);
	HoverWidget->SetCastShadow(false);
	HoverWidget->SetComponentTickEnabled(false);

	SetReplicates(true);
}

void AItem::RemoveFromWorld()
{
	if (!ensure(HasAuthority()))
	{
		return;
	}

	MulticastRemoveFromWorld();
}

bool AItem::SetItemData_Implementation(UItemData* NewData /*= nullptr*/)
{
	// It should be could once after actor creation ( so far )
	if (!ensureAlways(ItemData == nullptr))
	{
		return false;
	}

	ItemData = NewData;

	auto ItemStaticData = ItemData->GetStaticData();
	if (!ensure(ItemStaticData))
	{
		return false;
	}

	if (StaticMeshComponent)
	{
		UStaticMesh* Mesh = ItemData->GetStaticData()->WorldMesh.Get();
		StaticMeshComponent->SetStaticMesh(Mesh);
	}

	return true;
}

void AItem::MulticastRemoveFromWorld_Implementation()
{
	ensure(!HasAuthority());
	Destroy();
}

void AItem::BeginPlay()
{
	// Item can be created with ItemData set already or from default
	if (!IsValid(ItemData) && ensureAlways(IsValid(DefaultStaticData)))
	{
		UItemData* Data = NewObject<UItemData>(this, FName("ItemData"));
		Data->StaticData = DefaultStaticData;
		Data->InstanceData = DefaultInstanceData;

		if (!SetItemData(Data))
		{
			UE_LOG(LogActor, Error, TEXT("Item %s initialization failed"), *GetActorNameOrLabel());
			Data->MarkAsGarbage();
			Destroy();
		}
	}

	// Calls BP impl.
	Super::BeginPlay();
}

void AItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
