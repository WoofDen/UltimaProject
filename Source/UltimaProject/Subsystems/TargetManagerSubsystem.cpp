// Fill out your copyright notice in the Description page of Project Settings.


#include "TargetManagerSubsystem.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/AssetManager.h"
#include "UltimaProject/Common/InputHelpersFunctionLibrary.h"
#include "UltimaProject/Common/Macro.h"
#include "UltimaProject/UI/HUD/CursorWidget.h"

class UEnhancedInputLocalPlayerSubsystem;

FPrimaryAssetId UTargetingSettingsDataAsset::GetId()
{
	return FPrimaryAssetId("TargetingSettings", FName("Settings"));
}

FPrimaryAssetId UTargetingSettingsDataAsset::GetPrimaryAssetId() const
{
	return GetId();
}

bool UTargetManagerSubsystem::SetupBindings(TFunction<void()> TargetConfirmCallback)
{
	NULLCHECK_SP_RETURN(PlayerController, false);

	APawn* Pawn = PlayerController->GetPawn();
	NULLCHECK_RETURN(Pawn, false);

	UEnhancedInputComponent* InputComponent = Cast<UEnhancedInputComponent>(PlayerController->GetPawn()->InputComponent);
	NULLCHECK_RETURN(InputComponent, false);

	UEnhancedInputLocalPlayerSubsystem* EnhancedInputSubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer());
	NULLCHECK_RETURN(EnhancedInputSubsystem, false);

	UObject* SettingsObject = UAssetManager::Get().GetPrimaryAssetObject(UTargetingSettingsDataAsset::GetId());
	UTargetingSettingsDataAsset* Settings = Cast<UTargetingSettingsDataAsset>(SettingsObject);
	NULLCHECK_RETURN(Settings, false);

	// Setup inputs
	// Main input mapping context
	EnhancedInputSubsystem->AddMappingContext(Settings->TargetingIMC, 0);

	// On target selected callback
	TargetSelectedBindingHandle = InputComponent->BindActionValueLambda(Settings->TargetConfirmIA, ETriggerEvent::Completed, [this, TargetConfirmCallback](const FInputActionValue& InputActionValue)
	{
		TargetConfirmCallback();
	}).GetHandle();

	// On target canceled callback
	TargetCancelledBindingHandle = InputComponent->BindActionValueLambda(Settings->TargetCancelIA, ETriggerEvent::Started, [this](const FInputActionValue& InputActionValue)
	{
		if (!IsValid(this))
		{
			return;
		}

		OnTargetCancelled();
	}).GetHandle();
	
	return true;
}

void UTargetManagerSubsystem::ResetBindings()
{
	if (PlayerController.IsValid() && PlayerController->GetPawn())
	{
		if (UEnhancedInputComponent* InputComponent = Cast<UEnhancedInputComponent>(PlayerController->GetPawn()->InputComponent))
		{
			InputComponent->RemoveBindingByHandle(TargetSelectedBindingHandle);
			InputComponent->RemoveBindingByHandle(TargetCancelledBindingHandle);
		}
	}
}

void UTargetManagerSubsystem::RunTargetingScheme()
{
	if (!CurrentScheme.IsValidIndex(CurrentSchemeIndex))
	{
		OnTargetDataReady.ExecuteIfBound(this);
		return;
	}

	const auto TargetSchemeRow = CurrentScheme[CurrentSchemeIndex];
	if (TargetSchemeRow.GetScriptStruct() == FTargetDescriptor_Actor::StaticStruct())
	{
		const FTargetDescriptor_Actor& TargetActorDescriptor = TargetSchemeRow.Get<FTargetDescriptor_Actor>();

		TargetActor();
	}
	else
	{
		checkNoEntry();
	}
}

bool UTargetManagerSubsystem::TargetActor()
{
	UObject* SettingsObject = UAssetManager::Get().GetPrimaryAssetObject(UTargetingSettingsDataAsset::GetId());
	UTargetingSettingsDataAsset* Settings = Cast<UTargetingSettingsDataAsset>(SettingsObject);
	NULLCHECK_RETURN(Settings, false);

	// Validation ok.
	// Enable visuals
	if (UCursorWidget* CursorWidget = PlayerController->GetCursorWidget())
	{
		CursorWidget->SetTargetMode(true);
	}

	return SetupBindings([this]
	{
		if (!IsValid(this))
		{
			return;
		}

		AActor* Actor = UInputHelpersFunctionLibrary::GetActorUnderCursor(PlayerController.Get(), ECC_InteractableChannel);
		if (Actor)
		{
			OnTargetConfirmed(Actor);
		}
		else
		{
			OnTargetCancelled();
		}
	});
}

void UTargetManagerSubsystem::OnTargetConfirmed(AActor* Actor)
{
	if (!CurrentScheme.IsValidIndex(CurrentSchemeIndex))
	{
		OnTargetDataReady.ExecuteIfBound(this);
		return;
	}

	const auto TargetSchemeRow = CurrentScheme[CurrentSchemeIndex];
	bool bTargetIsValid = false;
	if (TargetSchemeRow.GetScriptStruct() == FTargetDescriptor_Actor::StaticStruct())
	{
		// Validate actor
		const FTargetDescriptor_Actor& TargetActorDescriptor = TargetSchemeRow.Get<FTargetDescriptor_Actor>();

		for (const UClass* FilterClass : TargetActorDescriptor.TargetClasses)
		{
			if (!Actor->IsA(FilterClass))
			{
				continue;
			}

			bTargetIsValid = true;
			SelectedActors.Add(Actor);
			break;
		}
	}
	else
	{
		checkNoEntry();
	}

	if (bTargetIsValid)
	{
		CurrentSchemeIndex++;
		RunTargetingScheme(); // Jump next target or finish
	}
	else
	{
		OnTargetInvalid();
	}
}

void UTargetManagerSubsystem::OnTargetInvalid()
{
	ResetBindings();
	OnTargetCancel.ExecuteIfBound();
}

void UTargetManagerSubsystem::OnTargetCancelled()
{
	ResetBindings();
	OnTargetCancel.ExecuteIfBound();
}

void UTargetManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UTargetManagerSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

void UTargetManagerSubsystem::PlayerControllerChanged(APlayerController* NewPlayerController)
{
	Super::PlayerControllerChanged(NewPlayerController);

	PlayerController = Cast<AUPPlayerController>(NewPlayerController);
	ensureAlways(PlayerController != nullptr);

	// Add the progress widget to HUD
	if (PlayerController.IsValid())
	{
		if (UCursorWidget* CursorWidget = PlayerController->GetCursorWidget())
		{
			CursorWidget->SetTargetMode(false);
		}
	}
}

void UTargetManagerSubsystem::StartTargetingFromContextAction(const FItemContextAction& Action)
{
	if (Action.TargetScheme.IsEmpty())
	{
		OnTargetDataReady.ExecuteIfBound(this);
		return;
	}

	// DA holding context action might get unloaded during the targeting so do a local copy
	CurrentScheme = Action.TargetScheme;
	CurrentSchemeIndex = 0;

	ResetBindings();
	RunTargetingScheme();
}
