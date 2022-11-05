// Fill out your copyright notice in the Description page of Project Settings.


#include "FindPlayerLocation.h"

#include "BlackboardKeys.h"
#include "NavigationSystem.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Blaster/AIControllers/HordeAIController.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"

UFindPlayerLocation::UFindPlayerLocation()
{
}

EBTNodeResult::Type UFindPlayerLocation::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	// Find the character that we can see

	ACharacter* const Player = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);

	if(Player == nullptr) return EBTNodeResult::Failed;

	auto const AIController = Cast<AHordeAIController>(OwnerComp.GetAIOwner());

	if(AIController == nullptr) return EBTNodeResult::Failed;

	APawn* const AIPlayer = AIController->GetPawn();
	if(AIPlayer == nullptr) return EBTNodeResult::Failed;
	
	FVector PlayerLocation = Player->GetActorLocation();

	FVector Origin = AIPlayer->GetActorLocation();
	if(SearchRandom)
	{
		FNavLocation Location;

		
		AIController->GetBlackboard()->SetValueAsVector(bb_keys::target_location, Location.Location);

		UNavigationSystemV1* const NavigationSystem = UNavigationSystemV1::GetCurrent(GetWorld());

		if(NavigationSystem)
		{
			
			if(NavigationSystem->GetRandomPointInNavigableRadius(Origin, MoveRadius, Location))
			{
				AIController->GetBlackboard()->SetValueAsVector(bb_keys::target_location, Location.Location);
			}
		}
	}else
	{
		AIController->GetBlackboard()->SetValueAsVector(bb_keys::target_location, PlayerLocation);
	}
	FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	return EBTNodeResult::Succeeded;
}
