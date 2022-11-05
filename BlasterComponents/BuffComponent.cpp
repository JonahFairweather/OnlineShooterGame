// Fill out your copyright notice in the Description page of Project Settings.


#include "BuffComponent.h"

#include "Blaster/BlasterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Math/UnitConversion.h"

// Sets default values for this component's properties
UBuffComponent::UBuffComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	
}



void UBuffComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UBuffComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	bHealing = AmountToHeal > 0.f;
	HealRampUp(DeltaTime);
	ShieldRampUp(DeltaTime);
}

void UBuffComponent::Heal(float HealAmount, float HealingTime)
{
	bHealing = true;
	AmountToHeal += HealAmount;
	HealingRate = AmountToHeal / HealingTime;
}

void UBuffComponent::ReplenishShield(float ShieldAmount, float ShieldTime)
{
	bShielding = true;
	ShieldPool += ShieldAmount;
	ShieldingRate = ShieldPool/ShieldTime;
}

void UBuffComponent::BuffSpeed(float BaseWalkSpeed, float BaseCrouchSpeed, float Duration)
{
	if(Character == nullptr) return;
	
	if(Character->GetCharacterMovement())
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;
		Character->GetCharacterMovement()->MaxWalkSpeedCrouched = BaseCrouchSpeed;
		if(Character->GetCombatComponent())
		{
			Character->GetCombatComponent()->BaseWalkSpeed = BaseWalkSpeed;
		}
	}
	MulticastSpeedBuff(BaseWalkSpeed, BaseCrouchSpeed);
	if(Character->GetWorldTimerManager().IsTimerActive(SpeedBuffTimer))
	{
		float TimeRemaining = Character->GetWorldTimerManager().GetTimerRemaining(SpeedBuffTimer);
		Character->GetWorldTimerManager().ClearTimer(SpeedBuffTimer);
		Character->GetWorldTimerManager().SetTimer(SpeedBuffTimer, this, &UBuffComponent::ResetSpeeds, Duration + TimeRemaining);
	}else
	{
		Character->GetWorldTimerManager().SetTimer(SpeedBuffTimer, this, &UBuffComponent::ResetSpeeds, Duration);
	}
	
	
}

void UBuffComponent::SetInitialSpeeds(float WalkSpeed, float CrouchSpeed)
{
	InitialBaseSpeed = WalkSpeed;
	InitialCrouchSpeed = CrouchSpeed;
}

void UBuffComponent::SetInitialJumpVelocity(float JumpVelocity)
{
	InitialJumpVelocity = JumpVelocity;
}

void UBuffComponent::ChangeFireRateFactor(float FactorMultiplier, float Duration)
{
	UE_LOG(LogTemp, Warning, TEXT("Changing Fire Rate factor by %f"), FactorMultiplier)
	if(Character == nullptr || Character->GetCombatComponent() == nullptr) return;
	UCombatComponent* Combat = Character->GetCombatComponent();
	if(Combat)
	{
		Combat->FireRateFactor = Combat->FireRateFactor * FactorMultiplier;
	}

	if(Duration <= 0.f) return;
	FTimerDelegate ElimMsgDelegate;
	ElimMsgDelegate.BindUFunction(this, FName("ChangeFireRateFactor"), 1.f/FactorMultiplier, 0.f);
	Character->GetWorldTimerManager().SetTimer(FireRateTimerHandle, ElimMsgDelegate, Duration, false);
}

void UBuffComponent::BuffJump(float Duration, float ZVelocity)
{
	if(Character == nullptr || Character->GetMovementComponent() == nullptr) return;
	Character->GetWorldTimerManager().SetTimer(JumpTimerHandle, this, &UBuffComponent::ResetJump, Duration);
	Character->GetCharacterMovement()->JumpZVelocity = ZVelocity;
	MulticastJumpBuff(ZVelocity);
}

void UBuffComponent::ResetSpeeds()
{
	if(Character && Character->GetCharacterMovement())
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = InitialBaseSpeed;
		Character->GetCharacterMovement()->MaxWalkSpeedCrouched = InitialCrouchSpeed;
		if(Character->GetCombatComponent())
		{
			Character->GetCombatComponent()->BaseWalkSpeed = InitialBaseSpeed;
		}
	}
	MulticastSpeedBuff(InitialBaseSpeed, InitialCrouchSpeed);
}

void UBuffComponent::ResetJump()
{
	if(Character && Character->GetCharacterMovement())
	{
		Character->GetCharacterMovement()->JumpZVelocity = InitialJumpVelocity;
	}
	MulticastJumpBuff(InitialJumpVelocity);
}

void UBuffComponent::MulticastJumpBuff_Implementation(float ZVelocity)
{
	if(Character && Character->GetCharacterMovement())
	{
		Character->GetCharacterMovement()->JumpZVelocity = ZVelocity;
	}
}

void UBuffComponent::MulticastSpeedBuff_Implementation(float BaseSpeed, float CrouchSpeed)
{
	if(Character && Character->GetCharacterMovement())
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = BaseSpeed;
		Character->GetCharacterMovement()->MaxWalkSpeedCrouched = CrouchSpeed;
		if(Character->GetCombatComponent())
		{
			Character->GetCombatComponent()->BaseWalkSpeed = BaseSpeed;
		}
	}
}

void UBuffComponent::HealRampUp(float DeltaTime)
{
	if(!bHealing || Character == nullptr || Character->IsElimmed()) return;
	const float HealThisFrame = HealingRate * DeltaTime;
	Character->HealCharacter(HealThisFrame);
	Character->SetHudHealth();
	AmountToHeal-=HealThisFrame;

	if(AmountToHeal <= 0.f || Character->GetHealth() >= Character->GetMaxHealth())
	{
		bHealing = false;
		AmountToHeal = 0.f;
	}
	
}

void UBuffComponent::ShieldRampUp(float DeltaTime)
{
	if(!bShielding || Character == nullptr || Character->IsElimmed()) return;
	const float ShieldThisFrame = ShieldingRate * DeltaTime;
	Character->SetSheild(FMath::Clamp(Character->GetShield() + ShieldThisFrame, 0.f, Character->GetMaxShield()));
	Character->SetHudShield();
	ShieldPool-=ShieldThisFrame;

	if(ShieldPool <= 0.f || Character->GetShield() >= Character->GetMaxShield())
	{
		bShielding = false;
		ShieldPool = 0.f;
	}
}

