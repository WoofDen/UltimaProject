#include "Item.h"

#include "Misc/TypeContainer.h"
#include "Net/UnrealNetwork.h"
#include "UltimaProject/Common/Macro.h"
#include "UltimaProject/Framework/UPPlayerState.h"
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
	const bool bWasInitialized = HasActorBegunPlay() || ItemData.IsValid();
	ensureAlways(!bWasInitialized);

	if (bWasInitialized)
	{
		return false;
	}

	ItemData = NewData;
	ItemData.LoadStaticData();

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
			FItemDataDefinition DefaultItemDefinition(DefaultStaticData, DefaultInstanceData);
			FItemData DefaultItemData(MoveTemp(DefaultItemDefinition));

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

	if (HasAuthority())
	{
		IHeartbeatInterface::Register();
	}
}

void AItem::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (HasAuthority())
	{
		IHeartbeatInterface::Register();
	}

	Super::EndPlay(EndPlayReason);
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
