// Fill out your copyright notice in the Description page of Project Settings.


#include "FreeForAllLeaderboard.h"

#include "Blaster/PlayerState/BlasterPlayerState.h"
#include "Components/TextBlock.h"

void UFreeForAllLeaderboard::UpdateLeaderboardText()
{
	Algo::Sort(Players, [](const ABlasterPlayerState* One, const ABlasterPlayerState* Two){ return One->GetScore() > Two->GetScore(); });
	
	FString OverallStats;
	for(ABlasterPlayerState* Player : Players){
		FString PlayerStatistics;
		PlayerStatistics.Append(Player->GetPlayerName());
		PlayerStatistics.Append(" ");
		PlayerStatistics.AppendInt(FMath::RoundToInt(Player->GetScore()));
		PlayerStatistics.Append("\n");
		OverallStats.Append(PlayerStatistics);
	}
	if(PlayerStats)
	{
		PlayerStats->SetText(FText::FromString(OverallStats));
	}
}

void UFreeForAllLeaderboard::UpdateLeaderboard(APlayerState* Attacker, APlayerState* Victim)
{
	Super::UpdateLeaderboard(Attacker, Victim);
	ABlasterPlayerState* AttackerPlayerState = Cast<ABlasterPlayerState>(Attacker);
	ABlasterPlayerState* VictimPlayerState = Cast<ABlasterPlayerState>(Victim);
	if(VictimPlayerState == nullptr || AttackerPlayerState == nullptr) return;
	if(!Players.Contains(AttackerPlayerState))
	{
		Players.Add(AttackerPlayerState);
	}
	if(!Players.Contains(VictimPlayerState))
	{
		Players.Add(VictimPlayerState);
	}
	UpdateLeaderboardText();
}

void UFreeForAllLeaderboard::UpdateLeaderboard(APlayerState* Player)
{
	Super::UpdateLeaderboard(Player);
	ABlasterPlayerState* PlayerState = Cast<ABlasterPlayerState>(Player);
	if(!Players.Contains(PlayerState))
	{
		Players.Add(PlayerState);
	}

	UpdateLeaderboardText();
	
}
