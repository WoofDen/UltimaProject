// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "PlantCultureDataAsset.h"
#include "GameFramework/Actor.h"
#include "UltimaProject/World/HeartbeatSystem/HeartbeatInterface.h"
#include "CropBase.generated.h"

UCLASS(Abstract, Blueprintable, BlueprintType)
class ULTIMAPROJECT_API ACropBase : public AActor, public IHeartbeatInterface
{
	GENERATED_BODY()

	int64 CurrentCycleTime;
	
	float CurrentVisualsMilestone = -1.f;

protected:
	UPROPERTY(VisibleAnywhere, ReplicatedUsing=OnRep_GrowProgress)
	float GrowProgress = 0.f;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UInstancedStaticMeshComponent> CropsISM;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UPlantCultureDataAsset> CultureDataAsset;
	
	UFUNCTION()
	void OnRep_GrowProgress(float PrevProgress);
	
	void UpdateProgressVisuals(const FPlantCultureGrowVisuals& VisualsData);

	void SetProgress(float NewProgress);
	
	UFUNCTION(BlueprintNativeEvent)
	void OnProgressUpdated(float PrevValue);
	
	const UPlantCultureDataAsset* GetCultureDataAsset() const;

public:
	ACropBase();

	// ĄActor
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	// ~AActor
	
	// IHeartbeatInterface
	virtual void Heartbeat_Implementation(int64 CurrentTime, int32 TimePassed) override;
	// ~IHeartbeatInterface
};
