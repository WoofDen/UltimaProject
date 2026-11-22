// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ItemData.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ItemsFunctionLibrary.generated.h"

struct FItemData;
/**
 * 
 */
UCLASS()
class ULTIMAPROJECT_API UItemsFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, BlueprintPure)
	static float GetItemProp(UPARAM(ref) const FItemData& ItemData, EItemIntProperty Property);
	
	UFUNCTION(BlueprintCallable)
	static void SetItemProp(UPARAM(ref) FItemData& ItemData, EItemIntProperty Property, float Value);
	
	UFUNCTION(BlueprintCallable)
	static void AddItemProp(UPARAM(ref) FItemData& ItemData, EItemIntProperty Property, float Value);
};
