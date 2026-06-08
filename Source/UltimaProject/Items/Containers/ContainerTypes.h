#pragma once

enum class EContainerRelationType : uint8
{
	Invalid = 0 UMETA(Hidden),

	// Player's personal backpack
	Inventory,

	// World actor container
	InWorldContainer
};
