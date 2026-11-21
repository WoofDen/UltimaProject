#pragma once

namespace UPGlobals
{
	constexpr FVector InvalidLocation = FVector(FLT_MAX, UE::Math::TVectorConstInit{});
}

constexpr ECollisionChannel ECC_InteractableChannel = ECC_GameTraceChannel2;