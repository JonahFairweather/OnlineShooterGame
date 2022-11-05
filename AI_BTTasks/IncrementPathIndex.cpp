// Fill out your copyright notice in the Description page of Project Settings.


#include "IncrementPathIndex.h"

#include "BlackboardKeys.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Blaster/AICharacters/BaseAICharacter.h"
#include "Blaster/AIControllers/HordeAIController.h"

UIncrementPathIndex::UIncrementPathIndex()
{
	NodeName = TEXT("Increment Path Index");
}

EBTNodeResult::Type UIncrementPathIndex::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	
	AHordeAIController* const AIController = Cast<AHordeAIController>(OwnerComp.GetAIOwner());

	int const Index = AIController->GetBlackboard()->GetValueAsInt(bb_keys::PatrolPathIndex);

	ABaseAICharacter* const AICharacter = Cast<ABaseAICharacter>(AIController->GetPawn());

	int const NumPoints = AICharacter->PatrolPath->Num();

	// Get and set the black board index

	if(Index == NumPoints - 1)
	{
		Direction = EDirectionType::Reverse;
	}else if(Index == 0)
	{
		Direction = EDirectionType::Forward;
	}

	int NewIndex = Direction == EDirectionType::Forward ? Index + 1 : Index - 1;

	AIController->GetBlackboard()->SetValueAsInt(bb_keys::PatrolPathIndex, NewIndex);
	
	FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	
	return EBTNodeResult::Succeeded;
}
