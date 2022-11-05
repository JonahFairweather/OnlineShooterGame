// Fill out your copyright notice in the Description page of Project Settings.


#include "TeamsGameMode.h"

#include "Blaster/GameState/BlasterGameState.h"
#include "Blaster/PlayerState/BlasterPlayerState.h"
#include "Kismet/GameplayStatics.h"

void ATeamsGameMode::HandleMatchHasStarted()
{
	Super::HandleMatchHasStarted();

	ABlasterGameState* BGameState = Cast<ABlasterGameState>(UGameplayStatics::GetGameState(this));
	if (BGameState)
	{
		for (auto PState : BGameState->PlayerArray)
		{
			ABlasterPlayerState* BPState = Cast<ABlasterPlayerState>(PState.Get());
			if (BPState && BPState->GetTeam() == ETeam::ET_NoTeam)
			{
				if (BGameState->BlueTeam.Num() >= BGameState->RedTeam.Num())
				{
					BGameState->RedTeam.AddUnique(BPState);
					BPState->SetTeam(ETeam::ET_RedTeam);
				}
				else
				{
					BGameState->BlueTeam.AddUnique(BPState);
					BPState->SetTeam(ETeam::ET_BlueTeam);
				}
			}
		}
	}
}

void ATeamsGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	ABlasterGameState* BGameState = Cast<ABlasterGameState>(UGameplayStatics::GetGameState(this));
	if (BGameState)
	{
		ABlasterPlayerState* BPState = NewPlayer->GetPlayerState<ABlasterPlayerState>();
		if (BPState && BPState->GetTeam() == ETeam::ET_NoTeam)
		{
			if (BGameState->BlueTeam.Num() >= BGameState->RedTeam.Num())
			{
				BGameState->RedTeam.AddUnique(BPState);
				BPState->SetTeam(ETeam::ET_RedTeam);
			}
			else
			{
				BGameState->BlueTeam.AddUnique(BPState);
				BPState->SetTeam(ETeam::ET_BlueTeam);
			}
		}
	}
}

void ATeamsGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);
	
	ABlasterGameState* BGameState = Cast<ABlasterGameState>(UGameplayStatics::GetGameState(this));
	if (BGameState)
	{
		ABlasterPlayerState* BPState = Exiting->GetPlayerState<ABlasterPlayerState>();
		if (BPState)
		{
			if (BGameState->RedTeam.Contains(BPState))
			{
				BGameState->RedTeam.Remove(BPState);
			}
			else if(BGameState->BlueTeam.Contains(BPState))
			{
				BGameState->BlueTeam.Remove(BPState);
			}
		}
	}
}

void ATeamsGameMode::PlayerEliminated(ABlasterCharacter* ElimmedCharacter, ABlasterPlayerController* VictimController,
	ABlasterPlayerController* AttackerController)
{
	Super::PlayerEliminated(ElimmedCharacter, VictimController, AttackerController);
	ABlasterGameState* BlasterGameState = GetGameState<ABlasterGameState>();
	ABlasterPlayerState* AttackerPlayerState = AttackerController ? Cast<ABlasterPlayerState>(AttackerController->PlayerState) : nullptr;
	ABlasterPlayerState* VictimPlayerState = VictimController ? Cast<ABlasterPlayerState>(VictimController->PlayerState) : nullptr;
	if(BlasterGameState && AttackerPlayerState && VictimPlayerState)
	{
		if(AttackerPlayerState->GetTeam() == ETeam::ET_RedTeam && VictimPlayerState->GetTeam() != ETeam::ET_RedTeam)
		{
			BlasterGameState->RedTeamScores();
		}
		if(AttackerPlayerState->GetTeam() == ETeam::ET_BlueTeam && VictimPlayerState->GetTeam() != ETeam::ET_BlueTeam)
		{
			BlasterGameState->BlueTeamScores();
		}
	}
}

float ATeamsGameMode::CalculateDamage(AController* Attacker, AController* Victim, float BaseDamage)
{
	ABlasterPlayerState* AttackerPlayerState = Attacker->GetPlayerState<ABlasterPlayerState>();
	ABlasterPlayerState* VictimPlayerState = Victim->GetPlayerState<ABlasterPlayerState>();
	if(VictimPlayerState == nullptr || AttackerPlayerState == nullptr) return BaseDamage;

	if(VictimPlayerState == AttackerPlayerState) return BaseDamage/2.f;

	if(VictimPlayerState->GetTeam() == AttackerPlayerState->GetTeam() && VictimPlayerState->GetTeam() != ETeam::ET_NoTeam) return BaseDamage/4.f;

	return BaseDamage;
}

ATeamsGameMode::ATeamsGameMode()
{
	bIsTeamMatch = true;
}
