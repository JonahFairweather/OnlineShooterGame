// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "LobbyGameMode.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API ALobbyGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	virtual void PostLogin(APlayerController* NewPlayer) override;

	void PlayerReady();

	void PlayerUnready();

	virtual void BeginPlay() override;

	int32 NumPlayersReady = 0;

private:

	UPROPERTY(EditAnywhere)
	float CountdownTime = 5.f;
	
	float TimeUntilStart;

	bool bIsStartingMatch = false;

	FTimerHandle GameStartTimer;
	
	void StartGame();

protected:
	
};
