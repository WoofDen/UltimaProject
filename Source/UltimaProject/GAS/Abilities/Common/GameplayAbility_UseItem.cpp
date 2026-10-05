// Fill out your copyright notice in the Description page of Project Settings.

#include "GameplayAbility_UseItem.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "UltimaProject/Common/GameplayTags.h"
#include "UltimaProject/Common/InputHelpersFunctionLibrary.h"
#include "UltimaProject/Common/Macro.h"
#include "UltimaProject/UI/HUD/CursorWidget.h"
#include "UltimaProject/UI/HUD/GameplayHUDWidget.h"

bool FGameplayAbilityTargetData_UseItem::NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
{
	Ar << TargetItem;
	Ar << ContainerItemHandle;
	Ar << TargetContainer;

	return true;
}

void UGameplayAbility_UseItem::Initialize()
{
	PlayerController = Cast<AUPPlayerController>(GetActorInfo().PlayerController.Get(false));

	// Load Item Processor
	if (AItem* Item = Data.TargetItem.Get())
	{
		const FItemData& ItemData = Item->GetItemDataConst();

		ItemDataAsset = ItemData.GetStaticData();
		ItemOrigin = Item->GetActorLocation();
	}
	else
	{
		const FContainerItemData& ContainerItemData = Data.TargetContainer->GetItem(Data.ContainerItemHandle);
		if (ContainerItemData.IsValid())
		{
			const FItemData& ItemData = ContainerItemData.GetItemData();

			ItemDataAsset = ItemData.GetStaticData();
		}

		if (const IContainerOwnerInterface* ContainerOwner = Data.TargetContainer->GetOwnerInterface())
		{
			if (const AActor* Owner = ContainerOwner->GetOwningActor())
			{
				ItemOrigin = Owner->GetActorLocation();
			}
		}
	}
}

void UGameplayAbility_UseItem::OnTargetReady(AActor* Actor)
{
	AItem* Item = Cast<AItem>(Actor);
}

void UGameplayAbility_UseItem::OnTargetCancelled()
{
	CancelAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true);
}

bool UGameplayAbility_UseItem::CanUseItem() const
{
	NULLCHECK_RETURN(ItemDataAsset, false);

	const AActor* AvatarActor = GetAvatarActorFromActorInfo();
	NULLCHECK_RETURN(AvatarActor, false);

	const float Distance = FVector::Dist(ItemOrigin, AvatarActor->GetActorLocation());

	if (Distance > MaxTargetDistance)
	{
		UGameplayHUDWidget::GameLog(GetActorInfo().PlayerController.Get(), TEXT("Too far."), false, UP::LogStyle::Error);
		return false;
	}

	return true;
}

bool UGameplayAbility_UseItem::RequestTarget(UClass* TargetClass, TFunction<void(AActor* SelectedTarget)> OnSelectedCallback)
{
	check(GetCurrentActivationInfoRef().ActivationMode == EGameplayAbilityActivationMode::Predicting);

	NULLCHECK_RETURN(TargetClass, false);
	NULLCHECK_RETURN_LOG(TargetingIMC, false, Error, "Targeting input mapping context isn't set for the UseItem ability");
	NULLCHECK_SP_RETURN(PlayerController, false);
	NULLCHECK_RETURN(PlayerController->GetPawn(), false);

	UEnhancedInputLocalPlayerSubsystem* EnhancedInputSubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer());
	NULLCHECK_RETURN(EnhancedInputSubsystem, false);

	// Validation ok.
	// Register targeting inputs
	EnhancedInputSubsystem->AddMappingContext(TargetingIMC, 0);

	// Enable visuals
	if (UCursorWidget* CursorWidget = PlayerController->GetCursorWidget())
	{
		CursorWidget->SetTargetMode(true);
	}

	// Register target callbacks
	UEnhancedInputComponent* InputComponent = Cast<UEnhancedInputComponent>(PlayerController->GetPawn()->InputComponent);

	// On target selected callcback
	TargetSelectedBindingHandle = InputComponent->BindActionValueLambda(TargetSelectIA, ETriggerEvent::Completed, [this, OnSelectedCallback](const FInputActionValue& InputActionValue)
	{
		if (!IsValid(this))
		{
			return;
		}

		AActor* Actor = UInputHelpersFunctionLibrary::GetActorUnderCursor(PlayerController.Get(), ECC_InteractableChannel);
		OnSelectedCallback(Actor);
	}).GetHandle();

	// On target canceled callback
	TargetCancelledBindingHandle = InputComponent->BindActionValueLambda(TargetCancelIA, ETriggerEvent::Started, [this](const FInputActionValue& InputActionValue)
	{
		if (!IsValid(this))
		{
			return;
		}

		CancelAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), false);
	}).GetHandle();

	return true;
}

void UGameplayAbility_UseItem::CancelTarget()
{
	NULLCHECK_SP(PlayerController);

	// Remove IMC
	if (UEnhancedInputLocalPlayerSubsystem* EnhancedInputSubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
	{
		EnhancedInputSubsystem->RemoveMappingContext(TargetingIMC);
	}

	// Remove callbacks
	if (APawn* Pawn = PlayerController->GetPawn())
	{
		if (UEnhancedInputComponent* InputComponent = Cast<UEnhancedInputComponent>(Pawn->InputComponent))
		{
			InputComponent->RemoveBindingByHandle(TargetSelectedBindingHandle);
			InputComponent->RemoveBindingByHandle(TargetCancelledBindingHandle);
		}
	}

	// Enable visuals
	if (UCursorWidget* CursorWidget = PlayerController->GetCursorWidget())
	{
		CursorWidget->SetTargetMode(false);
	}
}

void UGameplayAbility_UseItem::OnDataCollected()
{
	check(GetCurrentActivationInfo().ActivationMode == EGameplayAbilityActivationMode::Predicting);
}

UGameplayAbility_UseItem::UGameplayAbility_UseItem()
{
	SetAssetTags(FGameplayTagContainer(TAG_Ability_UseItem));

	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

	bIsCancelable = true;
	bServerRespectsRemoteAbilityCancellation = true;

	FAbilityTriggerData TriggerData;
	TriggerData.TriggerSource = EGameplayAbilityTriggerSource::Type::GameplayEvent;
	TriggerData.TriggerTag = TAG_Ability_UseItem;

	AbilityTriggers.Add(TriggerData);
}

void UGameplayAbility_UseItem::PreActivate(const FGameplayAbilitySpecHandle Handle,
                                           const FGameplayAbilityActorInfo* ActorInfo,
                                           const FGameplayAbilityActivationInfo ActivationInfo,
                                           FOnGameplayAbilityEnded::FDelegate* OnGameplayAbilityEndedDelegate,
                                           const FGameplayEventData* TriggerEventData)
{
	Super::PreActivate(Handle, ActorInfo, ActivationInfo, OnGameplayAbilityEndedDelegate, TriggerEventData);

	const FGameplayAbilityTargetData_UseItem* TargetData = static_cast<const
		FGameplayAbilityTargetData_UseItem*>(TriggerEventData->TargetData.Get(0));

	if (!TargetData || !TargetData->IsValid())
	{
		CancelAbility(Handle, ActorInfo, ActivationInfo, true);
		return;
	}

	Data = *TargetData;

	Initialize();

	// Init. ok
	// Validate the interaction
	if (!CanUseItem())
	{
		CancelAbility(Handle, ActorInfo, ActivationInfo, true);
	}
}

void UGameplayAbility_UseItem::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                               const FGameplayAbilityActorInfo* ActorInfo,
                                               const FGameplayAbilityActivationInfo ActivationInfo,
                                               const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (GetCurrentActivationInfo().ActivationMode == EGameplayAbilityActivationMode::Predicting)
	{
	}
}

void UGameplayAbility_UseItem::EndAbility(const FGameplayAbilitySpecHandle Handle,
                                          const FGameplayAbilityActorInfo* ActorInfo,
                                          const FGameplayAbilityActivationInfo ActivationInfo,
                                          bool bReplicateEndAbility,
                                          bool bWasCancelled)
{
	CancelTarget();

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
