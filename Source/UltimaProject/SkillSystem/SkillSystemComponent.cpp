// Fill out your copyright notice in the Description page of Project Settings.

#include "SkillSystemComponent.h"

USkillSystemComponent::USkillSystemComponent()
{
	for (const ESkill Skill : TEnumRange<ESkill>())
	{
		TSubclassOf<USkillBase> SkillClass = SkillClasses[Skill];
		const FName SkillName = SkillClass->GetDefaultObject<USkillBase>()->GetSystemName();
		TObjectPtr<USkillBase> NewSkill = NewObject<USkillBase>(this, SkillClass, SkillName);
		Skills.Add(Skill, NewSkill);
	}
}

USkillBase* USkillSystemComponent::GetSkill(const ESkill SkillName) const
{
	check(Skills.Contains(SkillName));
	return Skills[SkillName];
}
