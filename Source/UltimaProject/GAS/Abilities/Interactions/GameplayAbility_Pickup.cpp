// Fill out your copyright notice in the Description page of Project Settings.

// Game includes
#include "GameplayAbility_Pickup.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemLog.h"
#include "UltimaProject/Characters/UPCharacter.h"
#include "UltimaProject/Common/Macro.h"
#include "UltimaProject/Items/Common/Item.h"
#include "UltimaProject/Items/Containers/PlayerInventory/InventoryComponent.h"

AItem* UGameplayAbility_Pickup::GetItemUnderCursor()
{
	check(!GetActorInfo().OwnerActor->HasAuthority());

	APlayerController* PC = GetActorInfo().PlayerController.Get();
	if (!PC || !PC->Player)
	{
		return nullptr;
	}

	// Trace for an actor under the cursor
	FHitResult HitResult;
	if (ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(PC->Player); LocalPlayer && LocalPlayer->ViewportClient)
	{
		FVector2D MousePosition;
		if (LocalPlayer->ViewportClient->GetMousePosition(MousePosition))
		{
			TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
			ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_WorldDynamic));

			PC->GetHitResultAtScreenPosition(
				MousePosition,
				ObjectTypes,
				false,
				HitResult);
		}
	}

	if (!HitResult.bBlockingHit)
	{
		return nullptr;
	}

	return Cast<AItem>(HitResult.GetActor());
}

bool UGameplayAbility_Pickup::CanPickupItem(const AItem* Item)
{
	// Validation
	AUPCharacter* Character = Cast<AUPCharacter>(GetAvatarActorFromActorInfo());
	if (!Item || !Character || !Character->GetInventoryComponent())
	{
		return false;
	}

	// Distance check
	float Distance = (GetActorInfo().AvatarActor->GetActorLocation() - Item->GetActorLocation()).Length();
	if (Distance > PickupRadius)
	{
		return false;
	}

	// Inventory capacity & other checks
	if (!Character->GetInventoryComponent()->CanStoreItem(Item))
	{
		return false;
	}

	return true;
}

void UGameplayAbility_Pickup::PickupItemInternal()
{
	check(K2_HasAuthority()); // Server only
	
	if (!bServerTargetDataReady || !TargetItem.IsValid() || !IsInteractionFinished())
	{
		// To start the picking attempt the next conditions should be met
		// - Data is replicated ( most probably the earliest event )
		// - Interaction is done
		return;
	}
	
	if (AUPCharacter* Character = Cast<AUPCharacter>(GetAvatarActorFromActorInfo()))
	{
		if (UInventoryComponent* InventoryComponent = Character->GetInventoryComponent())
		{
			InventoryComponent->TryStoreItem(TargetItem.Get());
		}
	}
	
	// Regardless of the result, end the ability
	EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, false);
}

void UGameplayAbility_Pickup::OnTargetDataReady(const FGameplayAbilityTargetDataHandle& DataHandle,
                                                FGameplayTag Tag)
{
	// Validation
	if (!IsActive() || DataHandle.Num() == 0 || DataHandle.Get(0)->GetActors().IsEmpty())
	{
		UE_LOG(LogAbilitySystem, Error, TEXT("Pickup: Invalid data received from server"));
		EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), false, true);
		return;
	}
	
	bServerTargetDataReady = true;

	if (const FGameplayAbilityTargetData* TargetDataFirst = DataHandle.Get(0))
	{
		if (TArray<TWeakObjectPtr<AActor>> TargetActors = TargetDataFirst->GetActors(); !TargetActors.IsEmpty())
		{
			TargetItem = Cast<AItem>(TargetActors[0]);
			PickupItemInternal();
			
			return;
		}
	}
	
	UE_LOG(LogAbilitySystem, Error, TEXT("Pickup(2): Invalid data received from server"));
	EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), false, true);
}

void UGameplayAbility_Pickup::OnInteractionFinished()
{
	Super::OnInteractionFinished();
	PickupItemInternal();
}

void UGameplayAbility_Pickup::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                              const FGameplayAbilityActorInfo* ActorInfo,
                                              const FGameplayAbilityActivationInfo ActivationInfo,
                                              const FGameplayEventData* TriggerEventData)
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	const AActor* AvatarActor = GetActorInfo().AvatarActor.Get();

	// Validation
	if (!ASC || !GetActorInfo().AvatarActor.IsValid() || !IsValid(AvatarActor))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// (Client) 
	if (GetActorInfo().OwnerActor->GetNetMode() == NM_Client)
	{
		AItem* CursorItem = GetItemUnderCursor();
		if (!CursorItem || !CanPickupItem(CursorItem))
		{
			EndAbility(Handle, ActorInfo, ActivationInfo, false, true);
			return;
		}

		// Send TargetData to Server
		FGameplayAbilityTargetData_ActorArray* Data = new FGameplayAbilityTargetData_ActorArray();
		Data->TargetActorArray.Add(CursorItem);
		FGameplayAbilityTargetDataHandle Handle(Data); // Wraps Data in TSharedPtr

		FGameplayTag ApplicationTag;
		ASC->CallServerSetReplicatedTargetData( // Reliable
			GetCurrentAbilitySpecHandle(),
			GetCurrentActivationInfo().GetActivationPredictionKey(),
			Handle,
			ApplicationTag,
			ASC->ScopedPredictionKey
		);
	}
	// (Server)
	else if (GetActorInfo().OwnerActor->GetNetMode() == NM_DedicatedServer)
	{
		bServerTargetDataReady = false;
		// Set up a delegate to wait for target data replication
		ASC->AbilityTargetDataSetDelegate(
			GetCurrentAbilitySpecHandle(),
			GetCurrentActivationInfo().GetActivationPredictionKey()
		).AddUObject(this, &ThisClass::OnTargetDataReady);
	}

	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void UGameplayAbility_Pickup::EndAbility(const FGameplayAbilitySpecHandle Handle,
                                         const FGameplayAbilityActorInfo* ActorInfo,
                                         const FGameplayAbilityActivationInfo ActivationInfo,
                                         bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
	{
		ASC->AbilityTargetDataSetDelegate(
			GetCurrentAbilitySpecHandle(),
			GetCurrentActivationInfo().GetActivationPredictionKey()
		).RemoveAll(this);
	}
}
