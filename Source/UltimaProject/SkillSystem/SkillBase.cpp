// Fill out your copyright notice in the Description page of Project Settings.


#include "SkillBase.h"

DEFINE_LOG_CATEGORY(LogSkillExp);

void USkillBase::LevelUp(const float Value)
{	
	UE_LOG(LogSkillExp, Display, TEXT("%s was improved from %f to %f"), *GetSystemName().ToString(), RawValue,
	       RawValue + Value);
	RawValue += Value;
}

void USkillBase::AddExp(const float Value)
{
	Experience += Value;

	UE_LOG(LogSkillExp, Display, TEXT("%s get %f exp."), *GetSystemName().ToString(), Value);

	if (Experience > 100)
	{
		LevelUp(1);
		Experience = 0;
	}
}
