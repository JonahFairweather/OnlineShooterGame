// Fill out your copyright notice in the Description page of Project Settings.


#include "AttackNotifyState.h"

#include "Blaster/AICharacters/BaseAICharacter.h"

void UAttackNotifyState::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                     float TotalDuration)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration);

	if(MeshComp && MeshComp->GetOwner())
	{
		if(ABaseAICharacter* const AICharacter = Cast<ABaseAICharacter>(MeshComp->GetOwner()))
		{
			AICharacter->EnableBoxCollision(true);
		}
	}
}

void UAttackNotifyState::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	Super::NotifyEnd(MeshComp, Animation);
	if(MeshComp && MeshComp->GetOwner())
	{
		if(ABaseAICharacter* const AICharacter = Cast<ABaseAICharacter>(MeshComp->GetOwner()))
		{
			AICharacter->EnableBoxCollision(false);
		}
	}
}
