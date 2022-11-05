// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blaster/BlasterTypes/Team.h"
#include "Blaster/Interfaces/Interactables.h"

#include "GameFramework/Actor.h"
#include "Weapon.generated.h"

class USphereComponent;
enum class EWeaponType : uint8_t;
UENUM(BlueprintType)
enum class EWeaponState : uint8
{
	EWS_Initial UMETA(DisplayName = "Initial State"),
	EWS_Equipped UMETA(DisplayName = "Equipped"),
	EWS_Dropped UMETA(DisplayName = "Dropped"),
	EWS_Secondary UMETA(DisplayName = "Seconday"),
	EWS_MAX UMETA(DisplayName = "DefaultMAX")
};

UENUM(BlueprintType)
enum class EFireType : uint8
{
	EFT_HitScan UMETA(DisplayName = "Hit Scan Weapon"),
	EFT_Projectile UMETA(DisplayName = "Projectile Weapon"),
	EFT_Shotgun UMETA(DisplayName = "Shotgun"),
	EFT_MAX UMETA(DisplayName = "DefaultMAX")
};

UCLASS()
class BLASTER_API AWeapon : public AActor, public IInteractables
{
	GENERATED_BODY()
	
public:
	AWeapon();
	virtual void Tick(float DeltaTime) override;
	void ShowPickupWidget(bool bShowWidget);
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void Fire(const FVector& HitTarget);

	virtual void Dropped();

	void SpendRound();
	void IncreaseAmmo(int32 increase);
	void SetHUDAmmo();
	bool IsEmpty();
	bool IsFull();

	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
	bool  bUseScatter;

protected:
	virtual void BeginPlay() override;
	virtual void HandleWeaponEquipped();
	virtual void HandleWeaponDropped();
	virtual void HandleWeaponSecondary();
	virtual void OnWeaponState();

	UFUNCTION()
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, 
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	UFUNCTION()
		void OnSphereEndOverlap(
			UPrimitiveComponent* OverlappedComponent,
			AActor* OtherActor,
			UPrimitiveComponent* OtherComp,
			int32 OtherBodyIndex
		);

	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
		class UAnimationAsset* FireAnimation;

	UPROPERTY(EditAnywhere)
	float damage = 10.f;

	UPROPERTY(EditAnywhere)
	float HeadshotDamage = 50.f;

	UPROPERTY(EditAnywhere, Replicated)
	bool bUseServerSideRewind = false;

	UPROPERTY()
	class ABlasterCharacter* BlasterOwnerCharacter;

	UPROPERTY()
	class ABlasterPlayerController* BlasterOwnerController;

	UFUNCTION()
	void OnPingTooHigh(bool bHighPing);

	FORCEINLINE USphereComponent* GetAreaSphere() const { return AreaSphere; }
	FORCEINLINE USphereComponent* GetInteractableSphere() const { return InteractableSphere; }

	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	class UWidgetComponent* PickupWidget;

	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	class USphereComponent* InteractableSphere;

public:	
	void SetWeaponState(EWeaponState State);

	FORCEINLINE bool IsNotEquipped() { return WeaponState != EWeaponState::EWS_Equipped; }
	FORCEINLINE USkeletalMeshComponent* GetWeaponMesh() const { return WeaponMesh; }
	FORCEINLINE float GetZoomedFOV() const { return ZoomedFOV; }
	FORCEINLINE float GetZoomInterpSpeed() const { return ZoomInterpSpeed; }
	FORCEINLINE EWeaponType GetWeaponType() const { return WeaponType; }
	FORCEINLINE float GetDamage() const {return damage;}
	FORCEINLINE float GetHeadshotDamage() const { return HeadshotDamage; }

	/*
	* Textures for the weapon crosshairs
	*/

	UPROPERTY(EditAnywhere, Category = Crosshairs)
		class UTexture2D* CrosshairsLeft;

	UPROPERTY(EditAnywhere, Category = Crosshairs)
		UTexture2D* CrosshairsRight;

	UPROPERTY(EditAnywhere, Category = Crosshairs)
		UTexture2D* CrosshairsTop;

	UPROPERTY(EditAnywhere, Category = Crosshairs)
		UTexture2D* CrosshairsBottom;

	UPROPERTY(EditAnywhere, Category = Crosshairs)
		UTexture2D* CrosshairsCentre;

	/**
	* Zoomed FOV while aiming
	*/

	UPROPERTY(EditAnywhere)
		float ZoomedFOV = 30.f;
	UPROPERTY(EditAnywhere)
		float ZoomInterpSpeed = 20.f;

	UPROPERTY(EditAnywhere, Category = "WeaponRotationCorrection")
		float RightHandRotationRoll;
	UPROPERTY(EditAnywhere, Category = "WeaponRotationCorrection")
		float RightHandRotationYaw;
	UPROPERTY(EditAnywhere, Category = "WeaponRotationCorrection")
		float RightHandRotationPitch;

	virtual void BecameOverlappingInteractable() override;

	virtual void StoppedBeingOverlappingInteractable() override;
	
	virtual void InteractedWith(AActor* Interactor) override;

	/*
	* The weapon's fire time and automatic status
	*/

	UPROPERTY(EditAnywhere, Category = Combat);
	float FireDelay = 0.15f;

	UPROPERTY(EditAnywhere, Category = Combat);
	bool bAutomatic = true;

	bool bDestroyWeapon = false;

	UPROPERTY(EditAnywhere)
	EFireType FireType;

private:
	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	class USkeletalMeshComponent* WeaponMesh;

	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	class USphereComponent* AreaSphere;

	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	int32 CurrentAmmo;

	int32 Sequence = 0; //Number of unprocessed server requests for ammo
	//Incremented in SpendRound, Decremented in ClientUpdateAmmo

	UFUNCTION(Client, Reliable)
	void ClientUpdateAmmo(int32 ServerAmmo);

	UFUNCTION(Client, Reliable)
	void ClientAddAmmo(int32 AmmoToAdd);

	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	int32 MaxAmmo;
public:
	[[nodiscard]] int32 CurrentAmmo1() const
	{
		return CurrentAmmo;
	}

	[[nodiscard]] int32 MaxAmmo1() const
	{
		return MaxAmmo;
	}

	UPROPERTY(EditAnywhere)
	class USoundCue* EquipSound;
	
	void EnableCustomDepth(bool bEnable);
	
	FVector TraceEndWithScatter(const FVector& HitTarget);

	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
	bool IncreaseScatterOverTime = false;
	
	UPROPERTY(EditAnywhere, Category = "Weapon Scatter");
	float DistanceToSphere = 800.f;

	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
	float SphereRadius = 75.f;

	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
	float MinimumSphereRaduis = 75.f;

	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
	float FireSpreadIncrease = 5.f;

	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
	float ResetAccurarySpeed = 5.f;

	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
	float MaximumInnacuracy = 200.f;

	UPROPERTY(EditAnywhere)
	ETeam Team;

	FVector InitialPosition;

private:
	UPROPERTY(ReplicatedUsing = OnRep_WeaponState, VisibleAnywhere, Category = "Weapon Properties")
	EWeaponState WeaponState;

	UFUNCTION()
		void OnRep_WeaponState();
	 
	virtual void OnRep_Owner() override;

	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	EWeaponType WeaponType;

	UPROPERTY(EditAnywhere)
		TSubclassOf<class ABulletShell> BulletCasing;

	void ResetInnacuracy();

	bool bIsPickup = false;

	bool bGoUp = false;

	void PickupMovement(float DeltaTime);

	UPROPERTY(EDITANYWHERE)
	float BaseTurnRate = 45.f;

	UPROPERTY(EDITANYWHERE)
	float BaseLevitateRate = 45.f;

	

	

	
	

};
