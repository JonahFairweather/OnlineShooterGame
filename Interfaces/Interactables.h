// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Interactables.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UInteractables : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class BLASTER_API IInteractables
{
	GENERATED_BODY()


	
public:
	virtual void BecameOverlappingInteractable();

	virtual void StoppedBeingOverlappingInteractable();

	virtual void InteractedWith(AActor* Interactor);

	
};
