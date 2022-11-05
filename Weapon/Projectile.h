// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Projectile.generated.h"

class UNiagaraComponent;
UCLASS()
class BLASTER_API AProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	
	AProjectile();

	virtual void Destroyed() override;
	

	/*
	 * Used with server-side rewind
	 */

	bool bUseServerSideRewind;
	FVector_NetQuantize InitialLocation;
	FVector_NetQuantize100 InitialVelocity;

	UPROPERTY(EditAnywhere)
	float InitialSpeed = 15000;

	//Only set for grenades and rockets
	UPROPERTY(EditAnywhere)
	float damage = 10.f;

	// Doesn't matter if you set at all
	UPROPERTY(EditAnywhere)
	float HeadshotDamage = 30.f;

protected:
	
	virtual void BeginPlay() override;

	void StartDestroyTimer();

	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	UPROPERTY(EditAnywhere)
	class UParticleSystem* ImpactParticles;

	UPROPERTY(EditAnywhere)
	class USoundCue* ImpactSound;

	UPROPERTY(EditAnywhere)
	class UBoxComponent* CollisionBox;

	UPROPERTY(VisibleAnywhere)
	class UProjectileMovementComponent* ProjectileMovementComponent;

	UPROPERTY(EditAnywhere)
	class UNiagaraSystem* TrailSystem;

	UPROPERTY()
	UNiagaraComponent* TrailSystemComponent;

	void SpawnTrailSystem();

	void DestroyTimerFinished();
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* ProjectileMesh;

	void ExplodeDamage();

	UPROPERTY(EditAnywhere)
	float DamageInnerRadius = 200.f;

	UPROPERTY(EditAnywhere)
	float DamageOuterRadius = 500.f;

	


private:
	UPROPERTY(EditAnywhere)
	class UParticleSystem* Tracer;

	class UParticleSystemComponent* TracerComponent;

	UPROPERTY(EditAnywhere)
	float DestroyTime = 3.f;

	FTimerHandle DestroyTimer;

	

	

	

public:	
	
	virtual void Tick(float DeltaTime) override;

};
