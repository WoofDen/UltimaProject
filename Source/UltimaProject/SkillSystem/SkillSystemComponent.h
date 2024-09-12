// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SkillBase.h"
#include "Skills/SkillWalking.h"
#include "SkillSystemComponent.generated.h"

UENUM(BlueprintType)
enum class ESkill : uint8
{
	NONE,

	Walking,

	MAX
};

inline TMap<ESkill, TSubclassOf<USkillBase>> SkillClasses = {
	{ESkill::Walking, USkillWalking::StaticClass()}
};

ENUM_RANGE_BY_FIRST_AND_LAST(ESkill, ESkill::Walking, ESkill::Walking)

/**
 * 
 */
UCLASS()
class ULTIMAPROJECT_API USkillSystemComponent : public UActorComponent
{
	GENERATED_BODY()

	USkillSystemComponent();

	UPROPERTY()
	TMap<ESkill, TObjectPtr<USkillBase>> Skills;

public:
	USkillBase* GetSkill(const ESkill SkillName) const;

	template <typename T>
	T* GetSkill(const ESkill SkillName) const;

	void AddSkillExp(const ESkill SkillName, const float Value) const
	{
		GetSkill(SkillName)->AddExp(Value);
	}
};

template <typename T>
T* USkillSystemComponent::GetSkill(const ESkill SkillName) const
{
	check(Skills.Contains(SkillName));
	return Cast<T>(Skills[SkillName]);
}
