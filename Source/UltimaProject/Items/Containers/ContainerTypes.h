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
enum class ESubstance : uint8
{
	Water = 0,

	Wart = 10
};

USTRUCT(BlueprintType)
struct FLiquidDescriptor
{
	GENERATED_BODY()

	virtual ~FLiquidDescriptor() = default;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	ESubstance Type;

	// 256 bit
	UPROPERTY(EditAnywhere)
	uint64 MetaFlags = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UEnum> MetaFlagsEnum;

	virtual uint32 GetTypeHash() const
	{
		const uint32 EnumHash = MetaFlagsEnum.GetPtrTypeHash();
		const uint32 TypeHash = static_cast<uint8>(Type);
		const uint32 FlagsHash = static_cast<uint32>(MetaFlags);

		return HashCombine(EnumHash, TypeHash, FlagsHash);
	}

	FLiquidDescriptor()
	{
		Type = ESubstance::Water;
	};

	FLiquidDescriptor(ESubstance InType)
		: Type(InType)
	{
	}

	bool operator==(const FLiquidDescriptor& Other) const
	{
		return Type == Other.Type && MetaFlags == Other.MetaFlags;
	}

	template <typename EnumType>
	void SetMeta(EnumType MetaFlag, bool Value)
	{
		static_assert(std::is_enum_v<EnumType>);

		const uint64 Bit = uint64(1) << static_cast<uint8>(MetaFlag);
		checkSlow(static_cast<uint64>(MetaFlag) < 64);

		if (Value)
		{
			MetaFlags |= Bit;
		}
		else
		{
			MetaFlags &= ~Bit;
		}
	}

	template <typename EnumType>
	bool GetMeta(EnumType MetaFlag) const
	{
		static_assert(std::is_enum_v<EnumType>);
		checkSlow(static_cast<uint64>(MetaFlag) < 64);

		return (MetaFlags & (uint64(1) << static_cast<uint8>(MetaFlag))) != 0;
	}
};

inline uint32 GetTypeHash(const FLiquidDescriptor& Item)
{
	return Item.GetTypeHash();
}

namespace UP::Liquids
{
	const FLiquidDescriptor Water = FLiquidDescriptor{ESubstance::Water};
}
