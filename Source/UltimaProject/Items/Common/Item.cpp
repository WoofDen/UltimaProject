#include "Item.h"

#include "ISinglePropertyView.h"
#include "Net/UnrealNetwork.h"
#include "ItemFactoryHelper.h"
#include "TimerManager.h"
#include "UltimaProject/Common/Macro.h"
#include "UltimaProject/World/HeartbeatSystem/HeartbeatItemProcessor.h"

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

	ItemDefault.InstanceData.Amount = 1;
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
	// It should be could once after actor creation (so far)
	const bool bWasInitialized = HasActorBegunPlay() || ItemData.IsValid();
	ensureAlways(!bWasInitialized);

	if (bWasInitialized)
	{
		return false;
	}

	ItemData = NewData;
	ItemData.LoadStaticData();

	OnStaticDataSet();

	return true;
}

void AItem::OnStaticDataSet()
{
	NULLCHECK(StaticMeshComponent);

	const UItemDataAsset* ItemDataAsset = ItemData.GetStaticData();
	NULLCHECK_LOG(ItemDataAsset, Error, "Failed to initialize static data");

	// Default properties
	ItemData.InstanceData.Properties = ItemDataAsset->DefaultProperties;

	// Visual overrides
	if (UStaticMesh* Mesh = ItemDataAsset->WorldMesh.LoadSynchronous())
	{
		StaticMeshComponent->SetStaticMesh(Mesh);

		if (ItemDataAsset->bOverrideWorldTransform)
		{
			StaticMeshComponent->SetRelativeTransform(ItemDataAsset->WorldMeshTransform);
		}

		const auto Materials = Mesh->GetStaticMaterials();
		for (int32 i = 0; i < Materials.Num(); i++)
		{
			StaticMeshComponent->SetMaterial(0, Materials[i].MaterialInterface);
		}
	}
}

void AItem::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (HasAuthority() && !HasAnyFlags(RF_ClassDefaultObject))
	{
		// Item can be created with ItemData set already or from default
		if (!ItemData.IsValid() && ItemDefault.IsValid())
		{
			// TODO as any AItem has a UItemData, it may be better to create one within constructor rather than a dynamic one
			FItemDataDefinition DefaultItemDefinition(ItemDefault.GetStaticData(), ItemDefault.InstanceData);
			FItemData DefaultItemData(MoveTemp(DefaultItemDefinition));

			if (!SetItemData(MoveTemp(DefaultItemData)))
			{
				UE_LOG(LogActor, Error, TEXT("Item %s initialization failed"), *GetActorNameOrLabel());
				Destroy();
			}
		}
	}
}

void AItem::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		IHeartbeatInterface::Register();
	}

	// Validate DA
	if (const UItemDataAsset* DataAsset = ItemData.GetStaticData())
	{
		// Destroy actor if DA class was changed
		if (DataAsset->ActorClass != GetClass())
		{
			UE_LOG(LogTemp, Warning, TEXT("Actor item mismatch - item %s is a child of %s, expected %s"), *GetActorNameOrLabel(), GetClass() ? *GetClass()->GetName() : TEXT_EMPTY,
			       *DataAsset->ActorClass->GetName());

			Destroy();
		}
		else
		{
			OnStaticDataSet();
		}
	}
	else if (HasAuthority())
	{
		// Sometimes ItemData isn't ready on BeginPlay
		// Initialization will happen on OnRep_ItemData
		UE_LOG(LogTemp, Warning, TEXT("Actor item %s missing item data asset"), *GetActorNameOrLabel());

		Destroy();
	}
}

void AItem::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (HasAuthority())
	{
		IHeartbeatInterface::Unregister();
	}

	Super::EndPlay(EndPlayReason);
}

void AItem::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	// Initialize ItemDefault
	if (ItemDefault.IsValid())
	{
		ItemData = FItemData(ItemDefault);
		OnStaticDataSet();
	}
	// TODO add an invalid item visuals to highlight the invalid actor
}

void AItem::OnRep_ItemData(FItemData PrevValue)
{
	if (PrevValue.GetStaticData() == nullptr)
	{
		// Late client initialization
		OnStaticDataSet();
	}

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

bool AItem::Heartbeat_Implementation(int64 CurrentTime, int32 TimePassed)
{
	check(HasAuthority());
	IS_VALID_RETURN(ItemData.StaticData, false);

	auto ProcessorClass = ItemData.StaticData->HeartbeatProcessorClass;
	NULLCHECK_RETURN(ProcessorClass, false);

	UHeartbeatItemProcessor* ProcessorInstance = UHeartbeatItemProcessor::GetInstance(ProcessorClass);
	NULLCHECK_RETURN(ProcessorInstance, false);

	return ProcessorInstance->ProcessHeartbeat(this, CurrentTime, TimePassed);
}
