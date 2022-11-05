// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Blaster/Interfaces/InteractWithCrosshairsInterface.h"
#include "TestingDummy.generated.h"

UCLASS()
class BLASTER_API ATestingDummy : public AActor, public IInteractWithCrosshairsInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATestingDummy();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
