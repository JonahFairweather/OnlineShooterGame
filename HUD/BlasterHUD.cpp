// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterHUD.h"

#include "Announcement.h"
#include "GameFramework/PlayerController.h"
#include "CharacterOverlay.h"
#include "ElimAnnouncement.h"
#include "Leaderboard.h"
#include "Blaster/PlayerState/BlasterPlayerState.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/TextBlock.h"
#include "Components/WrapBox.h"

class ABlasterPlayerState;

void ABlasterHUD::DrawHUD()
{
	Super::DrawHUD();

	FVector2D ViewportSize;
	if (GEngine) {
		GEngine->GameViewport->GetViewportSize(ViewportSize);
		const FVector2D ViewportCenter(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);

		float SpreadScaled = CrosshairSpreadMax * HUDPackage.CrosshairSpread;
		if (HUDPackage.CrosshairsCenter) {
			FVector2D Spread(0.f, 0.f);
			DrawCrosshair(HUDPackage.CrosshairsCenter, ViewportCenter, Spread, HUDPackage.CrosshairsColor);
		}
		if (HUDPackage.CrosshairsRight) {
			FVector2D Spread(SpreadScaled, 0.f);
			DrawCrosshair(HUDPackage.CrosshairsRight, ViewportCenter, Spread, HUDPackage.CrosshairsColor);
		}
		if (HUDPackage.CrosshairsLeft) {
			FVector2D Spread(-SpreadScaled, 0.f);
			DrawCrosshair(HUDPackage.CrosshairsLeft, ViewportCenter, Spread, HUDPackage.CrosshairsColor);
		}
		if (HUDPackage.CrosshairsTop) {
			FVector2D Spread(0.f, -SpreadScaled);
			DrawCrosshair(HUDPackage.CrosshairsTop, ViewportCenter, Spread, HUDPackage.CrosshairsColor);
		}
		if (HUDPackage.CrosshairsBottom) {
			FVector2D Spread(0.f, SpreadScaled);
			DrawCrosshair(HUDPackage.CrosshairsBottom, ViewportCenter, Spread, HUDPackage.CrosshairsColor);
		}
	}

	
}



void ABlasterHUD::BeginPlay()
{
	Super::BeginPlay();
	APlayerController* PlayerController = GetOwningPlayerController();
	
	
}

void ABlasterHUD::AddAnnouncement()
{
	APlayerController* PlayerController = GetOwningPlayerController();
	if (PlayerController && AnnouncementClass) {
		AnnouncementWidget = CreateWidget<UAnnouncement>(PlayerController, AnnouncementClass);
		AnnouncementWidget->AddToViewport();
	}
}

void ABlasterHUD::AddCharacterOverlay()
{
	if(CharacterOverlay != nullptr) return;
	
	APlayerController* PlayerController = GetOwningPlayerController();
	if (PlayerController && CharacterOverlayClass) {
		CharacterOverlay = CreateWidget<UCharacterOverlay>(PlayerController, CharacterOverlayClass);
		CharacterOverlay->AddToViewport();
	}
}


void ABlasterHUD::AddLeaderboard(bool bIsTeamsMatch)
{
	
	if(bIsTeamsMatch)
	{
		AddTeamsLeaderboard();
		
	}else
	{
		AddFfaLeaderboard();
		
	}
	
}

void ABlasterHUD::AddTeamsLeaderboard()
{
	if(Leaderboard != nullptr)
	{
		return;
	}
	APlayerController* PlayerController = GetOwningPlayerController();
	if (PlayerController && TeamsLeaderboardClass) {
		Leaderboard = CreateWidget<ULeaderboard>(PlayerController, TeamsLeaderboardClass);
		Leaderboard->AddToViewport();
		Leaderboard->SetVisibility(ESlateVisibility::Hidden);
	}
}

void ABlasterHUD::AddFfaLeaderboard()
{
	if(Leaderboard != nullptr)
	{
		return;
	}
	APlayerController* PlayerController = GetOwningPlayerController();
	if (PlayerController && FreeForAllLeaderboardClass) {
		Leaderboard = CreateWidget<ULeaderboard>(PlayerController, FreeForAllLeaderboardClass);
		Leaderboard->AddToViewport();
		Leaderboard->SetVisibility(ESlateVisibility::Hidden);
	}
}


void ABlasterHUD::ShowLeaderboard(bool Show)
{
	if(Leaderboard)
	{
		if(Show)
		{
			Leaderboard->SetVisibility(ESlateVisibility::Visible);
		}else
		{
			Leaderboard->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}

void ABlasterHUD::AddElimAnnouncement(FString Attacker, FString Victim)
{
	OwningPlayer = OwningPlayer == nullptr ? GetOwningPlayerController() : OwningPlayer;
	if(OwningPlayer && ElimAnnouncementClass)
	{
		
		UElimAnnouncement* ElimAnnouncementWidget = CreateWidget<UElimAnnouncement>(OwningPlayer, ElimAnnouncementClass);
		if(ElimAnnouncementWidget)
		{
			ElimAnnouncementWidget->SetElimAnnouncementText(Attacker, Victim);
			ElimAnnouncementWidget->AddToViewport();
			ElimAnnouncements.Add(ElimAnnouncementWidget);
			for(auto Message : ElimAnnouncements)
			{
				if(Message && Message->AnnouncementBox)
				{
					UCanvasPanelSlot* CanvasSlot = UWidgetLayoutLibrary::SlotAsCanvasSlot(Message->AnnouncementBox);
					if(CanvasSlot)
					{
						FVector2D Position = CanvasSlot->GetPosition();
						FVector2D NewPosition(Position.X, Position.Y -CanvasSlot->GetSize().Y);
						CanvasSlot->SetPosition(NewPosition);
					}
					
				}
			}

			FTimerHandle ElimMSGTimer;
			FTimerDelegate ElimMsgDelegate;
			ElimMsgDelegate.BindUFunction(this, FName("ElimAnnouncementTimerFinished"), ElimAnnouncementWidget);
			GetWorldTimerManager().SetTimer(ElimMSGTimer, ElimMsgDelegate, ElimAnnouncementTime, false);
		}
	}
}

void ABlasterHUD::AddDynamicAnnouncement(FString Announcement)
{
	OwningPlayer = OwningPlayer == nullptr ? GetOwningPlayerController() : OwningPlayer;
	if(OwningPlayer && ElimAnnouncementClass)
	{
		
		UElimAnnouncement* ElimAnnouncementWidget = CreateWidget<UElimAnnouncement>(OwningPlayer, ElimAnnouncementClass);
		if(ElimAnnouncementWidget)
		{
			ElimAnnouncementWidget->AnnouncementText->SetText(FText::FromString(Announcement));
			ElimAnnouncementWidget->AddToViewport();
			ElimAnnouncements.Add(ElimAnnouncementWidget);
			for(auto Message : ElimAnnouncements)
			{
				if(Message && Message->AnnouncementBox)
				{
					UCanvasPanelSlot* CanvasSlot = UWidgetLayoutLibrary::SlotAsCanvasSlot(Message->AnnouncementBox);
					if(CanvasSlot)
					{
						FVector2D Position = CanvasSlot->GetPosition();
						FVector2D NewPosition(Position.X, Position.Y -CanvasSlot->GetSize().Y);
						CanvasSlot->SetPosition(NewPosition);
					}
					
				}
			}

			FTimerHandle ElimMSGTimer;
			FTimerDelegate ElimMsgDelegate;
			ElimMsgDelegate.BindUFunction(this, FName("ElimAnnouncementTimerFinished"), ElimAnnouncementWidget);
			GetWorldTimerManager().SetTimer(ElimMSGTimer, ElimMsgDelegate, ElimAnnouncementTime, false);
		}
	}
}


void ABlasterHUD::ElimAnnouncementTimerFinished(UElimAnnouncement* MsgToRemove)
{
	if(MsgToRemove)
	{
		ElimAnnouncements.Remove(MsgToRemove);
		MsgToRemove->RemoveFromParent();
	}
}
void ABlasterHUD::AddChat(FString PlayerName, FString Message)
{
	OwningPlayer = OwningPlayer == nullptr ? GetOwningPlayerController() : OwningPlayer;
	if(OwningPlayer && PlayerChatClass)
	{
		UPlayerChat* PlayerChatWidget = CreateWidget<UPlayerChat>(OwningPlayer, PlayerChatClass);
		if(PlayerChatWidget)
		{
			PlayerChatWidget->AddPlayerChat(PlayerName, Message);
			PlayerChatWidget->AddToViewport();
			PlayerChats.Add(PlayerChatWidget);
			for(auto PlayerChat : PlayerChats)
			{
				if(PlayerChat && PlayerChat->ChatBox)
				{
					UCanvasPanelSlot* CanvasPanel = UWidgetLayoutLibrary::SlotAsCanvasSlot(PlayerChat->ChatBox);
					if(CanvasPanel)
					{
						FVector2D InitPosition = CanvasPanel->GetPosition();
						FVector2D NewPosition(InitPosition.X, InitPosition.Y - CanvasPanel->GetSize().Y);
						CanvasPanel->SetPosition(NewPosition);
					}
				}
			}
			FTimerHandle DeleteTimer;
			FTimerDelegate ElimTimer;
			ElimTimer.BindUFunction(this, FName("ChatTimerFinished"), PlayerChatWidget);
			GetWorldTimerManager().SetTimer(DeleteTimer, ElimTimer, ChatTime, false);
		}
	}
}

void ABlasterHUD::AddPowerupImage(UImage* Image)
{
	if(CharacterOverlay && CharacterOverlay->ActivePowerups)
	{
		
	}
}

void ABlasterHUD::OpenTextBox()
{
	if(PlayerChatBoxClass == nullptr) return;
	APlayerController* PlayerController = GetOwningPlayerController();
	if (PlayerController && PlayerChatBoxClass) {
		PlayerChatBoxWidget = CreateWidget<UPlayerChatBox>(PlayerController, PlayerChatBoxClass);
		if(PlayerChatBoxWidget)
		{
			PlayerChatBoxWidget->AddToViewport();
		}
	}
}

void ABlasterHUD::ChatTimerFinished(UPlayerChat* ChatToRemove)
{
	if(ChatToRemove)
	{
		PlayerChats.Remove(ChatToRemove);
		ChatToRemove->RemoveFromParent();
	}
}


void ABlasterHUD::DrawCrosshair(UTexture2D* Texture, FVector2D ViewportCenter, FVector2D Spread, FLinearColor CrosshairColor) {
	const float TextureWidth = Texture->GetSizeX();
	const float TextureHeight = Texture->GetSizeY();

	const FVector2D TextureDrawPoint(
		ViewportCenter.X - TextureWidth / 2.f + Spread.X, 
		ViewportCenter.Y - TextureHeight / 2.f + Spread.Y
	);

	DrawTexture(
		Texture,
		TextureDrawPoint.X,
		TextureDrawPoint.Y,
		TextureWidth,
		TextureHeight,
		0.f,
		0.f,
		1.f,
		1.f,
		CrosshairColor
		);
}


