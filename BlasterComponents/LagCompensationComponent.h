// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Components/ActorComponent.h"
#include "LagCompensationComponent.generated.h"

USTRUCT(BlueprintType)
struct FBoxInformation
{
	GENERATED_BODY()

	UPROPERTY()
	FVector Location;

	UPROPERTY()
	FRotator BoxRotation;

	UPROPERTY()
	FVector BoxExtent;
};

USTRUCT(BlueprintType)
struct FServerSideRewindResult
{
	GENERATED_BODY()

	UPROPERTY()
	bool bHitConfirmed;

	UPROPERTY()
	bool bHeadShot;
};

USTRUCT(BlueprintType)
struct FServerSideShotgunResult
{
	GENERATED_BODY()

	UPROPERTY()
	TMap<class ABlasterCharacter*, uint32> Damage;
	
};

USTRUCT(BlueprintType)
struct FFramePackage
{
	GENERATED_BODY()
	
	UPROPERTY()
	float Time;

	UPROPERTY()
	TMap<FName, FBoxInformation> HitBoxInfo;

	UPROPERTY()
	ABlasterCharacter* Character;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BLASTER_API ULagCompensationComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	friend class ABlasterCharacter;
	ULagCompensationComponent();
	void ShowFramePackage(const FFramePackage& Package, const FColor& Color);

	FServerSideRewindResult ServerSideRewind(class ABlasterCharacter* HitCharacter, const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize& HitLocation, float HitTime);

	UFUNCTION(Server, Reliable)
	void ServerScoreRequest(ABlasterCharacter* HitCharacter, const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize& TraceEnd, float HitTime, class AWeapon* DamageCauser);

	UFUNCTION(Server, Reliable)
	void ServerShotgunScoreRequest(const TArray<ABlasterCharacter*>& HitCharacters, const FVector_NetQuantize& TraceStart, const TArray<FVector_NetQuantize>& TraceEnds,
		float HitTime, class AShotgun* DamageCauser);
	/*
	 * Projectile Server side rewind
	 */

	FServerSideRewindResult ProjectileServerSideRewind(ABlasterCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize100& InitialVelocity,
		float HitTime);

	FServerSideRewindResult ProjectileConfirmHit( FFramePackage FrameToCheck,
		ABlasterCharacter* HitCharacter, 
		const FVector_NetQuantize& TraceStart, 
		const FVector_NetQuantize100& InitialVelocity,
		float HitTime);

	UFUNCTION(Server, Reliable)
	void ProjectileServerScoreRequest(ABlasterCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize100& InitialVelocity,
		float HitTime);

protected:
	
	virtual void BeginPlay() override;

	void SaveFramePackage(FFramePackage& Package);
	void SaveFrameData();

	FFramePackage InterpBetweenFrames(const FFramePackage& Older, const FFramePackage& Newer, float HitTime);
	
	FServerSideRewindResult ConfirmHit(
		const FFramePackage& Package, 
		ABlasterCharacter* HitCharacter, 
		const FVector_NetQuantize& TraceStart, 
		const FVector_NetQuantize& HitLocation);
	
	void CacheBoxPositions(ABlasterCharacter* HitCharacter, FFramePackage& OutFramePackage);

	void MoveBoxes(ABlasterCharacter* HitCharacter, const FFramePackage& InFramePackage);

	void ResetHitBoxes(ABlasterCharacter* HitCharacter, const FFramePackage& InFramePackage);

	void EnableCharacterMeshCollision(ABlasterCharacter* HitCharacter, ECollisionEnabled::Type NewType);
	
	FFramePackage GetFrameToCheck(ABlasterCharacter* HitCharacter, float HitTime);

	/*
	 * Shotgun
	 */

	FServerSideShotgunResult ShotgunServerSideRewind(const TArray<ABlasterCharacter*>& HitCharacters, const FVector_NetQuantize& TraceStart,
		const TArray<FVector_NetQuantize>& TraceEnd, float HitTime, AShotgun* DamageCauser);

	FServerSideShotgunResult ShotgunConfirmHit(const TArray<FFramePackage>& FramesToCheck, const FVector_NetQuantize& TraceStart,
		const TArray<FVector_NetQuantize>& TraceEnd, AShotgun* DamageCauser);



	

public:	
	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	
private:
	UPROPERTY()
	ABlasterCharacter* Character;

	UPROPERTY()
	ABlasterPlayerController* CharacterController;

	TDoubleLinkedList<FFramePackage> FrameHitBoxHistory; //Tracks the hit box history of the character

	UPROPERTY(EditAnywhere)
	float MaxRecordTime = 4.f; // The oldest frame we wish to keep
};
