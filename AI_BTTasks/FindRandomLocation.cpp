// Fill out your copyright notice in the Description page of Project Settings.


#include "FindRandomLocation.h"

#include "NavigationSystem.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Blaster/AIControllers/HordeAIController.h"
#include "BlackboardKeys.h"

UFindRandomLocation::UFindRandomLocation()
{
}

EBTNodeResult::Type UFindRandomLocation::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	
	auto const OwnerController = Cast<AHordeAIController>(OwnerComp.GetAIOwner());

	
	
	if(OwnerController == nullptr) Super::ExecuteTask(OwnerComp, NodeMemory);
	auto const AIPlayer = OwnerController->GetPawn();

	FVector const Origin = AIPlayer->GetActorLocation();

	FNavLocation Location;

	UNavigationSystemV1* const NavigationSystem = UNavigationSystemV1::GetCurrent(GetWorld());

	if(NavigationSystem->GetRandomPointInNavigableRadius(Origin, MoveRadius, Location))
	{
		OwnerController->GetBlackboard()->SetValueAsVector(bb_keys::target_location, Location.Location);
		
	}

	FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	return EBTNodeResult::Succeeded;
}
