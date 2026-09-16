// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "UObject/Interface.h"
#include "Interactable.generated.h"

UENUM(BlueprintType)
enum class EInteractionType : uint8
{
	Main
};

constexpr ECollisionChannel ECC_InteractableChannel = ECC_GameTraceChannel2;

UINTERFACE()
class UInteractable : public UInterface
{
	GENERATED_BODY()
};

/**
 * Describes an actor(!) user can send an input ( mouse dblcl. ) to activate
 * For chests it will be an attempt to open it or trying to harvest a field crop
 */
class ULTIMAPROJECT_API IInteractable
{
	GENERATED_BODY()

public:
	// Cursor over item
	UFUNCTION(BlueprintNativeEvent)
	void SetFocus(bool Focus, bool IsAccessible);

	// Can we access this object? ( visibility, distance or other obstacles )
	virtual bool IsInteractionAccessible(const AController* Instigator) const;

	virtual void AttemptInteraction(AController* Instigator, EInteractionType Type = EInteractionType::Main);
};
