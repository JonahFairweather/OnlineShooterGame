// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "StopInvestigating.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API UStopInvestigating : public UBTTask_BlackboardBase
{
	GENERATED_BODY()
public:
	UStopInvestigating();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

private:
};
