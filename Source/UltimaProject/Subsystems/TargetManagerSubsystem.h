// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "InputMappingContext.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "UltimaProject/Items/Common/ItemData.h"
#include "UltimaProject/Items/ContextActions/TargetDescriptors.h"
#include "UltimaProject/UI/HUD/GameplayHUDWidget.h"
#include "StructUtils/InstancedStruct.h"
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
	UTargetingSettingsDataAsset* GetTargetingSettings() const;

	// Start targeting
	void RunTargetingScheme();

	bool TargetActor();

	void OnTargetConfirmed(AActor* Actor);
	void OnTargetInvalid();
	void OnTargetCancelled();

public:
	DECLARE_DELEGATE(FOnTargetDataReady);
	FOnTargetDataReady OnTargetDataReady;

	DECLARE_DELEGATE(FOnTargetCancel)
	FOnTargetCancel OnTargetCancel;

	// USubsystem
	virtual auto Initialize(FSubsystemCollectionBase& Collection) -> void override;
	virtual void Deinitialize() override;
	virtual void PlayerControllerChanged(APlayerController* NewPlayerController) override;
	// ~USubsystem

	void StartTargetingScheme(const TArray<TInstancedStruct<FTargetDescriptor>>& Scheme);
	void StopTargeting();
};
