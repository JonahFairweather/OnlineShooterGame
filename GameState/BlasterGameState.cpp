// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterGameState.h"

#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Blaster/PlayerState/BlasterPlayerState.h"
#include "Net/UnrealNetwork.h"

void ABlasterGameState::UpdateTopScore(ABlasterPlayerState* ScoringPlayer)
{
	if(TopScoringPlayers.IsEmpty())
	{
		TopScoringPlayers.Add(ScoringPlayer);
		TopScore = ScoringPlayer->GetScore();
	}else if(ScoringPlayer->GetScore() == TopScore)
	{
		TopScoringPlayers.AddUnique(ScoringPlayer);
	}else if(ScoringPlayer->GetScore() > TopScore)
	{
		TopScoringPlayers.Empty();
		TopScore = ScoringPlayer->GetScore();
		TopScoringPlayers.AddUnique(ScoringPlayer);
	}
}

void ABlasterGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{

	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ABlasterGameState, TopScoringPlayers);
	DOREPLIFETIME(ABlasterGameState, PlayerStats);
	DOREPLIFETIME(ABlasterGameState, RedTeamScore);
	DOREPLIFETIME(ABlasterGameState, BlueTeamScore);
}


void ABlasterGameState::OnRep_RedTeamScore()
{
	ABlasterPlayerController* PlayerController = Cast<ABlasterPlayerController>(GetWorld()->GetFirstPlayerController());
	if(PlayerController)
	{
		PlayerController->SetRedTeamScore(RedTeamScore);
	}
}

void ABlasterGameState::OnRep_BlueTeamScore()
{
	ABlasterPlayerController* PlayerController = Cast<ABlasterPlayerController>(GetWorld()->GetFirstPlayerController());
	if(PlayerController)
	{
		PlayerController->SetBlueTeamScore(BlueTeamScore);
	}
}

void ABlasterGameState::RedTeamScores()
{
	++RedTeamScore;
	ABlasterPlayerController* PlayerController = Cast<ABlasterPlayerController>(GetWorld()->GetFirstPlayerController());
	if(PlayerController)
	{
		PlayerController->SetRedTeamScore(RedTeamScore);
	}
}

void ABlasterGameState::BlueTeamScores()
{
	++BlueTeamScore;
	ABlasterPlayerController* PlayerController = Cast<ABlasterPlayerController>(GetWorld()->GetFirstPlayerController());
	if(PlayerController)
	{
		PlayerController->SetBlueTeamScore(BlueTeamScore);
	}
}

