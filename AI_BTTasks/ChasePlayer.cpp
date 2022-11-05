// Fill out your copyright notice in the Description page of Project Settings.


#include "ChasePlayer.h"

#include "BlackboardKeys.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Blaster/AIControllers/HordeAIController.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"

UChasePlayer::UChasePlayer()
{
	NodeName = TEXT("Chase Player");
}

EBTNodeResult::Type UChasePlayer::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	auto const OwnerController = Cast<AHordeAIController>(OwnerComp.GetAIOwner());
	
	if(OwnerController == nullptr) return Super::ExecuteTask(OwnerComp, NodeMemory);

	FVector const PlayerLocation = OwnerController->GetBlackboard()->GetValueAsVector(bb_keys::target_location);

	UAIBlueprintHelperLibrary::SimpleMoveToLocation(OwnerController, PlayerLocation);

	FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	return EBTNodeResult::Succeeded;
}
