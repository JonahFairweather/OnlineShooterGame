// Fill out your copyright notice in the Description page of Project Settings.


#include "ChangeSpeed.h"

#include "AIController.h"
#include "Blaster/AICharacters/BaseAICharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

UChangeSpeed::UChangeSpeed()
{
	NodeName = TEXT("Change Speed");

	bNotifyBecomeRelevant = true;
}

void UChangeSpeed::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnBecomeRelevant(OwnerComp, NodeMemory);

	auto const AIController = OwnerComp.GetAIOwner();

	if(AIController == nullptr) return;

	ABaseAICharacter* AICharacter = Cast<ABaseAICharacter>(AIController->GetPawn());

	if(AICharacter == nullptr || AICharacter->GetCharacterMovement() == nullptr) return;

	AICharacter->GetCharacterMovement()->MaxWalkSpeed = Speed;
}

FString UChangeSpeed::GetStaticServiceDescription() const
{
	return FString("Changes the character's speed to the given float amount");
}