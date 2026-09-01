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

bool AItem::SetItemData(FItemData&& NewData)
{
	// It should be could once after actor creation ( so far )
	if (ensureAlways(!ItemData.IsValid()))
	{
		return false;
	}

	ItemData = NewData;

	auto ItemStaticData = ItemData.GetStaticData();
	if (!ensure(ItemStaticData))
	{
		return false;
	}

	if (StaticMeshComponent)
	{
		UStaticMesh* Mesh = ItemData.GetStaticData()->WorldMesh.Get();
		StaticMeshComponent->SetStaticMesh(Mesh);
	}

	return true;
}

void AItem::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	
	if (HasAuthority() && !HasAnyFlags(RF_ClassDefaultObject))
	{
		// Item can be created with ItemData set already or from default
		if (!ItemData.IsValid() && ensureAlways(IsValid(DefaultStaticData)))
		{
			// TODO as any AItem has a UItemData, it may be better to create one within constructor rather than a dynamic one
			FItemDataDefinition DefaultItemDefinition (DefaultStaticData, DefaultInstanceData);
			FItemData DefaultItemData (MoveTemp(DefaultItemDefinition));
			
			if (!SetItemData(MoveTemp(DefaultItemData)))
			{
				UE_LOG(LogActor, Error, TEXT("Item %s initialization failed"), *GetActorNameOrLabel());
				//Data->MarkAsGarbage();
				Destroy();
			}

			//AddReplicatedSubObject(Data);

		}
	}
}

void AItem::BeginPlay()
{
	Super::BeginPlay();
	
}

void AItem::OnRep_ItemData()
{
	OnItemDataChanged();
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
