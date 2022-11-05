// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseAICharacter.h"

#include "Blaster/AIControllers/HordeAIController.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"


ABaseAICharacter::ABaseAICharacter()
	:
WidgetComponent(CreateDefaultSubobject<UWidgetComponent>(TEXT("HealthBar"))),
RightFistHitbox(CreateDefaultSubobject<UBoxComponent>(TEXT("Right Fist Hitbox")))
{
 	
	PrimaryActorTick.bCanEverTick = true;
	if(WidgetComponent)
	{
		WidgetComponent->SetupAttachment(RootComponent);
		WidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
		WidgetComponent->SetRelativeLocation(FVector(0.f, 0.f, 85.f));
		
		static ConstructorHelpers::FClassFinder<UUserWidget> WidgetClass(TEXT("/Game/Blueprints/HUD/WBP_HealthBar"));
		if(WidgetClass.Succeeded())
		{
			WidgetComponent->SetWidgetClass(WidgetClass.Class);
		}
	}

	if(RightFistHitbox)
	{
		FVector const Extent(5.f);

		RightFistHitbox->SetBoxExtent(Extent);

		RightFistHitbox->SetCollisionProfileName("NoCollision");
	}
}


void ABaseAICharacter::UpdateHealthBar()
{
	HealthBar = Cast<UHealthBar>(WidgetComponent->GetUserWidgetObject());
	if(HealthBar && HealthBar->HealthBar)
	{
		HealthBar->HealthBar->SetPercent(Health/MaxHealth);
	}else
	{
		UE_LOG(LogTemp, Warning, TEXT("No health bar"))
	}
}

void ABaseAICharacter::BeginPlay()
{
	Super::BeginPlay();
	UpdateHealthBar();
	if(HasAuthority())
	{
		OnTakeAnyDamage.AddDynamic(this, &ThisClass::ABaseAICharacter::ReceiveDamage);
	}
	if(RightFistHitbox)
	{
		FAttachmentTransformRules const Rules(EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget,
		EAttachmentRule::KeepWorld, false);
		RightFistHitbox->AttachToComponent(GetMesh(), Rules, "RightHandHitbox");
		RightFistHitbox->SetRelativeLocation(FVector(-7.0f, 0.f, 0.f));
		RightFistHitbox->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnAttackOverlap);
		RightFistHitbox->OnComponentEndOverlap.AddDynamic(this, &ThisClass::ABaseAICharacter::OnSphereEndOverlap);
	}
	
}


void ABaseAICharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ABaseAICharacter::ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType,
	AController* InstigatorController, AActor* DamageCauser)
{
	if(DamageCauser == DamagedActor) return;
	UE_LOG(LogTemp, Warning, TEXT("AI took Damage %f"), Damage)
	
	Health -= Damage;
	if(Health <= 0)
	{
		Elimmed();
	}
	UpdateHealthBar();
}


void ABaseAICharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void ABaseAICharacter::EnableBoxCollision(bool bEnable)
{
	if(RightFistHitbox)
	{
		RightFistHitbox->SetCollisionProfileName(bEnable ? "OverlapOnlyPawn" : "NoCollision");
	}
}

bool ABaseAICharacter::MontageFinished()
{
	if(GetMesh() && GetMesh()->GetAnimInstance())
	{
		return GetMesh()->GetAnimInstance()->Montage_GetIsStopped(AttackMontage);
	}
	
	return false;
}

int ABaseAICharacter::MeleeAttack_Implementation()
{
	if(AttackMontage)
	{
		if(AttackMontage->IsValidToPlay())
		{
			PlayAnimMontage(AttackMontage);
		}
	}
	return 0;
}


void ABaseAICharacter::Elimmed()
{
	UCharacterMovementComponent* MovementComponent  = GetCharacterMovement();
	if(MovementComponent)
	{
		MovementComponent->StopMovementImmediately();
		MovementComponent->DisableMovement();
		if(GetMesh())
		{
			//GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			GetMesh()->SetSimulatePhysics(true);
			GetMesh()->SetCollisionProfileName("Ragdoll");
		}
		//GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		GetCapsuleComponent()->SetSimulatePhysics(true);
		GetCapsuleComponent()->SetCollisionProfileName("Ragdoll");
		
	}
	AHordeAIController* AIController = Cast<AHordeAIController>(GetController());
	if(AIController)
	{
		AIController->StopTree();
	}
	if(HealthBar)
	{
		HealthBar->SetVisibility(ESlateVisibility::Hidden);
	}
}

void ABaseAICharacter::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	UE_LOG(LogTemp, Warning, TEXT("AI has hit player"))
	ACharacter* HitCharacter = Cast<ACharacter>(OtherActor);
	if(HitCharacter)
	{
		UGameplayStatics::ApplyDamage(HitCharacter, 20.f, GetController(), this, UDamageType::StaticClass());
	}
}

void ABaseAICharacter::OnAttackOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
}

