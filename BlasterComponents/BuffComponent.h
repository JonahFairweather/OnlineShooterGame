// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BuffComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BLASTER_API UBuffComponent : public UActorComponent
{
	GENERATED_BODY()
	friend class ABlasterCharacter;
public:	
	// Sets default values for this component's properties
	UBuffComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	void HealRampUp(float DeltaTime);

	void ShieldRampUp(float DeltaTime);

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY()
	class ABlasterCharacter* Character;

	void Heal(float HealAmount, float HealingTime);

	void ReplenishShield(float ShieldAmount, float ShieldTime);

	void BuffSpeed(float BaseWalkSpeed, float BaseCrouchSpeed, float Duration);

	void SetInitialSpeeds(float WalkSpeed, float CrouchSpeed);

	void BuffJump(float Duration, float ZVelocity);

	void SetInitialJumpVelocity(float JumpVelocity);

	UFUNCTION()
	void ChangeFireRateFactor(float FactorMultiplier, float Duration);
	
		

private:

	/*
	 * Healing Buff
	 */
	bool bHealing = false;
	float HealingRate = 0.f;
	float AmountToHeal = 0.f;

	/*
	 * Speed Buff
	 */

	FTimerHandle SpeedBuffTimer;

	void ResetSpeeds();

	float InitialBaseSpeed;

	float InitialCrouchSpeed;

	UFUNCTION(NetMulticast, Reliable)
	void MulticastSpeedBuff(float BaseSpeed, float CrouchSpeed);

	/*
	 * Jump Buff 
	 */

	FTimerHandle JumpTimerHandle;

	float InitialJumpVelocity;

	void ResetJump();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastJumpBuff(float ZVelocity);

	/*
	 * Sheild buff
	 */

	bool bShielding = false;
	float ShieldingRate = 0.f;
	float ShieldPool = 0.f;

	/*
	 * Fire rate buff
	 */

	

	FTimerHandle FireRateTimerHandle;
		
};
