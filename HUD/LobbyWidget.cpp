// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyWidget.h"

#include "MultiplayerSessionsSubsystem.h"
#include "Blaster/GameMode/LobbyGameMode.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Blaster/PlayerState/BlasterPlayerState.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"


void ULobbyWidget::ReadyButtonClicked()
{
	//Tell the server that this player is ready
	UWorld* World = GetWorld();
	if(!World) return;
	ReadyButton->SetIsEnabled(false);
	
	OwningPlayerController = OwningPlayerController == nullptr ? Cast<ABlasterPlayerController>(World->GetFirstPlayerController()) : OwningPlayerController;
	if(OwningPlayerController)
	{
		OwningPlayerController->PlayerReadiedUp(true);
	}
	if(UnreadyButton)
	{
		UnreadyButton->SetVisibility(ESlateVisibility::Visible);
		UnreadyButton->SetIsEnabled(true);
	}
	if(ReadyButton)
	{
		ReadyButton->SetVisibility(ESlateVisibility::Hidden);
		ReadyButton->SetIsEnabled(false);
	}
	
	
}

void ULobbyWidget::UnreadyButtonClicked()
{
	//Tell the server that this player is ready
	UWorld* World = GetWorld();
	if(!World) return;
	ReadyButton->SetIsEnabled(false);
	
	OwningPlayerController = OwningPlayerController == nullptr ? Cast<ABlasterPlayerController>(World->GetFirstPlayerController()) : OwningPlayerController;
	if(OwningPlayerController)
	{
		OwningPlayerController->PlayerReadiedUp(false);
	}
	if(ReadyButton)
	{
		ReadyButton->SetVisibility(ESlateVisibility::Visible);
		ReadyButton->SetIsEnabled(true);
	}
	if(UnreadyButton)
	{
		UnreadyButton->SetVisibility(ESlateVisibility::Hidden);
		UnreadyButton->SetIsEnabled(false);
	}
}

void ULobbyWidget::GetNumberReadyPlayers_Implementation()
{
	ALobbyGameMode* LobbyGameMode = Cast<ALobbyGameMode>(UGameplayStatics::GetGameMode(this));
	if(LobbyGameMode)
	{
		ReportNumReadyPlayers(LobbyGameMode->NumPlayersReady);
	}
}

void ULobbyWidget::ReportNumReadyPlayers_Implementation(int32 Num)
{
	ReadyPlayers = Num;
	if(NumPlayersReady)
	{
		NumPlayersReady->SetText(FText::FromString(FString::Printf(TEXT("%d"), ReadyPlayers)));
	}
}

void ULobbyWidget::PlayerReadyUp_Implementation()
{
	ALobbyGameMode* LobbyGameMode = Cast<ALobbyGameMode>(UGameplayStatics::GetGameMode(this));
	UE_LOG(LogTemp, Warning, TEXT("Player Pressed Ready Button!"))
	if(LobbyGameMode)
	{
		LobbyGameMode->PlayerReady();
	}
}

void ULobbyWidget::UpdatePlayersReady(int32 NumReadyPlayers)
{
	ReadyPlayers = NumReadyPlayers;
	if(NumPlayersReady)
	{
		NumPlayersReady->SetText(FText::FromString(FString::Printf(TEXT("%d/%d"), ReadyPlayers, MaxReadyPlayers)));
	}
}

void ULobbyWidget::MenuTearDown()
{
	RemoveFromParent();
	RemoveFromViewport();
	UWorld* World = GetWorld();
	if (World)
	{
		APlayerController* PlayerController = World->GetFirstPlayerController();
		if (PlayerController)
		{
			const FInputModeGameOnly InputModeData;
			PlayerController->SetInputMode(InputModeData);
			PlayerController->SetShowMouseCursor(false);
		}
	}
	if(ReadyButton && ReadyButton->OnClicked.IsBound())
	{
		ReadyButton->OnClicked.RemoveDynamic(this, &ULobbyWidget::ReadyButtonClicked);
	}
}

void ULobbyWidget::UpdatePlayers()
{
	UGameInstance* GameInstance = GetGameInstance();
	if(GameInstance)
	{
		UMultiplayerSessionsSubsystem* Subsystem = GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>();
		check(Subsystem);
		if(Subsystem)
		{
			MaxReadyPlayers = Subsystem->DesiredNumPublicConnections;
			UE_LOG(LogTemp, Warning, TEXT("Found the number of desired connections: %d"), Subsystem->DesiredNumPublicConnections)
			UpdatePlayersReady(ReadyPlayers);
			return;
		}
	}
	MaxReadyPlayers = 1;
	UpdatePlayersReady(ReadyPlayers);
}

void ULobbyWidget::TickSecondCountdown()
{
	if(Countdown && Countdown->IsVisible())
	{
		TimeUntilStart = TimeUntilStart <= 1 ? 1 : TimeUntilStart - 1;
		Countdown->SetText(FText::FromString(FString::Printf(TEXT("%d"), TimeUntilStart)));
		if(TimeUntilStart > 1)
		{
			UWorld* World = GetWorld();
			if(World)
			{
				World->GetTimerManager().SetTimer(TickTimer, this, &ULobbyWidget::TickSecondCountdown, 1.f);
			}
		}
	}
}

void ULobbyWidget::MenuSetup(int32 DesiredPublicConnections)
{
	//Add to the viewport and allow the mouse to exist

	AddToViewport();
	SetVisibility(ESlateVisibility::Visible);
	bIsFocusable = true;
	OwningPlayerController = OwningPlayerController == nullptr ? Cast<ABlasterPlayerController>(GetOwningPlayer()) : OwningPlayerController;
	if(OwningPlayerController)
	{
		OwningPlayerController->LobbyOverlay = this;
	}
	MaxReadyPlayers = DesiredPublicConnections;

	UWorld* World = GetWorld();
	if (World)
	{
		APlayerController* PlayerController = World->GetFirstPlayerController();
		if (PlayerController)
		{
			FInputModeUIOnly InputModeData;
			
			//InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			//PlayerController->SetInputMode(InputModeData);
			PlayerController->SetShowMouseCursor(true);
			ABlasterPlayerController* BlasterPlayerController = Cast<ABlasterPlayerController>(PlayerController);
			if(BlasterPlayerController)
			{
				BlasterPlayerController->PlayerReadyDelegate.AddDynamic(this, &ULobbyWidget::UpdatePlayersReady);
				OwningPlayerController = BlasterPlayerController;
			}
		}
		if(UnreadyButton)
		{
			UnreadyButton->SetVisibility(ESlateVisibility::Hidden);
			UnreadyButton->SetIsEnabled(false);
		}
	}

	if(NumPlayersReady)
	{
		GetNumberReadyPlayers();
		NumPlayersReady->SetText(FText::FromString(FString::Printf(TEXT("%d/%d"), 0, MaxReadyPlayers)));
	}
	UpdatePlayers();
	if(Countdown)
	{
		Countdown->SetVisibility(ESlateVisibility::Hidden);
	}
	UE_LOG(LogTemp, Warning, TEXT(" %d"), MaxReadyPlayers)
}

void ULobbyWidget::StartCountdown(float CountdownTime)
{
	TimeUntilStart = CountdownTime;
	if(Countdown)
	{
		Countdown->SetVisibility(ESlateVisibility::Visible);
		Countdown->SetText(FText::FromString(FString::Printf(TEXT("%d"), FMath::CeilToInt(CountdownTime))));
		UWorld* World = GetWorld();
		if(World)
		{
			World->GetTimerManager().SetTimer(TickTimer, this, &ULobbyWidget::TickSecondCountdown, 1.f);
		}
	}
}

void ULobbyWidget::StopCountdown()
{
	if(Countdown)
	{
		Countdown->SetVisibility(ESlateVisibility::Hidden);
		Countdown->SetText(FText());
	}
}


bool ULobbyWidget::Initialize()
{
	if(!Super::Initialize())
	{
		return false;
	}

	if(ReadyButton)
	{
		ReadyButton->OnClicked.AddDynamic(this, &ULobbyWidget::ReadyButtonClicked);
	}
	if(UnreadyButton)
	{
		UnreadyButton->OnClicked.AddDynamic(this, &ULobbyWidget::UnreadyButtonClicked);
	}
	UpdatePlayers();
	return true;
}

void ULobbyWidget::OnLevelRemovedFromWorld(ULevel* InLevel, UWorld* InWorld)
{
	MenuTearDown();
	Super::OnLevelRemovedFromWorld(InLevel, InWorld);
}

void ULobbyWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
}
