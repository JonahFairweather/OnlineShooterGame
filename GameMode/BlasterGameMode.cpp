// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterGameMode.h"
#include "Blaster/BlasterCharacter.h"
#include "Blaster/GameState/BlasterGameState.h"
#include "Blaster/PlayerState/BlasterPlayerState.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Blaster/WorldSpawners/AISpawner.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"

namespace MatchState
{
	const FName Cooldown = FName("Cooldown");
}
ABlasterGameMode::ABlasterGameMode()
{
	bDelayedStart = true;
	
}
float ABlasterGameMode::CalculateDamage(AController* Attacker, AController* Victim, float BaseDamage)
{
	return BaseDamage;
}
void ABlasterGameMode::PlayerEliminated(class ABlasterCharacter* ElimmedCharacter, class ABlasterPlayerController* VictimController, ABlasterPlayerController* AttackerController)
{
	ABlasterPlayerState* AttackerPlayerState = AttackerController ? Cast<ABlasterPlayerState>(AttackerController->PlayerState) : nullptr;
	ABlasterPlayerState* VictimPlayerState = VictimController ? Cast<ABlasterPlayerState>(VictimController->PlayerState) : nullptr;
	if (ElimmedCharacter) {
		ElimmedCharacter->Elim(false);
	}
	ABlasterGameState* BlasterGameState = GetGameState<ABlasterGameState>();
	TArray<ABlasterPlayerState*> TopPlayersCurrently;
	if(AttackerPlayerState && AttackerPlayerState != VictimPlayerState && BlasterGameState)
	{
		
		for(auto LeadPlayer : BlasterGameState->TopScoringPlayers)
		{
			TopPlayersCurrently.Add(LeadPlayer);
		}
		AttackerPlayerState->AddToScore(100.f);
		BlasterGameState->UpdateTopScore(AttackerPlayerState);
		
		for(int32 i = 0; i < TopPlayersCurrently.Num() ; i++)
		{
			if(!BlasterGameState->TopScoringPlayers.Contains(TopPlayersCurrently[i]))
			{
				ABlasterCharacter* Loser = Cast<ABlasterCharacter>(TopPlayersCurrently[i]->GetPawn());
				if(Loser)
				{
					Loser->MulticastLostTheLead();
				}
			}
		}
		if(BlasterGameState->TopScoringPlayers.Contains(AttackerPlayerState))
		{
			ABlasterCharacter* Winner = Cast<ABlasterCharacter>(AttackerPlayerState->GetPawn());
			{
				if(Winner)
				{
					Winner->MulticastGainedTheLead();
				}
			}
		}
	}
	if(VictimPlayerState)
	{
		VictimPlayerState->AddToDefeats(1);
	}
	for(FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		ABlasterPlayerController* BlasterPlayer = Cast<ABlasterPlayerController>(*It);
		if(BlasterPlayer && AttackerPlayerState && VictimPlayerState)
		{
			BlasterPlayer->BroadcastElim(AttackerPlayerState, VictimPlayerState);
		}
	}
}



void ABlasterGameMode::RequestRespawn(ABlasterCharacter* RespawningCharacter,
                                      AController* RespawningController)
{
	if(RespawningCharacter)
	{
		RespawningCharacter->Reset();
		RespawningCharacter->Destroy();
	}

	if(RespawningController)
	{
		TArray<AActor*> ActorPointers;
		UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), ActorPointers);
		const int32 Selection = FMath::RandRange(0, ActorPointers.Num() - 1);
		RestartPlayerAtPlayerStart(RespawningController, ActorPointers[Selection]);
	}
}

void ABlasterGameMode::PlayerLeftGame(ABlasterPlayerState* PlayerLeaving)
{
	// Call elim, letting it know that the player has left the game
	if(PlayerLeaving == nullptr) return;
	ABlasterGameState* BlasterGameState = GetGameState<ABlasterGameState>();
	if(BlasterGameState && BlasterGameState->TopScoringPlayers.Contains(PlayerLeaving))
	{
		BlasterGameState->TopScoringPlayers.Remove(PlayerLeaving);
	}
	ABlasterCharacter* CharacterLeaving = Cast<ABlasterCharacter>(PlayerLeaving->GetPawn());
	if(CharacterLeaving)
	{
		CharacterLeaving->Elim(true);
	}
}

void ABlasterGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if(MatchState == MatchState::WaitingToStart)
	{
		CountdownTime = WarmupTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if(CountdownTime <= 0.f)
		{
			StartMatch();
		}
	}else if(MatchState == MatchState::InProgress)
	{
		//Once the game timer expires
		CountdownTime = WarmupTime + MatchTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if(CountdownTime <= 0.f)
		{
			SetMatchState(MatchState::Cooldown);
		}
	}else if(MatchState == MatchState::Cooldown)
	{
		CountdownTime = WarmupTime + MatchTime + CooldownTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if(CountdownTime <= 0.f)
		{
			RestartGame();
		}
	}
	
}

void ABlasterGameMode::PlayerSentChat_Implementation(const FString& PlayerName, const FString& Message)
{
	for(FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		ABlasterPlayerController* BlasterPlayer = Cast<ABlasterPlayerController>(*It);
		BlasterPlayer->BroadcastChat(PlayerName, Message);
	}
}

void ABlasterGameMode::OnMatchStateSet()
{
	Super::OnMatchStateSet();


	

	for(FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator() ; Iterator ; ++Iterator)
	{
		ABlasterPlayerController* BlasterPlayer = Cast<ABlasterPlayerController>(*Iterator);
		if(BlasterPlayer)
		{
			BlasterPlayer->OnMatchStateSet(MatchState, bIsTeamMatch);
		}
	}
	
}

void ABlasterGameMode::BeginPlay()
{
	Super::BeginPlay();

	LevelStartingTime = GetWorld()->GetTimeSeconds();
}


