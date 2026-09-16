// Fill out your copyright notice in the Description page of Project Settings.

#include "InputHelpersFunctionLibrary.h"
#include "Macro.h"

AActor* UInputHelpersFunctionLibrary::GetActorUnderCursor(const APlayerController* PC, ECollisionChannel CollisionChannel)
{
	NULLCHECK_RETURN(PC, nullptr);

	// Client-only
	if (PC->GetNetMode() == NM_DedicatedServer)
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
				CollisionChannel,
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
