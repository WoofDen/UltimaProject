// Fill out your copyright notice in the Description page of Project Settings.

#include "Utils.h"

FVector Utils::RandomPointInSquare(const FVector& Center, const FVector2D& Size)
{
	const float HalfSizeX = Size.X * 0.5f;
	const float HalfSizeY = Size.Y * 0.5f;

	const FVector A = Center + FVector(-HalfSizeX, -HalfSizeY, 0.0f);
	const FVector B = Center + FVector(HalfSizeX, -HalfSizeY, 0.0f);
	const FVector C = Center + FVector(HalfSizeX, HalfSizeY, 0.0f);
	const FVector D = Center + FVector(-HalfSizeX, HalfSizeY, 0.0f);

	// Выбираем один из двух треугольников
	const bool bFirstTriangle = FMath::FRand() < 0.5f;

	const FVector& V1 = A;
	const FVector& V2 = bFirstTriangle ? B : C;
	const FVector& V3 = bFirstTriangle ? C : D;

	// Генерируем барицентрические координаты
	const float R1 = FMath::FRand();
	const float R2 = FMath::FRand();

	const float U = FMath::Sqrt(R1);

	const float Lambda1 = 1.0f - U;
	const float Lambda2 = U * (1.0f - R2);
	const float Lambda3 = U * R2;

	return Lambda1 * V1
		+ Lambda2 * V2
		+ Lambda3 * V3;
}
