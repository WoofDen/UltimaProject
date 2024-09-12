// Fill out your copyright notice in the Description page of Project Settings.


#include "UPBaseAttributeSet.h"

float UUPBaseAttributeSet::GetHealth() const
{
	return FMath::Max(Health.GetCurrentValue(), 0.0f);
}

void UUPBaseAttributeSet::SetHealth(float NewVal)
{
	NewVal = FMath::Max(NewVal, 0.0f);

	UAbilitySystemComponent* ASC = GetOwningAbilitySystemComponent();
	if (ensure(ASC))
	{
		ASC->SetNumericAttributeBase(GetHealthAttribute(), NewVal);
	}
}
