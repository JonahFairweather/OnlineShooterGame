// Fill out your copyright notice in the Description page of Project Settings.


#include "CTFGameMode.h"

#include "Blaster/CaptureTheFlagStuff/EndZone.h"
#include "Blaster/GameState/BlasterGameState.h"
#include "Blaster/Weapon/Flag.h"
#include "GameFramework/PlayerState.h"
#include "Sound/SoundCue.h"

void ACTFGameMode::PlayerEliminated(ABlasterCharacter* ElimmedCharacter, ABlasterPlayerController* VictimController,
                                    ABlasterPlayerController* AttackerController)
{
	ABlasterGameMode::PlayerEliminated(ElimmedCharacter, VictimController, AttackerController);
}

void ACTFGameMode::FlagCaptured(AFlag* Flag, AEndZone* Zone)
{
	bool bValidCapture = Flag->Team != Zone->Team;
	ABlasterGameState* BlasterGameState = GetGameState<ABlasterGameState>();
	if(Zone->Team == ETeam::ET_BlueTeam)
	{
		BlasterGameState->BlueTeamScores();
	}
	else if(Zone->Team == ETeam::ET_RedTeam)
	{
		BlasterGameState->RedTeamScores();
	}
	for(FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		ABlasterPlayerController* BlasterPlayer = Cast<ABlasterPlayerController>(*It);
		if(BlasterPlayer)
		{
			FString FlagColor = Flag->Team == ETeam::ET_RedTeam ? "red" : "blue";
			APlayerState* PlayerState = BlasterPlayer->GetPlayerState<APlayerState>();
			if(PlayerState)
			{
				BlasterPlayer->ClientAddGameAnnouncement(FString::Printf(TEXT("%s has captured the %s flag!"), *PlayerState->GetPlayerName(), *FlagColor));
				BlasterPlayer->ClientPlaySound(FlagCapturedSound);
			}
			
		}
	}
}
