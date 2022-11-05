// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponSpawner.h"

#include "Weapon.h"
#include "Components/WidgetComponent.h"


AWeaponSpawner::AWeaponSpawner()
{
 
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	SpawnerMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SpawnerMesh"));
	SpawnerMesh->SetupAttachment(RootComponent);
	SpawnerMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	TimerWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("PikcupWidget"));
	TimerWidget->SetupAttachment(RootComponent);
	
}


void AWeaponSpawner::BeginPlay()
{
	Super::BeginPlay();
	StartPickupTimer((AActor*) nullptr);
}

void AWeaponSpawner::SpawnPickup()
{
	int32 NumPickups = Weapons.Num();
	if(NumPickups > 0)
	{
		int32 selection = FMath::RandRange(0, NumPickups-1);
		SpawnedWeapon = GetWorld()->SpawnActor<AWeapon>(Weapons[selection], GetActorLocation() + FVector(0.f, 0.f, 50.f), GetActorRotation() - FRotator(180.f, 90.f, 90.f));
		if(HasAuthority() && SpawnedWeapon)
		{
			SpawnedWeapon->bDestroyWeapon = true;
			SpawnedWeapon->SetWeaponState(EWeaponState::EWS_Initial);
			UE_LOG(LogTemp, Warning, TEXT("Binding Pickup Timer to spawned weapon"))
			SpawnedWeapon->OnDestroyed.AddDynamic(this, &AWeaponSpawner::StartPickupTimer);
		}
	}
}

void AWeaponSpawner::StartPickupTimer(AActor* DestroyedWeapon)
{
	TimerWidget->SetVisibility(true);
	UE_LOG(LogTemp, Warning, TEXT("Starting respawn Timer"))
	const float SpawnTime = FMath::Clamp(FMath::RandRange(WeaponTime - Variance, WeaponTime + Variance), 1.f, 1000.f);
	GetWorldTimerManager().SetTimer(RespawnTimer,
		this,
		&AWeaponSpawner::SpawnTimerFinished,
		SpawnTime);
}

void AWeaponSpawner::SpawnTimerFinished()
{
	if(HasAuthority())
	{
		SpawnPickup();
	}
}


