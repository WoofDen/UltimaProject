// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UltimaProject/SkillSystem/SkillBase.h"
#include "UObject/Object.h"
#include "SkillWalking.generated.h"

/**
 * 
 */
UCLASS()
class ULTIMAPROJECT_API USkillWalking : public USkillBase
{
	GENERATED_BODY()

public:
	virtual FName GetSystemName() override;
};
