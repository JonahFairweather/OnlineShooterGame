// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blaster/BlasterTypes/Team.h"
#include "GameFramework/GameState.h"
#include "BlasterGameState.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API ABlasterGameState : public AGameState
{
	GENERATED_BODY()
public:
	UPROPERTY(Replicated)
	TArray<class ABlasterPlayerState*> TopScoringPlayers;

	UPROPERTY(Replicated)
	TArray<class ABlasterPlayerState*> PlayerStats;

	void UpdateTopScore(class ABlasterPlayerState* ScoringPlayer);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	

	/*
	 * Teams
	 */

	TArray<ABlasterPlayerState*> RedTeam;
	
	TArray<ABlasterPlayerState*> BlueTeam;

	UPROPERTY(ReplicatedUsing = OnRep_RedTeamScore);
	float RedTeamScore = 0.f;
	UFUNCTION()
	void OnRep_RedTeamScore();

	UPROPERTY(ReplicatedUsing = OnRep_BlueTeamScore)
	float BlueTeamScore = 0.f;
	UFUNCTION()
	void OnRep_BlueTeamScore();

	void RedTeamScores();
	void BlueTeamScores();

private:
	float TopScore = 0.f;
};
