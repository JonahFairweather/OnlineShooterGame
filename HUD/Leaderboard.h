// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Leaderboard.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API ULeaderboard : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* PlayerStats;

	virtual void UpdateLeaderboard(APlayerState* Attacker, APlayerState* Victim);

	virtual void UpdateLeaderboard(APlayerState* Player);

	virtual void UpdateLeaderboardText();

protected:
	

private:
	float UpdateLeaderboardTime = 2.f;

	float UpdateTime = 0.f;
	
};
