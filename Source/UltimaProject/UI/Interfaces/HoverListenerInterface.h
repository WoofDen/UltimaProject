// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "UObject/Interface.h"
#include "HoverListenerInterface.generated.h"

UINTERFACE()
class UHoverListenerInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 */
class ULTIMAPROJECT_API IHoverListenerInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent)
	void OnHovered();
	
	UFUNCTION(BlueprintNativeEvent)
	void OnUnhovered();
};
