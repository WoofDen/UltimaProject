// Fill out your copyright notice in the Description page of Project Settings.

#include "TargetManagerSubsystem.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/AssetManager.h"
#include "UltimaProject/Common/InputHelpersFunctionLibrary.h"
#include "UltimaProject/Common/Macro.h"
#include "UltimaProject/Items/ActionProcessors/ContextActionHandler.h"
#include "UltimaProject/Items/ContextActions/TargetDescriptors.h"
#include "UltimaProject/UI/HUD/CursorWidget.h"

class UEnhancedInputLocalPlayerSubsystem;

FPrimaryAssetId UTargetingSettingsDataAsset::GetId()
{
	return FPrimaryAssetId("TargetingSettings", "TargetingSettings");
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

	UTargetingSettingsDataAsset* Settings = GetTargetingSettings();
	NULLCHECK_RETURN(Settings, false);

	// Setup inputs
	// Main input mapping context
	EnhancedInputSubsystem->AddMappingContext(Settings->TargetingIMC, 10);

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

		// Remove targeting IMC
		UEnhancedInputLocalPlayerSubsystem* EnhancedInputSubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer());
		UTargetingSettingsDataAsset* Settings = GetTargetingSettings();

		if (EnhancedInputSubsystem && Settings)
		{
			EnhancedInputSubsystem->RemoveMappingContext(Settings->TargetingIMC);
		}
	}
}

UTargetingSettingsDataAsset* UTargetManagerSubsystem::GetTargetingSettings() const
{
	UAssetManager& AssetManager = UAssetManager::Get();

	// Load if not in memory
	const FPrimaryAssetId PrimaryAssetId = UTargetingSettingsDataAsset::GetId();
	if (TSharedPtr<FStreamableHandle> StreamHandle = AssetManager.LoadPrimaryAsset(PrimaryAssetId))
	{
		StreamHandle->WaitUntilComplete();
	}

	return UAssetManager::Get().GetPrimaryAssetObject<UTargetingSettingsDataAsset>(PrimaryAssetId);
}

void UTargetManagerSubsystem::RunTargetingScheme()
{
	if (!CurrentScheme.IsValidIndex(CurrentSchemeIndex))
	{
		OnTargetDataReady.ExecuteIfBound();
		return;
	}

	const auto TargetSchemeRow = CurrentScheme[CurrentSchemeIndex];
	if (TargetSchemeRow.GetScriptStruct() == FTargetDescriptor_Actor::StaticStruct())
	{
		UGameplayHUDWidget::GameLog(PlayerController.Get(), TargetSchemeRow->StartedMessage, false, UP::LogStyle::Default);

		TargetActor();
	}
	else
	{
		checkNoEntry();
	}
}

bool UTargetManagerSubsystem::TargetActor()
{
	UTargetingSettingsDataAsset* Settings = GetTargetingSettings();
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
		OnTargetDataReady.ExecuteIfBound();
		return;
	}

	const auto TargetSchemeRow = CurrentScheme[CurrentSchemeIndex];
	bool bTargetIsValid = false;
	if (TargetSchemeRow.GetScriptStruct() == FTargetDescriptor_Actor::StaticStruct())
	{
		// Validate actor
		const FTargetDescriptor_Actor& TargetActorDescriptor = TargetSchemeRow.Get<FTargetDescriptor_Actor>();

		if (TargetActorDescriptor.Filters.IsEmpty())
		{
			bTargetIsValid = true;
			SelectedActors.Add(Actor);
		}
		else
		{
			for (const TSubclassOf<UObject> FilterClass : TargetActorDescriptor.Filters)
			{
				bool bMatch = false;
				if (FilterClass->IsInA(AActor::StaticClass()))
				{
					bMatch = Actor->IsA(FilterClass);
				}
				else if (FilterClass->IsChildOf(UActorComponent::StaticClass()))
				{
					bMatch = Actor->GetComponentByClass(FilterClass.Get()) != nullptr;
				}
				
				if (!bMatch)
				{
					continue;
				}

				bTargetIsValid = true;
				SelectedActors.Add(Actor);
				break;
			}
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
	UGameplayHUDWidget::GameLog(PlayerController.Get(), TEXT("Invalid target."), false, UP::LogStyle::Default);

	StopTargeting();
	OnTargetCancel.ExecuteIfBound();
}

void UTargetManagerSubsystem::OnTargetCancelled()
{
	StopTargeting();
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

void UTargetManagerSubsystem::StartTargetingScheme(const TArray<TInstancedStruct<FTargetDescriptor>>& Scheme)
{
	if (Scheme.IsEmpty())
	{
		OnTargetDataReady.ExecuteIfBound();
		return;
	}

	// Copy
	CurrentScheme = Scheme;
	CurrentSchemeIndex = 0;

	ResetBindings();
	RunTargetingScheme();
}

void UTargetManagerSubsystem::StopTargeting()
{
	// Reset keybindings
	ResetBindings();

	// Reset cursor
	if (PlayerController.IsValid())
	{
		if (UCursorWidget* CursorWidget = PlayerController->GetCursorWidget())
		{
			CursorWidget->SetTargetMode(false);
		}
	}
}

AActor* UTargetManagerSubsystem::GetSelectedActor(const int32 Index)
{
	if (SelectedActors.IsValidIndex(Index))
	{
		const TWeakObjectPtr<AActor> ActorWeakPtr = SelectedActors[Index];
		return ActorWeakPtr.Get();
	}
	
	return nullptr;
}
