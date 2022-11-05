// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blaster/BlasterCharacter.h"
#include "Blaster/HUD/HealthBar.h"
#include "HordePlayerCharacter.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API AHordePlayerCharacter : public ABlasterCharacter
{
	GENERATED_BODY()
public:
	AHordePlayerCharacter();

	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	virtual void ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatorController, AActor* DamageCauser) override;

private:
	class UAIPerceptionStimuliSourceComponent* Stimulus;

	void SetupStimulus();

	void OnDistractButtonPressed();

	UPROPERTY(EditAnywhere, SaveGame)
	USoundBase* DistractionSound;

	UPROPERTY(EditDefaultsOnly, meta = (AllowPrivateAccess = "true"))
	UHealthBar* HealthBar;
	void Elimmed();


	
};
