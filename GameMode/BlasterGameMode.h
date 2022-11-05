// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "GameFramework/GameMode.h"
#include "BlasterGameMode.generated.h"

namespace MatchState
{
	extern BLASTER_API const FName Cooldown; //After the match has ended. Winner gets displayed and cooldown timer begins
}

/**
 * 
 */
UCLASS()
class BLASTER_API ABlasterGameMode : public AGameMode
{
	GENERATED_BODY()


public:
	ABlasterGameMode();
	virtual void PlayerEliminated(class ABlasterCharacter* ElimmedCharacter, class ABlasterPlayerController* VictimController, ABlasterPlayerController* AttackerController);
	virtual void RequestRespawn(ABlasterCharacter* RespawningCharacter, AController* RespawningController);

	void PlayerLeftGame(class ABlasterPlayerState* PlayerLeaving);

	virtual void Tick(float DeltaSeconds) override;

	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly)
	float WarmupTime = 10.f;

	UPROPERTY(EditDefaultsOnly)
	float MatchTime = 120.f;

	UPROPERTY(EditDefaultsOnly)
	float CooldownTime = 10.f;

	float LevelStartingTime = 0.f;

	FORCEINLINE float GetCountdownTime() { return CountdownTime; }

	UFUNCTION(Server, Reliable)
	void PlayerSentChat(const FString& PlayerName, const FString& Message);

	virtual float CalculateDamage(AController* Attacker, AController* Victim, float BaseDamage);

	bool bIsTeamMatch = false;

protected:
	virtual void OnMatchStateSet() override;
	
private:
	float CountdownTime = 0.f;
	
};
