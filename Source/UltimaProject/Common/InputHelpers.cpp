// Fill out your copyright notice in the Description page of Project Settings.

#include "InputHelpers.h"

#include "Macro.h"

AActor* InputHelpers::GetActorUnderCursor(const APlayerController* PC)
{
	NULLCHECK_RETURN(PC, nullptr);

	// Client-only
	if (!PC->GetNetMode() == NM_DedicatedServer)
	{
		return nullptr;
	}

	// Trace for an actor under the cursor
	FHitResult HitResult;
	if (ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(PC->Player); LocalPlayer && LocalPlayer->ViewportClient)
	{
		FVector2D MousePosition;
		if (LocalPlayer->ViewportClient->GetMousePosition(MousePosition))
		{
			TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
			ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_WorldDynamic));

			PC->GetHitResultAtScreenPosition(
				MousePosition,
				ObjectTypes,
				false,
				HitResult);
		}
	}

	if (!HitResult.bBlockingHit)
	{
		return nullptr;
	}

	return HitResult.GetActor();
}
