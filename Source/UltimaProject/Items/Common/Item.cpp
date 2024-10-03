#include "Item.h"

AItem::AItem()
{
	PrimaryActorTick.bCanEverTick = true;

	SphereComponent = CreateDefaultSubobject<USphereComponent>("SphereCollisionComponent");
	SetRootComponent(SphereComponent);

	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>("StaticMeshComponent");

	StaticMeshComponent->SetGenerateOverlapEvents(false);
	StaticMeshComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	StaticMeshComponent->SetCollisionObjectType(ECC_WorldDynamic);
	StaticMeshComponent->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	StaticMeshComponent->SetupAttachment(RootComponent);

	HoverWidget = CreateDefaultSubobject<UWidgetComponent>("HoverWidgetComponent");
	HoverWidget->SetVisibility(false);
	HoverWidget->SetupAttachment(RootComponent);
	HoverWidget->SetCastShadow(false);
	HoverWidget->SetComponentTickEnabled(false);
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
