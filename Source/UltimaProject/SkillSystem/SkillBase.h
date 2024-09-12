// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "SkillBase.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSkillExp, Display, All);


/**
 * 
 */
UCLASS(Abstract)
class ULTIMAPROJECT_API USkillBase : public UObject
{
	GENERATED_BODY()

	friend class USkillSystemComponent;
private:
	// Value that belongs to the holder
	float RawValue = 0;

	// Sum of modifications
	float ModValue = 0;

	float Experience = 0;

	void LevelUp(const float Value);

	UFUNCTION(BlueprintCallable)
	virtual float GetRawValue() const { return RawValue; }

	UFUNCTION(BlueprintCallable)
	virtual float GetModValue() const { return ModValue; }

	UFUNCTION(BlueprintCallable)
	virtual float GetValue() const { return RawValue + ModValue; }

	virtual void AddExp(const float Value);

	virtual FName GetSystemName() { return NAME_None; };
};
