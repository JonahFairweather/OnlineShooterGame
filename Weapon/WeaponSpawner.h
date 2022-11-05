// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WeaponSpawner.generated.h"

UCLASS()
class BLASTER_API AWeaponSpawner : public AActor
{
	GENERATED_BODY()
	
public:	
	
	AWeaponSpawner();

protected:
	
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere)
	TArray<TSubclassOf<class AWeapon>> Weapons;

	void SpawnPickup();

	UFUNCTION()
	void StartPickupTimer(AActor* DestroyedWeapon);

	void SpawnTimerFinished();

	
private:
	FTimerHandle RespawnTimer;

	UPROPERTY(EditAnywhere)
	float WeaponTime = 60.f;

	UPROPERTY(EditAnywhere)
	float Variance = 30.f;

	UPROPERTY()
	AWeapon* SpawnedWeapon;

	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* SpawnerMesh;

	UPROPERTY(EditAnywhere)
	class UWidgetComponent* TimerWidget;

	

	



};
