// Fill out your copyright notice in the Description page of Project Settings.


#include "IsPlayerInMeleeRange.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "Blaster/BlasterCharacter.h"
#include "Blaster/AICharacters/BaseAICharacter.h"
#include "Blaster/AIControllers/HordeAIController.h"
#include "Blaster/AI_BTTasks/BlackboardKeys.h"
#include "Kismet/GameplayStatics.h"

UIsPlayerInMeleeRange::UIsPlayerInMeleeRange()
{
	bNotifyBecomeRelevant = true;
	NodeName = TEXT("Is Player in Melee Range");
}

void UIsPlayerInMeleeRange::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AHordeAIController* AIController = Cast<AHordeAIController>(OwnerComp.GetAIOwner());

	ABaseAICharacter* AICharacter = Cast<ABaseAICharacter>(AIController->GetPawn());

	ACharacter* const Player = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);

	AIController->GetBlackboard()->SetValueAsBool(bb_keys::PlayerIsInMeleeRange, AICharacter->GetDistanceTo(Player) <= MeleeRange);
	
	Super::OnBecomeRelevant(OwnerComp, NodeMemory);
}

FString UIsPlayerInMeleeRange::GetStaticServiceDescription() const
{
	return Super::GetStaticServiceDescription();
}
