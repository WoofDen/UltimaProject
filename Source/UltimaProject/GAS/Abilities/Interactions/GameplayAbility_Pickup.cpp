// Fill out your copyright notice in the Description page of Project Settings.

// Game includes
#include "GameplayAbility_Pickup.h"

#include "AbilitySystemComponent.h"
#include "UltimaProject/Characters/UPCharacter.h"
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
	AUPCharacter* Character = Cast<AUPCharacter>(GetAvatarActorFromActorInfo());
	if (!Item || !Character || !Character->GetInventoryComponent())
	{
		return false;
	}

	float Distance = (GetActorInfo().AvatarActor->GetActorLocation() - Item->GetActorLocation()).Length();
	if (Distance > PickupRadius)
	{
		return false;
	}

	if (!Character->GetInventoryComponent()->CanStoreItem(Item))
	{
		return false;
	}

	return true;
}

void UGameplayAbility_Pickup::ServerOnReplicatedDataReady(const FGameplayAbilityTargetDataHandle& DataHandle,
                                                          FGameplayTag Tag)
{
	// Validation
	if (!IsActive() || DataHandle.Num() == 0 || DataHandle.Get(0)->GetActors().IsEmpty())
	{
		return;
	}

	AItem* Item = Cast<AItem>(DataHandle.Get(0)->GetActors()[0]);
	AUPCharacter* Character = Cast<AUPCharacter>(GetAvatarActorFromActorInfo());
	if (!Item || !Character || !CanPickupItem(Item) || !Character->GetInventoryComponent())
	{
		return;
	}

	Character->GetInventoryComponent()->ServerTryPickupItem(Item);
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
		AItem* TargetItem = GetItemUnderCursor();
		if (!TargetItem || !CanPickupItem(TargetItem))
		{
			EndAbility(Handle, ActorInfo, ActivationInfo, false, true);
			return;
		}

		FGameplayAbilityTargetData_ActorArray* Data = new FGameplayAbilityTargetData_ActorArray();
		Data->TargetActorArray.Add(TargetItem);
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
		ASC->AbilityTargetDataSetDelegate(
			GetCurrentAbilitySpecHandle(),
			GetCurrentActivationInfo().GetActivationPredictionKey()
		).AddUObject(this, &ThisClass::ServerOnReplicatedDataReady);
	}

	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void UGameplayAbility_Pickup::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
	
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
	{
		ASC->AbilityTargetDataSetDelegate(GetCurrentAbilitySpecHandle(), GetCurrentActivationInfo().GetActivationPredictionKey()).RemoveAll(this);
	}
}
