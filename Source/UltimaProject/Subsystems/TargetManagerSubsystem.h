// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InputMappingContext.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "UltimaProject/Items/Common/ItemData.h"
#include "UltimaProject/UI/HUD/GameplayHUDWidget.h"
#include "TargetManagerSubsystem.generated.h"

UCLASS()
class UTargetingSettingsDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// UPrimaryDataAsset
	static FPrimaryAssetId GetId();
	virtual FPrimaryAssetId GetPrimaryAssetId() const override;
	// ~UPrimaryDataAsset

	UPROPERTY(EditAnywhere)
	UInputMappingContext* TargetingIMC;

	UPROPERTY(EditAnywhere)
	UInputAction* TargetConfirmIA;

	UPROPERTY(EditAnywhere)
	UInputAction* TargetCancelIA;

	bool IsValid() const
	{
		return TargetingIMC
			&& TargetConfirmIA
			&& TargetCancelIA;
	}
};

/**
 * 
 */
UCLASS()
class ULTIMAPROJECT_API UTargetManagerSubsystem : public ULocalPlayerSubsystem
{
	GENERATED_BODY()

	TArray<TWeakObjectPtr<AActor>> SelectedActors;
	TWeakObjectPtr<AUPPlayerController> PlayerController;

	uint32 TargetSelectedBindingHandle = 0;
	uint32 TargetCancelledBindingHandle = 0;

	uint32 CurrentSchemeIndex = 0;
	TArray<TInstancedStruct<FTargetDescriptor>> CurrentScheme;

	bool SetupBindings(TFunction<void()> TargetConfirmCallback);
	void ResetBindings();

	DECLARE_DELEGATE_OneParam(FOnTargetDataReady, UTargetManagerSubsystem*);
	FOnTargetDataReady OnTargetDataReady;

	DECLARE_DELEGATE(FOnTargetCancel)
	FOnTargetCancel OnTargetCancel;

	// Start targeting
	void RunTargetingScheme();

	bool TargetActor();

	void OnTargetConfirmed(AActor* Actor);
	void OnTargetInvalid();
	void OnTargetCancelled();

public:
	// USubsystem
	virtual auto Initialize(FSubsystemCollectionBase& Collection) -> void override;
	virtual void Deinitialize() override;
	virtual void PlayerControllerChanged(APlayerController* NewPlayerController) override;
	// ~USubsystem

	void StartTargetingFromContextAction(const FItemContextAction& Action);
};
