// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LobbyWidget.h"
#include "PlayerChat.h"
#include "PlayerChatBox.h"
#include "GameFramework/HUD.h"
#include "BlasterHUD.generated.h"

USTRUCT(BlueprintType)
struct FHUDPackage
{
	GENERATED_BODY()
public:
	class UTexture2D* CrosshairsCenter;
	UTexture2D* CrosshairsRight;
	UTexture2D* CrosshairsLeft;
	UTexture2D* CrosshairsTop;
	UTexture2D* CrosshairsBottom;
	float CrosshairSpread;
	FLinearColor CrosshairsColor;

};
/**
 * 
 */
UCLASS()
class BLASTER_API ABlasterHUD : public AHUD
{
	GENERATED_BODY()

public:
	virtual void DrawHUD() override;

	UPROPERTY(EditAnywhere, Category = "Player Stats");
	TSubclassOf<class UUserWidget> CharacterOverlayClass;

	UPROPERTY(EditAnywhere, Category = "Announcements");
	TSubclassOf<class UElimAnnouncement> ElimAnnouncementClass;

	UPROPERTY(EditAnywhere, Category = "Chat");
	TSubclassOf<class UPlayerChat> PlayerChatClass;
	UPROPERTY(EditAnywhere, Category = "Chat");
	TSubclassOf<class UUserWidget> PlayerChatBoxClass;

	UPROPERTY()
	UPlayerChatBox* PlayerChatBoxWidget;

	UPROPERTY()
	class UAnnouncement* AnnouncementWidget;

	UPROPERTY(EditAnywhere, Category = "Announcements");
	TSubclassOf<class UUserWidget> AnnouncementClass;
	
	UPROPERTY()
	class UCharacterOverlay* CharacterOverlay;

	UPROPERTY(EditAnywhere, Category = "Announcements");
	TSubclassOf<class UUserWidget> FreeForAllLeaderboardClass;
	
	UPROPERTY(EditAnywhere, Category = "Announcements");
	TSubclassOf<class UUserWidget> TeamsLeaderboardClass;
	
	UPROPERTY()
	class ULeaderboard* Leaderboard;

	void AddAnnouncement();
	
	void AddCharacterOverlay();

	void AddLeaderboard(bool bIsTeamsMatch);

	void AddTeamsLeaderboard();
	void AddFfaLeaderboard();
	
	void ShowLeaderboard(bool Show);

	void AddElimAnnouncement(FString Attacker, FString Victim);

	void AddDynamicAnnouncement(FString Announcement);

	void AddChat(FString PlayerName, FString Message);

	void AddPowerupImage(class UImage* Image);

	void OpenTextBox();

	UFUNCTION()
	void ChatTimerFinished(UPlayerChat* ChatToRemove);

protected:
	virtual void BeginPlay() override;
	
private:
	UPROPERTY()
	class APlayerController* OwningPlayer;
	
	FHUDPackage HUDPackage;

	void DrawCrosshair(UTexture2D* Texture, FVector2D ViewportCenter, FVector2D Spread, FLinearColor CrosshairColor);

	UPROPERTY(EditAnywhere)
	float CrosshairSpreadMax = 16.f;

	UPROPERTY(EditAnywhere)
	float ElimAnnouncementTime = 1.5f;

	UPROPERTY(EditAnywhere)
	float ChatTime = 10.f;

	UFUNCTION()
	void ElimAnnouncementTimerFinished(UElimAnnouncement* MsgToRemove);

	UPROPERTY()
	TArray<UElimAnnouncement*> ElimAnnouncements;

	UPROPERTY()
	TArray<UPlayerChat*> PlayerChats; 
public:
	FORCEINLINE void SetHUDPackage(const FHUDPackage& Package) { HUDPackage = Package; }
};