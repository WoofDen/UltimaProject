// Fill out your copyright notice in the Description page of Project Settings.

// Game includes
#include "UPPlayerState.h"
#include "UltimaProject/GAS/UPAbilitySystemComponent.h"
#include "UltimaProject/GAS/Attributes/UPBaseAttributeSet.h"
#include "UltimaProject/SkillSystem/SkillSystemComponent.h"

// Engine includes
#include "GameplayAbilitySet.h"

void AUPPlayerState::OnPawnChanged(APlayerState* Player, APawn* NewPawn, APawn* OldPawn)
{
	UpdateAbilitySystemComponent();
}

void AUPPlayerState::UpdateAbilitySystemComponent()
{
	if (AbilitySystemComponent)
	{
		APawn* Pawn = GetPawn();
		AbilitySystemComponent->InitAbilityActorInfo(this, Pawn);

		// If Pawn is not null, PC is exptexted to be set
		ensureAlways(!Pawn || AbilitySystemComponent->AbilityActorInfo->PlayerController.IsValid());
	}
}

AUPPlayerState::AUPPlayerState()
{
	// GAS
	AbilitySystemComponent = CreateDefaultSubobject<UUPAbilitySystemComponent>("AbilitySystemComponent");

	// Skills
	SkillSystemComponent = CreateDefaultSubobject<USkillSystemComponent>("SkillSystemComponent");
}

void AUPPlayerState::BeginPlay()
{
	Super::BeginPlay();

	if (APawn* Pawn = GetPawn(); Pawn && Pawn->GetController())
	{
		UpdateAbilitySystemComponent();
	}
	else
	{
		OnPawnSet.AddDynamic(this, &ThisClass::OnPawnChanged);
	}
}

void AUPPlayerState::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	OnPawnSet.RemoveDynamic(this, &ThisClass::OnPawnChanged);
}

void AUPPlayerState::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->AddSet<UUPBaseAttributeSet>();

		if (HasAuthority() && DefaultAbilitySet)
		{
			DefaultAbilitySet->GiveAbilities(AbilitySystemComponent);
		}
	}
}

UAbilitySystemComponent* AUPPlayerState::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}
