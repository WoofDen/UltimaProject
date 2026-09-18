// Fill out your copyright notice in the Description page of Project Settings.

#include "CropBase.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Net/UnrealNetwork.h"
#include "UltimaProject/Common/Macro.h"
#include "UltimaProject/Common/Utils.h"

const UPlantCultureDataAsset* ACropBase::GetCultureDataAsset() const
{
	return CultureDataAsset;
}

ACropBase::ACropBase()
{
	PrimaryActorTick.bCanEverTick = false;
	NetPriority = .1f;

	CropsISM = CreateDefaultSubobject<UInstancedStaticMeshComponent>(FName("CropsISM"), true);
	CropsISM->SetIsReplicated(false);

	// Grass, who cares
	SetNetUpdateFrequency(1.f);
	bReplicates = true;
}

void ACropBase::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		IHeartbeatInterface::Register();
	}
	else
	{
		// Setup visuals
		OnProgressUpdated(0.f);
	}

	CurrentCycleTime = 0.f;
}

void ACropBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (HasAuthority())
	{
		IHeartbeatInterface::Unregister();
	}

	Super::EndPlay(EndPlayReason);
}

void ACropBase::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ACropBase, GrowProgress);
}

void ACropBase::Heartbeat_Implementation(int64 CurrentTime, int32 TimePassed)
{
	IHeartbeatInterface::Heartbeat_Implementation(CurrentTime, TimePassed);

	if (const UPlantCultureDataAsset* Data = GetCultureDataAsset())
	{
		CurrentCycleTime += TimePassed;
		SetProgress(static_cast<float>(CurrentCycleTime) / Data->GrowTime);
	}
}

void ACropBase::OnRep_GrowProgress(float PrevValue)
{
	OnProgressUpdated(PrevValue);
}

void ACropBase::UpdateProgressVisuals(const FPlantCultureGrowVisuals& VisualsData)
{
	ensureAlways(!HasAuthority());
	NULLCHECK(CropsISM);

	const UPlantCultureDataAsset* Data = GetCultureDataAsset();
	NULLCHECK(Data);

	// Update SM
	bool bSMChanged = false;
	if (ensureAlways(VisualsData.StaticMesh) && CropsISM->GetStaticMesh() != VisualsData.StaticMesh)
	{
		CropsISM->SetStaticMesh(VisualsData.StaticMesh);
		bSMChanged = true;
	}

	// Adjust required amount of instances
	int32 CurrentAmount = CropsISM->GetInstanceCount();
	if (CurrentAmount > VisualsData.CountRange.GetMax())
	{
		// Remove instances
		int32 TargetAmount = FMath::RandRange(VisualsData.CountRange.GetMin(), VisualsData.CountRange.GetMax());
		uint32 AmountToRemove = CurrentAmount - TargetAmount;
		for (uint32 i = 0; i < AmountToRemove; i++)
		{
			CropsISM->RemoveInstance(CurrentAmount - i - 1);
		}
	}
	else if (CurrentAmount < VisualsData.CountRange.GetMin())
	{
		// Add instances
		int32 TargetAmount = FMath::RandRange(VisualsData.CountRange.GetMin(), VisualsData.CountRange.GetMax());
		int32 InstancesToAdd = TargetAmount - CurrentAmount;

		for (int32 i = 0; i < InstancesToAdd; i++)
		{
			FTransform Transform = FTransform::Identity;
			Transform.SetLocation(Utils::RandomPointInSquare(FVector::ZeroVector, FVector2d(Data->FieldSize)));

			FQuat Rotation = FRotator(0.f, FMath::RandRange(0, 180), 0.f).Quaternion();
			Transform.SetRotation(Rotation);

			CropsISM->AddInstance(Transform, false);
		}
	}

	CurrentAmount = CropsISM->GetInstanceCount();
	TArray<FTransform> NewTransforms;
	for (int32 i = 0; i < CurrentAmount; i++)
	{
		FTransform Transform;
		CropsISM->GetInstanceTransform(i, Transform);

		// Update scale if needed
		if (bSMChanged
			|| Transform.GetScale3D().X < VisualsData.ScaleRange.GetMin()
			|| Transform.GetScale3D().X > VisualsData.ScaleRange.GetMax())
		{
			const float Scale = FMath::RandRange(VisualsData.ScaleRange.GetMin(), VisualsData.ScaleRange.GetMax());
			Transform.SetScale3D(FVector(Scale));
		}

		// If SM was changed, re-rotate everything
		if (bSMChanged)
		{
			FQuat Rotation = FRotator(0.f, FMath::RandRange(0, 180), 0).Quaternion();
			Transform.SetRotation(Rotation);
		}

		NewTransforms.Add(MoveTemp(Transform));
	}

	CropsISM->BatchUpdateInstancesTransforms(0, NewTransforms);
}

void ACropBase::SetProgress(float NewProgress)
{
	ensureAlways(HasAuthority());
	float PreValue = GrowProgress;
	GrowProgress = FMath::Clamp(NewProgress, 0.f, 1.f);

	OnProgressUpdated(PreValue);
}

void ACropBase::OnProgressUpdated_Implementation(float PrevValue)
{
	if (HasAuthority())
	{
		return;
	}

	// Client-side visual update
	// Check if the progress has passed a milestone value
	const UPlantCultureDataAsset* CultureData = GetCultureDataAsset();
	NULLCHECK(CultureData);

	// Ensure default is present
	if (CultureData->ProgressVisuals.IsEmpty())
	{
		return;
	}

	TArray<float> Milestones;
	CultureData->ProgressVisuals.GenerateKeyArray(Milestones);
	Milestones.Sort();

	// Map progress to milestone range
	// Ex.: 0, 0.6, 1 -> means ranges [0-0.6) -> 0, [0.6-1) -> 0.6, [1] -> 1
	float TargetMilestone = Milestones.Last();

	for (int32 i = 0; i < Milestones.Num(); i++)
	{
		if (GrowProgress >= Milestones[i])
		{
			TargetMilestone = Milestones[i];
		}
	}

	if (TargetMilestone != CurrentVisualsMilestone)
	{
		UpdateProgressVisuals(CultureData->ProgressVisuals[TargetMilestone]);
		CurrentVisualsMilestone = TargetMilestone;
	}
}
