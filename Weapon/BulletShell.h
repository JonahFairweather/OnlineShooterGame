// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BulletShell.generated.h"

UCLASS()
class BLASTER_API ABulletShell : public AActor
{
	GENERATED_BODY()
	
public:	
	
	ABulletShell();

protected:
	
	virtual void BeginPlay() override;

	UFUNCTION()
		virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

public:	
	
private:
	UPROPERTY(VisibleAnywhere, Category = "Casing Skin")
	UStaticMeshComponent* CasingMesh;

	UPROPERTY(EditAnywhere)
		float ShellEjectionImpulse;

	UPROPERTY(EditAnywhere)
		class USoundCue* ImpactSound;
};
