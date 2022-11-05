// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LobbyWidget.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API ULobbyWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable)
	void MenuSetup(int32 DesiredPublicConnections = 3);

	UFUNCTION()
	void StartCountdown(float CountdownTime);

	UFUNCTION()
	void StopCountdown();
	
	void MenuTearDown();

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* Countdown;
	
private:
	UPROPERTY(meta = (BindWidget))
	class UButton* ReadyButton;
	UPROPERTY(meta = (BindWidget))
	class UButton* UnreadyButton;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* GameModeText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* PlayersReadyText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* NumPlayersReady;

	UFUNCTION()
	void ReadyButtonClicked();

	UFUNCTION()
	void UnreadyButtonClicked();

	int32 ReadyPlayers = 0;

	UFUNCTION(Server, Reliable)
	void GetNumberReadyPlayers();

	UFUNCTION(Client, Reliable)
	void ReportNumReadyPlayers(int32 Num);

	UFUNCTION(Server, Reliable)
	void PlayerReadyUp();

	UFUNCTION()
	void UpdatePlayersReady(int32 NumReadyPlayers);

	UPROPERTY()
	class ABlasterPlayerController* OwningPlayerController;

	int32 MaxReadyPlayers;

	

	bool CountdownStarted = false;

	int32 TimeUntilStart = 0;

	void UpdatePlayers();

	void TickSecondCountdown();

	FTimerHandle TickTimer;
	


	

protected:
	virtual bool Initialize() override;
	virtual void OnLevelRemovedFromWorld(ULevel* InLevel, UWorld* InWorld) override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
};


