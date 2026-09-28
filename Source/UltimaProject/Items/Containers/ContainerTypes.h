#pragma once

#include "ContainerTypes.generated.h"

enum class EContainerRelationType : uint8
{
	Invalid = 0 UMETA(Hidden),

	// Player's personal backpack
	Inventory,

	// World actor container
	InWorldContainer,

	// One-time disposable container
	Disposable
};


UENUM(BlueprintType)
enum class ELiquid : uint8
{
	Water = 0,

	Wart = 10
};

USTRUCT(BlueprintType)
struct FLiquidDescriptor
{
	GENERATED_BODY()

	UPROPERTY()
	ELiquid Type;

	// 256 bit
	UPROPERTY()
	uint64 MetaFlags = 0;

	FLiquidDescriptor()
	{
		Type = ELiquid::Water;
	};

	FLiquidDescriptor(ELiquid InType)
		: Type(InType)
	{
	}

	template <typename EnumType>
	void SetMeta(EnumType MetaFlag, bool Value)
	{
		static_assert(std::is_enum_v<EnumType> == true);
		static_assert(sizeof(EnumType) == 255);

		uint8 IntValue = static_cast<uint8>(MetaFlag);

		if (Value)
		{
			MetaFlags = MetaFlags | 1 << IntValue;
		}
		else
		{
			MetaFlags = MetaFlags & ~(1 << IntValue);
		}
	}

	template <typename EnumType>
	bool GetMeta(EnumType MetaFlag)
	{
		static_assert(std::is_enum_v<EnumType> == true);
		static_assert(sizeof(EnumType) == 255);

		uint8 IntValue = static_cast<uint8>(MetaFlag);
		return MetaFlags & (1 << IntValue);
	}
};

namespace UP::Liquids
{
	const FLiquidDescriptor Water = FLiquidDescriptor{ELiquid::Water};
}
