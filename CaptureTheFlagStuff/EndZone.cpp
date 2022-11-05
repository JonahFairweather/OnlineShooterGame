// Fill out your copyright notice in the Description page of Project Settings.


#include "EndZone.h"

#include "Blaster/BlasterCharacter.h"
#include "Blaster/GameMode/CTFGameMode.h"
#include "Blaster/Weapon/Flag.h"
#include "Components/SphereComponent.h"

AEndZone::AEndZone()
{
 	
	PrimaryActorTick.bCanEverTick = false;
	ZoneSphere = CreateDefaultSubobject<USphereComponent>(TEXT("Flag Zone"));
	ZoneSphere->SetupAttachment(RootComponent);
}


void AEndZone::BeginPlay()
{
	Super::BeginPlay();

	ZoneSphere->OnComponentBeginOverlap.AddDynamic(this, &AEndZone::OnSphereOverlap);
	
}

void AEndZone::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	UE_LOG(LogTemp, Warning, TEXT("Flag has entered zone"))
	AFlag* OverlappingFlag = Cast<AFlag>(OtherActor);
	if(OverlappingFlag && OverlappingFlag->Team != Team)
	{
		ACTFGameMode* GameMode = GetWorld()->GetAuthGameMode<ACTFGameMode>();
		if(GameMode)
		{
			GameMode->FlagCaptured(OverlappingFlag, this);
		}
		OverlappingFlag->ResetLocation();
	}
	if(!OverlappingFlag)
	{
		UE_LOG(LogTemp, Warning, TEXT("Issue with flag cast, %s"), *OtherActor->GetName())
	}
}



