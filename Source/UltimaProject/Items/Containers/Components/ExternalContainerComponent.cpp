// Fill out your copyright notice in the Description page of Project Settings.

// Game includes
#include "ExternalContainerComponent.h"

#include "Net/UnrealNetwork.h"
#include "UltimaProject/Items/Common/ItemFactoryHelper.h"
#include "UltimaProject/World/HeartbeatSystem/HeartbeatItemProcessor.h"

UExternalContainerComponent::UExternalContainerComponent()
{
	SetIsReplicatedByDefault(true);
}

void UExternalContainerComponent::BeginPlay()
{
	Super::BeginPlay();

	// Server only
	// Spawn defaults
	if (AActor* Owner = GetOwner(); Owner && Owner->HasAuthority())
	{
		for (const auto& Data : DefaultItems)
		{
			SpawnItem(Data);
		}

		for (const auto& DefaultLiquidPair : DefaultLiquids)
		{
			float Amount = DefaultLiquidPair.Value * GetLiquidCapacity();
			SpawnLiquid(DefaultLiquidPair.Key, Amount, false);
		}

		IHeartbeatInterface::Register();
	}
}

void UExternalContainerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	IHeartbeatInterface::Unregister();

	Super::EndPlay(EndPlayReason);
}

void UExternalContainerComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ThisClass, bIsExplicitlyEmpty, COND_None);
}

void UExternalContainerComponent::NotifyContainerItemsChanged_Implementation()
{
	Super::NotifyContainerItemsChanged_Implementation();

	if (GetOwner() && GetOwner()->HasAuthority())
	{
		// TODO is locked check
		// Cache the empty flag for clients 
		bIsExplicitlyEmpty = GetNumItems() == 0;
	}
}

bool UExternalContainerComponent::IsEmpty() const
{
	// Items are not replicated to clients for external containers
	return bIsExplicitlyEmpty;
}

bool UExternalContainerComponent::Heartbeat_Implementation(int64 CurrentTime, int32 TimePassed)
{
	check(GetOwner() && GetOwner()->HasAuthority());

	if (ContainerItems.Items.IsEmpty())
	{
		return true;
	}

	bool bResult = true;
	for (auto& ItemData : ContainerItems.Items)
	{
		const UItemDataAsset* ItemStaticData = ItemData.GetItemData().GetStaticData();
		if (!ItemStaticData)
		{
			bResult = false;
			continue;
		}

		auto ProcessorClass = ItemStaticData->HeartbeatProcessorClass;
		if (!ProcessorClass)
		{
			continue;
		}

		UHeartbeatItemProcessor* ProcessorInstance = UHeartbeatItemProcessor::GetInstance(ProcessorClass);
		if (!ProcessorInstance)
		{
			bResult = false;
			continue;
		}

		bResult &= ProcessorInstance->ProcessHeartbeat(ItemData, CurrentTime, TimePassed);
	}

	return bResult;
}
