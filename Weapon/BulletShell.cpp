// Fill out your copyright notice in the Description page of Project Settings.


#include "BulletShell.h"
#include "Sound/SoundCue.h"
#include "Kismet/GameplayStatics.h"
// Sets default values
ABulletShell::ABulletShell()
{
 	
	PrimaryActorTick.bCanEverTick = false;
	CasingMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CasingMesh"));
	SetRootComponent(CasingMesh);
	CasingMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	CasingMesh->SetSimulatePhysics(true);
	CasingMesh->SetEnableGravity(true);
	CasingMesh->SetNotifyRigidBodyCollision(true);
	ShellEjectionImpulse = 10.f;
}


void ABulletShell::BeginPlay()
{
	Super::BeginPlay();
	CasingMesh->AddImpulse(GetActorForwardVector() * ShellEjectionImpulse);
	CasingMesh->AddAngularImpulseInDegrees(FVector(FMath::FRandRange(0.f, 20.f), FMath::FRandRange(0.f, 20.f), FMath::FRandRange(0.f, 20.f)));
	
	CasingMesh->OnComponentHit.AddDynamic(this, &ABulletShell::OnHit);
	SetLifeSpan(3.0
	);
	
	
}

void ABulletShell::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (ImpactSound) {
		UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation());
	}
	Destroy();
}


