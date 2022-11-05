// Fill out your copyright notice in the Description page of Project Settings.


#include "TeamsLeaderboard.h"

#include "Blaster/PlayerState/BlasterPlayerState.h"
#include "Components/TextBlock.h"

void UTeamsLeaderboard::UpdateLeaderboard(APlayerState* Player)
{
	Super::UpdateLeaderboard(Player);
	ABlasterPlayerState* PlayerState = Cast<ABlasterPlayerState>(Player);
	if(PlayerState)
	{
		ETeam PlayerTeam = PlayerState->GetTeam();
		if(PlayerTeam == ETeam::ET_BlueTeam)
		{
			BluePlayers.AddUnique(PlayerState);
		}
		else if(PlayerTeam == ETeam::ET_RedTeam)
		{
			RedPlayers.AddUnique(PlayerState);
		}
	}
	UpdateLeaderboardText();
}

void UTeamsLeaderboard::UpdateLeaderboard(APlayerState* Attacker, APlayerState* Victim)
{
	Super::UpdateLeaderboard(Attacker, Victim);
	UpdateLeaderboard(Attacker);
	UpdateLeaderboard(Victim);
}

void UTeamsLeaderboard::UpdateLeaderboardText()
{
	Super::UpdateLeaderboardText();
	Algo::Sort(RedPlayers, [](const ABlasterPlayerState* One, const ABlasterPlayerState* Two){ return One->GetScore() > Two->GetScore(); });
	Algo::Sort(BluePlayers, [](const ABlasterPlayerState* One, const ABlasterPlayerState* Two){ return One->GetScore() > Two->GetScore(); });
	
	FString OverallBlueStats;
	for(ABlasterPlayerState* Player : BluePlayers){
		FString PlayerStatistics;
		PlayerStatistics.Append(Player->GetPlayerName());
		PlayerStatistics.Append(" ");
		PlayerStatistics.AppendInt(FMath::RoundToInt(Player->GetScore()));
		PlayerStatistics.Append("\n");
		OverallBlueStats.Append(PlayerStatistics);
	}
	if(BlueTeamScores)
	{
		BlueTeamScores->SetText(FText::FromString(OverallBlueStats));
	}
	FString OverallRedStats;
	for(ABlasterPlayerState* Player : RedPlayers){
		FString PlayerStatistics;
		PlayerStatistics.Append(Player->GetPlayerName());
		PlayerStatistics.Append(" ");
		PlayerStatistics.AppendInt(FMath::RoundToInt(Player->GetScore()));
		PlayerStatistics.Append("\n");
		OverallRedStats.Append(PlayerStatistics);
	}
	if(RedTeamScores)
	{
		RedTeamScores->SetText(FText::FromString(OverallRedStats));
	}
}
