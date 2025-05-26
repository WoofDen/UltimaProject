#include "Item.h"

#include "Net/UnrealNetwork.h"
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

	bReplicates = true;
	bReplicateUsingRegisteredSubObjectList = true;
}

void AItem::RemoveFromWorld()
{
	if (!ensureAlways(HasAuthority()))
	{
		return;
	}

	Destroy();
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

void AItem::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	
	if (HasAuthority())
	{
		// Item can be created with ItemData set already or from default
		if (!IsValid(ItemData) && ensureAlways(IsValid(DefaultStaticData)))
		{
			// TODO as any AItem has a UItemData, it may be better to create one within constructor rather than a dynamic one
			UItemData* Data = NewObject<UItemData>(this, FName("ItemData"));
			Data->StaticData = DefaultStaticData;
			Data->InstanceData = DefaultInstanceData;

			AddReplicatedSubObject(Data);

			if (!SetItemData(Data))
			{
				UE_LOG(LogActor, Error, TEXT("Item %s initialization failed"), *GetActorNameOrLabel());
				Data->MarkAsGarbage();
				Destroy();
			}
		}
	}
}

void AItem::BeginPlay()
{
	Super::BeginPlay();
}

void AItem::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AItem, ItemData);
}

void AItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
