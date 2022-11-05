// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TeamsGameMode.h"
#include "CTFGameMode.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API ACTFGameMode : public ATeamsGameMode
{
	GENERATED_BODY()
public:
	virtual void PlayerEliminated(ABlasterCharacter* ElimmedCharacter, ABlasterPlayerController* VictimController, ABlasterPlayerController* AttackerController) override;

	void FlagCaptured(class AFlag* Flag, class AEndZone* Zone);

private:
	UPROPERTY(EditAnywhere)
	class USoundCue* FlagCapturedSound; //The sound to be played when a flag is captured; 
};
