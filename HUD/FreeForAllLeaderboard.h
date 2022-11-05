// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Leaderboard.h"
#include "FreeForAllLeaderboard.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API UFreeForAllLeaderboard : public ULeaderboard
{
	GENERATED_BODY()

public:
	
	virtual void UpdateLeaderboard(APlayerState* Attacker, APlayerState* Victim) override;
	virtual void UpdateLeaderboard(APlayerState* Player) override;

	virtual void UpdateLeaderboardText() override;

private:
	TArray<class ABlasterPlayerState*> Players;
};