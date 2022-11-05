// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterPlayerState.h"

#include "Blaster/BlasterCharacter.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Net/UnrealNetwork.h"


void ABlasterPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ABlasterPlayerState, Defeats);
	DOREPLIFETIME(ABlasterPlayerState, Team);
}

void ABlasterPlayerState::SetTeam(ETeam ToChange)
{
	Team = ToChange;
	Character = Character == nullptr ? Cast<ABlasterCharacter>(GetPawn()) : Character;
	if(Character)
	{
		Character->SetTeamColor(ToChange);
		
	}
	UpdateLocalPlayerLeaderboard();
}

void ABlasterPlayerState::UpdateLocalPlayerLeaderboard()
{
	UWorld* World = GetWorld();
	if(World)
	{
		ABlasterPlayerController* LocalPlayer = Cast<ABlasterPlayerController>(World->GetFirstPlayerController());
		if(LocalPlayer)
		{
			LocalPlayer->UpdatePlayerLeaderboard(this);
		}
	}
}

void ABlasterPlayerState::OnRep_Team()
{
	Character = Character == nullptr ? Cast<ABlasterCharacter>(GetPawn()) : Character;
	if(Character)
	{
		Character->SetTeamColor(Team);
	}
	UpdateLocalPlayerLeaderboard();
}

void ABlasterPlayerState::AddToScore(float ToAdd)
{
	
	SetScore(GetScore() + ToAdd);
	Character = Character == nullptr ? Cast<ABlasterCharacter>(GetPawn()) : Character;

	if(Character)
	{
		Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
		if(Controller)
		{
			Controller->SetHUDScore(GetScore());
		}
	}
	
	
}

void ABlasterPlayerState::AddToDefeats(int32 Amount)
{
	Defeats += Amount;
	Character = Character == nullptr ? Cast<ABlasterCharacter>(GetPawn()) : Character;
	if(Character)
	{
		Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
		if(Controller)
		{
			Controller->SetHUDDefeats(Defeats);
		}
	}
}

void ABlasterPlayerState::ResetScore()
{
	SetScore(0);
}

void ABlasterPlayerState::SetAmmo(int32 AmmoAmount)
{
	if(Character)
	{
		Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
		if(Controller)
		{
			Controller->SetHUDAmmo(AmmoAmount);
		}
	}
}



void ABlasterPlayerState::OnRep_Score()
{
	Super::OnRep_Score();
	
	// Set up the character and the player controller
	if(Character)
	{
		Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
		if(Controller)
		{
			Controller->SetHUDScore(GetScore());
		}
		UpdateLocalPlayerLeaderboard();
		
		
	}
}

void ABlasterPlayerState::OnRep_Defeats()
{
	Character = Character == nullptr ? Cast<ABlasterCharacter>(GetPawn()) : Character;
	if(Character)
	{
		Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
		if(Controller)
		{
			Controller->SetHUDDefeats(Defeats);
		}
	}
}


