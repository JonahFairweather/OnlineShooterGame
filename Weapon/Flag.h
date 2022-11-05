// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon.h"
#include "Flag.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API AFlag : public AWeapon
{
	GENERATED_BODY()
private:
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* FlagMesh;

	FTransform InitialTransform;

public:
	AFlag();
	void ResetLocation();
	virtual void Dropped() override;
	virtual void BeginPlay() override;
	
	
protected:
	virtual void HandleWeaponEquipped() override;
    virtual void HandleWeaponDropped() override;
	
    	
};
