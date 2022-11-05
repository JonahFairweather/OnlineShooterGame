// Fill out your copyright notice in the Description page of Project Settings.


#include "AISpawner.h"

#include "Kismet/KismetMathLibrary.h"


AAISpawner::AAISpawner()
	:
SpawnArea(CreateDefaultSubobject<UBoxComponent>(TEXT("Spawn Area")))
{
 	
	PrimaryActorTick.bCanEverTick = true;

}


void AAISpawner::BeginPlay()
{
	Super::BeginPlay();
	StartSpawning();
	
}

// Called every frame
void AAISpawner::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AAISpawner::SpawnAI()
{
	int32 Selection = FMath::RandRange(0, PossibleSpawns.Num() - 1);
	UWorld* World = GetWorld();
	if(World)
	{
		FVector Location = UKismetMathLibrary::RandomPointInBoundingBox(SpawnArea->GetComponentLocation(), SpawnArea->GetScaledBoxExtent());
		World->SpawnActor<ABaseAICharacter>(PossibleSpawns[Selection], Location, GetActorRotation());
	}
	GetWorldTimerManager().SetTimer(SpawnTimer, this, &ThisClass::SpawnAI, SpawnTime);
}

void AAISpawner::StartSpawning()
{
	GetWorldTimerManager().SetTimer(SpawnTimer, this, &ThisClass::SpawnAI, SpawnTime);
}

