// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Blaster/AI_BTTasks/ControlPath.h"
#include "Perception/AIPerceptionTypes.h"
#include "HordeAIController.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API AHordeAIController : public AAIController
{
	GENERATED_BODY()

protected:
	void EatBread();

public:
	AHordeAIController();

	virtual void BeginPlay() override;

	virtual void OnPossess(APawn* InPawn) override;

	class UBlackboardComponent* GetBlackboard() const;

	virtual void Tick(float DeltaSeconds) override;

	void StopTree();



private:
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "AI", meta = (AllowPrivateAccess))
	class UBehaviorTree* BehaviorTree;

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "AI", meta = (AllowPrivateAccess))
	class UBehaviorTreeComponent* BTreeComponent;

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "AI", meta = (AllowPrivateAccess))
	UBlackboardComponent* BlackboardComponent;

	UPROPERTY(BlueprintReadWrite, Category = "AI", meta = (AllowPrivateAccess))
	UAIPerceptionComponent* DefaultPerceptionComponent;
	
	class UAISenseConfig_Sight* SightConfig;

	class UAISenseConfig_Hearing* HearingConfig;

	UFUNCTION()
	void OnUpdated(TArray<AActor*> const& UpdatedActors);

	void SetupPerceptionSystem();

	UFUNCTION()
	void OnTargetDetected(AActor* Actor, FAIStimulus const Stimulus);

	
	
};