// Fill out your copyright notice in the Description page of Project Settings.


#include "HordePlayerCharacter.h"

#include "Blaster/AI_BTTasks/AITags.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"
#include "Perception/AISense_Hearing.h"
#include "Perception/AISense_Sight.h"

AHordePlayerCharacter::AHordePlayerCharacter()
{
	SetupStimulus();
	
}

void AHordePlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction("Distract", IE_Pressed, this, &ThisClass::OnDistractButtonPressed);
}

void AHordePlayerCharacter::ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType,
	AController* InstigatorController, AActor* DamageCauser)
{
	Super::ReceiveDamage(DamagedActor, Damage, DamageType, InstigatorController, DamageCauser);

	if(HealthBar)
	{
		HealthBar->HealthBar->SetPercent(GetHealth()/GetMaxHealth());
	}
}

void AHordePlayerCharacter::SetupStimulus()
{
	Stimulus = CreateDefaultSubobject<UAIPerceptionStimuliSourceComponent>(TEXT("Stimulus"));
	Stimulus->RegisterForSense(TSubclassOf<UAISense_Sight>());
	Stimulus->RegisterWithPerceptionSystem();
}

void AHordePlayerCharacter::OnDistractButtonPressed()
{
	if(DistractionSound)
	{
		FVector SoundLocation = GetActorLocation();
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), DistractionSound, SoundLocation);
		UAISense_Hearing::ReportNoiseEvent(GetWorld(), SoundLocation, 1.f, this, 0.f, tags::noise_tag);
	}
}

void AHordePlayerCharacter::Elimmed()
{
	UCharacterMovementComponent* MovementComponent  = GetCharacterMovement();
	if(MovementComponent)
	{
		MovementComponent->StopMovementImmediately();
		MovementComponent->DisableMovement();
		if(GetMesh())
		{
			GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
	}
}
