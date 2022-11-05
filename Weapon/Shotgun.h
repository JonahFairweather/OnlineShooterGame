// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HitscanWeapon.h"

#include "Shotgun.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API AShotgun : public AHitscanWeapon
{
	GENERATED_BODY()
public:
	virtual void FireShotgun(const TArray<FVector_NetQuantize>& HitTargets);
	void ShotgunTraceEndWithScatter(const FVector& HitTarget, TArray<FVector_NetQuantize>& HitTargets);

private:
	UPROPERTY(EditAnywhere)
	uint32 NumOfPellets = 10;

	UPROPERTY(EditAnywhere)
	float MaxDamageRange = 500.f;

	UPROPERTY(EditAnywhere)
	float MinDamageEndpoint = 2000.f;
};
