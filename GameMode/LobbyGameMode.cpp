// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyGameMode.h"

#include "MultiplayerSessionsSubsystem.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "GameFramework/GameStateBase.h"

void ALobbyGameMode::PostLogin(APlayerController* NewPlayer) {
	Super::PostLogin(NewPlayer);

	ABlasterPlayerController* PlayerController = Cast<ABlasterPlayerController>(NewPlayer);
	if(PlayerController)
	{
		PlayerController->ClientAddLobbyWidget();
	}
}

void ALobbyGameMode::PlayerReady()
{
	NumPlayersReady++;
	UE_LOG(LogTemp, Warning, TEXT("Player Readied Up!"))
	for(FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		ABlasterPlayerController* BlasterPlayer = Cast<ABlasterPlayerController>(*It);
		if(BlasterPlayer)
		{
			BlasterPlayer->ClientUpdatePlayersReady(NumPlayersReady);
		}
	}
	UGameInstance* GameInstance = GetGameInstance();
	if(GameInstance)
	{
		UMultiplayerSessionsSubsystem* Subsystem = GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>();
		check(Subsystem);
		if(Subsystem)
		{
			if(NumPlayersReady >= Subsystem->DesiredNumPublicConnections)
			{
				// Enough players have readied up, start the game
				UWorld* World = GetWorld();
				if(World)
				{
					GetWorldTimerManager().SetTimer(GameStartTimer, this, &ALobbyGameMode::StartGame, CountdownTime);
					for(FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
					{
						ABlasterPlayerController* BlasterPlayer = Cast<ABlasterPlayerController>(*It);
						if(BlasterPlayer)
						{
							BlasterPlayer->ClientStartGameCountdown(CountdownTime);
						}
					}
				}
			}
		}
	}
	
}

void ALobbyGameMode::PlayerUnready()
{
	NumPlayersReady--;
	for(FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		ABlasterPlayerController* BlasterPlayer = Cast<ABlasterPlayerController>(*It);
		if(BlasterPlayer)
		{
			BlasterPlayer->ClientUpdatePlayersReady(NumPlayersReady);
		}
	}
	UGameInstance* GameInstance = GetGameInstance();
	if(GameInstance)
	{
		UMultiplayerSessionsSubsystem* Subsystem = GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>();
		check(Subsystem);
		if(Subsystem)
		{
			if(NumPlayersReady < Subsystem->DesiredNumPublicConnections)
			{
				//A player has unreadied and the game timer should reflect that
				UWorld* World = GetWorld();
				if(World)
				{
					if(GetWorldTimerManager().IsTimerActive(GameStartTimer))
					{
						GetWorldTimerManager().ClearTimer(GameStartTimer);
					}
					for(FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
					{
						ABlasterPlayerController* BlasterPlayer = Cast<ABlasterPlayerController>(*It);
						if(BlasterPlayer)
						{
							BlasterPlayer->ClientStopGameCountdown();
						}
					}
				}
			}
		}
	}
}

void ALobbyGameMode::BeginPlay()
{
	Super::BeginPlay();
}

void ALobbyGameMode::StartGame()
{
	UE_LOG(LogTemp, Warning, TEXT("Game has been started"))
	for(FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		ABlasterPlayerController* BlasterPlayer = Cast<ABlasterPlayerController>(*It);
		if(BlasterPlayer)
		{
			BlasterPlayer->ClientLobbyWidgetTeardown();
		}
	}
	UGameInstance* GameInstance = GetGameInstance();
	if(GameInstance)
	{
		UMultiplayerSessionsSubsystem* Subsystem = GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>();
		check(Subsystem);
		
		UWorld* World = GetWorld();
		if (World) {
			bUseSeamlessTravel = true;
			FString MatchType = Subsystem->DesiredMatchType;
			if(MatchType == "FreeForAll")
			{
				World->ServerTravel(FString("/Game/Maps/FreeForAllMap?listen"));
			}else if (MatchType == "TeamDeathmatch")
			{
				World->ServerTravel(FString("/Game/Maps/TeamDeathmatchMap?listen"));
			}else if(MatchType == "CaptureTheFlag")
			{
				World->ServerTravel(FString("/Game/Maps/CaptureTheFlagMap?listen"));
			}
		}
	}
}

