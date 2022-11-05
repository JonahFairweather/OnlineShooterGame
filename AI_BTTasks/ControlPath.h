// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ControlPath.generated.h"

UCLASS()
class BLASTER_API AControlPath : public AActor
{
	GENERATED_BODY()
	
public:	
	
	AControlPath();

protected:
	

public:

	FVector GetPatrolPoint(int32 const Index) const;

	int32 Num() const;

private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI", meta = ( MakeEditWidget = "true", AllowPrivateAccess = "true" ))
	TArray<FVector> Data;
	
};
