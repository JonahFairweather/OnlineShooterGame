// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Blaster/BlasterTypes/Team.h"
#include "EndZone.generated.h"

UCLASS()
class BLASTER_API AEndZone : public AActor
{
	GENERATED_BODY()
	
public:	
	
	AEndZone();

	UPROPERTY(EditAnywhere)
	ETeam Team;

protected:
	
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, 
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

public:

private:
	UPROPERTY(EditAnywhere)
	class USphereComponent* ZoneSphere;

	


};
