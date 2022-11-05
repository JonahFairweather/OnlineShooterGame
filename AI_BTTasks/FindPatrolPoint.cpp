// Fill out your copyright notice in the Description page of Project Settings.


#include "FindPatrolPoint.h"

#include "BlackboardKeys.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Blaster/AICharacters/BaseAICharacter.h"
#include "Blaster/AIControllers/HordeAIController.h"

UFindPatrolPoint::UFindPatrolPoint()
{
	NodeName = TEXT("Find Patrol Path Point");
}

EBTNodeResult::Type UFindPatrolPoint::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{

	AHordeAIController* const AIController = Cast<AHordeAIController>(OwnerComp.GetAIOwner());

	int const Index = AIController->GetBlackboard()->GetValueAsInt(bb_keys::PatrolPathIndex);

	ABaseAICharacter* const AICharacter = Cast<ABaseAICharacter>(AIController->GetPawn());

	AControlPath* AIPath = AICharacter->PatrolPath;

	FVector const Point = AIPath->GetPatrolPoint(Index);

	FVector const GlobalPoint = AIPath->GetActorTransform().TransformPosition(Point);

	AIController->GetBlackboard()->SetValueAsVector(bb_keys::PatrolPathVector, GlobalPoint);

	FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);

	return EBTNodeResult::Succeeded;
	
}
