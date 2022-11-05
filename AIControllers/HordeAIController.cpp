// Fill out your copyright notice in the Description page of Project Settings.


#include "HordeAIController.h"

#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Blaster/BlasterCharacter.h"
#include "Blaster/AI_BTTasks/AITags.h"
#include "Blaster/AI_BTTasks/BlackboardKeys.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AISenseConfig_Sight.h"

void AHordeAIController::EatBread()
{
	
}

AHordeAIController::AHordeAIController()
{
	static ConstructorHelpers::FObjectFinder<UBehaviorTree> obj(TEXT("BehaviorTree'/Game/AI/Enemies/StarterEnemy/DefaultEnemy_BT.DefaultEnemy_BT'"));
	if(obj.Succeeded())
	{
		BehaviorTree = obj.Object;
	}
	BTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTree"));
	BlackboardComponent = CreateDefaultSubobject<UBlackboardComponent>(TEXT("Blackboard"));

	
	SetupPerceptionSystem();
}

void AHordeAIController::BeginPlay()
{
	Super::BeginPlay();
	
	RunBehaviorTree(BehaviorTree);
	BTreeComponent->StartTree(*BehaviorTree);
	BlackboardComponent->SetValueAsInt(bb_keys::PatrolPathIndex, 0);
	

	
}

void AHordeAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if(BlackboardComponent)
	{
		BlackboardComponent->InitializeBlackboard(*BehaviorTree->BlackboardAsset);
	}
}

UBlackboardComponent* AHordeAIController::GetBlackboard() const
{
	return BlackboardComponent;
}

void AHordeAIController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if(BlackboardComponent)
	{
		//UE_LOG(LogTemp, Warning, TEXT("Can see a character: %s"), BlackboardComponent->GetValueAsBool(bb_keys::CanSeePlayer) ? *FString("true") : *FString("False"))
	}
}

void AHordeAIController::StopTree()
{
	if(BTreeComponent)
	{
		BTreeComponent->StopTree();
	}
}

void AHordeAIController::OnUpdated(TArray<AActor*> const& UpdatedActors)
{
	for(size_t x = 0; x < UpdatedActors.Num(); ++x)
	{
		FActorPerceptionBlueprintInfo Info;
		GetPerceptionComponent()->GetActorsPerception(UpdatedActors[x], Info);
		for(size_t y = 0; y < Info.LastSensedStimuli.Num(); ++y)
		{
			FAIStimulus const Stim = Info.LastSensedStimuli[y];
			if(Stim.Tag == tags::noise_tag)
			{
				GetBlackboard()->SetValueAsBool(bb_keys::IsInvestigating, Stim.IsValid());
				GetBlackboard()->SetValueAsVector(bb_keys::target_location, Stim.StimulusLocation);
			}
		}
	}
}

void AHordeAIController::SetupPerceptionSystem()
{
	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("Sight Config"));
	if(SightConfig)
	{
		DefaultPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("Perception Component"));
		DefaultPerceptionComponent->bEditableWhenInherited = true;
		SetPerceptionComponent(*DefaultPerceptionComponent);
		SightConfig->SightRadius = 750.f;
		SightConfig->LoseSightRadius = SightConfig->SightRadius + 250.f;
		SightConfig->PeripheralVisionAngleDegrees = 90.0f;
		SightConfig->SetMaxAge(5.0f);
		SightConfig->AutoSuccessRangeFromLastSeenLocation = -1.f;
		SightConfig->DetectionByAffiliation.bDetectEnemies = SightConfig->DetectionByAffiliation.bDetectFriendlies =
			SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
		GetPerceptionComponent()->SetDominantSense(*SightConfig->GetSenseImplementation());
		GetPerceptionComponent()->ConfigureSense(*SightConfig);
		GetPerceptionComponent()->OnTargetPerceptionUpdated.AddDynamic(this, &AHordeAIController::OnTargetDetected);
	}
	HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("Hearing Config"));
	if(HearingConfig)
	{
		HearingConfig->HearingRange = 3000.f;
		HearingConfig->DetectionByAffiliation.bDetectEnemies = HearingConfig->DetectionByAffiliation.bDetectFriendlies =
			HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;

		GetPerceptionComponent()->ConfigureSense(*HearingConfig);
		GetPerceptionComponent()->SetDominantSense(*HearingConfig->GetSenseImplementation());
		GetPerceptionComponent()->OnPerceptionUpdated.AddDynamic(this, &ThisClass::AHordeAIController::OnUpdated);
	}
}

void AHordeAIController::OnTargetDetected(AActor* Actor, FAIStimulus const Stimulus)
{
	if(ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(Actor))
	{
		GetBlackboard()->SetValueAsBool(bb_keys::CanSeePlayer, Stimulus.IsValid());
	}
}


