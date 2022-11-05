// Fill out your copyright notice in the Description page of Project Settings.


#include "MeleeAttack.h"

#include "Blaster/AICharacters/BaseAICharacter.h"
#include "Blaster/AIControllers/HordeAIController.h"

UMeleeAttack::UMeleeAttack()
{
}

EBTNodeResult::Type UMeleeAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{

	AHordeAIController* AIController = Cast<AHordeAIController>(OwnerComp.GetAIOwner());
	if(AIController == nullptr) return EBTNodeResult::Failed;

	ABaseAICharacter* AICharacter = Cast<ABaseAICharacter>(AIController->GetPawn());

	if(AICharacter == nullptr) return EBTNodeResult::Failed;

	if(MontageHasFinished(AICharacter))
	{
		AICharacter->MeleeAttack();
	}
	FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	return EBTNodeResult::Succeeded;
}

bool UMeleeAttack::MontageHasFinished(ABaseAICharacter* Character)
{
	if(Character==nullptr) return false;

	return Character->MontageFinished();
	
}
