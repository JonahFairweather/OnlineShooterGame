// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blaster/BlasterTypes/Team.h"
#include "GameFramework/PlayerState.h"
#include "BlasterPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API ABlasterPlayerState : public APlayerState
{
	GENERATED_BODY()
public:
	virtual void OnRep_Score() override;
	UFUNCTION()
	virtual void OnRep_Defeats();
	void AddToScore(float ToAdd);
	void AddToDefeats(int32 Amount);
	void ResetScore();

	void SetAmmo(int32 AmmoAmount);

	void UpdateScoreboard(TArray<APlayerState*> ToUpdate);

	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void SetTeam(ETeam ToChange);
	void UpdateLocalPlayerLeaderboard();
	FORCEINLINE ETeam GetTeam() const { return Team; }
protected:
	
	

private:
	UPROPERTY()
	class ABlasterCharacter* Character = nullptr;

	UPROPERTY()
	class ABlasterPlayerController* Controller = nullptr;

	UPROPERTY(ReplicatedUsing=OnRep_Defeats)
	int32 Defeats;

	UPROPERTY(ReplicatedUsing = OnRep_Team)
	ETeam Team = ETeam::ET_NoTeam;

	UFUNCTION()
	void OnRep_Team();
	
};
