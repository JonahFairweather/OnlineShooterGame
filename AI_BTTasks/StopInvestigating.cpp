// Fill out your copyright notice in the Description page of Project Settings.


#include "StopInvestigating.h"

#include "BlackboardKeys.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Blaster/AICharacters/BaseAICharacter.h"
#include "Blaster/AIControllers/HordeAIController.h"

UStopInvestigating::UStopInvestigating()
{
}

EBTNodeResult::Type UStopInvestigating::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AHordeAIController* AIController = Cast<AHordeAIController>(OwnerComp.GetAIOwner());
	if(AIController == nullptr) return EBTNodeResult::Failed;

	AIController->GetBlackboard()->SetValueAsBool(bb_keys::IsInvestigating, false);

	
	FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	return EBTNodeResult::Succeeded;
	
}
