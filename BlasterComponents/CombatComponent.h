// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blaster/BlasterTypes/CombatStates.h"
#include "Components/ActorComponent.h"
#include "Blaster/HUD/BlasterHUD.h"
#include "Blaster/Weapon/Projectile.h"
#include "Blaster/Weapon/WeaponTypes.h"
#include "CombatComponent.generated.h"



class AWeapon;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BLASTER_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UCombatComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	friend class ABlasterCharacter;

	void EquipWeapon(AWeapon* WeaponToEquip);
	void SwapWeapons();



protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	void SetAiming(bool bIsAiming);

	UFUNCTION(Server, Reliable)
	void ServerSetAiming(bool bIsAiming);

	UFUNCTION(Server, Reliable)
	void ServerReload();

	void HandleReload();
	
	int32 AmountToReload();

	void ThrowGrenade();

	UFUNCTION(Server, Reliable)
	void ServerThrowGrenade();
	UFUNCTION(Server, Reliable)
	void ServerTossGrenade(const FVector_NetQuantize& Target);

	UPROPERTY(EditAnywhere)
	TSubclassOf<AProjectile> GrenadeClass;

	UFUNCTION()
	void OnRep_EquippedWeapon();

	UFUNCTION()
	void OnRep_SecondaryWeapon();

	UFUNCTION(Server, Reliable)
	void ServerFire(const FVector_NetQuantize& TraceHitTarget);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastFire(const FVector_NetQuantize& TraceHitTarget);

	UFUNCTION(Server, Reliable)
	void ServerShotgunFire(const TArray<FVector_NetQuantize>& TraceHitTargets);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastShotgunFire(const TArray<FVector_NetQuantize>& TraceHitTargets);

	void TraceUnderCrosshairs(FHitResult& TraceHitResult, FHitResult& InteractableHitResult);

	void SetHUDCrosshairs(float DeltaTime);
	void PlayWeaponEquipSound(AWeapon* WeaponToEquip);
	void ReloadIfEmpty();
	void UpdateCarriedAmmo();

	void DropEquippedWeapon();
	void AttachActorToRightHand(AActor* ActorToAttatch);
	void AttachActorToLeftHand(AActor* ActorToAttach);
	void AttachFlag(AWeapon* Flag);
	void AttachActorToBackpack(AActor* ActorToAttach);

	void EquipPrimaryWeapon(AWeapon* WeaponToEquip);

	void EquipSecondaryWeapon(AWeapon* WeaponToEquip);


public:
	bool ShouldSwapWeapons();
	
	UFUNCTION(BlueprintCallable)
	void FinishReloading();

	UFUNCTION(BlueprintCallable)
	void FinishSwap();

	UFUNCTION(BlueprintCallable)
	void FinishSwapAttachWeapons();
	
	UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapon);
	AWeapon* EquippedWeapon;

	UPROPERTY(ReplicatedUsing = OnRep_SecondaryWeapon)
	AWeapon* SecondaryWeapon;

	UPROPERTY()
	class ABlasterCharacter* Character;

	UPROPERTY()
	class ABlasterPlayerController* Controller;

	UPROPERTY()
	class ABlasterHUD* HUD;
	
	void FireButtonPressed(bool bPressed);

	void Fire();
	void FireProjectileWeapon();
	void FireHitScanWeapon();
	void FireShotgun();
	void LocalFire(const FVector_NetQuantize& TraceHitTarget);

	void LocalShotgunFire(const TArray<FVector_NetQuantize>& TraceHitTargets);

	void Reload();

	/**
	*  HUD and Crosshair
	**/
	float CrosshairVelocityFactor;
	float CrosshairInAirFactor;
	float CrosshairAimFactor;
	float CrosshairShootingFactor;
	float CrosshairEnemyFactor;

	FORCEINLINE float GetZoomedFOV() const { return ZoomedFOV;  }
	FORCEINLINE float GetZoomInterpSpeed() const { return ZoomInterpSpeed; }
	FORCEINLINE int32 GetNumGrenades() const { return NumGrenades; }
	

	UFUNCTION(BlueprintCallable)
	void ShotgunShellReload();

	void UpdateShotgunAmmoValues();

	void JumpToShotgunEnd();

	UFUNCTION(BlueprintCallable)
	void ThrowGrenadeFinished();

	void PickupAmmo(EWeaponType AmmoType, int32 AmmoAmmount);

	bool bLocallyReloading = false;

	UPROPERTY(Replicated)
	int32 FireRateFactor = 1;
	
	UPROPERTY(EditAnywhere)
	float BaseWalkSpeed;

private:
	UPROPERTY(ReplicatedUsing = OnRep_Aiming)
	bool bAiming = false;

	bool bAimButtonPressed = false;

	UFUNCTION()
	void OnRep_Aiming();

	UPROPERTY(EditAnywhere)
	float AimWalkSpeed;

	bool bFireButtonPressed;

	FHitResult HitResult;

	FVector HitTarget;

	FHitResult IHitResult;
	

	/**
	* Aiming and FOV
	*/


	//FOV when not aiming, set to camera's base FOV in BeginPlay
	float DefaultFOV;

	UPROPERTY(EditAnywhere, Category = Combat)
	float ZoomedFOV = 30.f;
	
	float CurrentFOV;

	UPROPERTY(EditAnywhere, Category = Combat)
	float ZoomInterpSpeed = 30.f;

	void InterpFOV(float DeltaTime);

	FHUDPackage HUDPackage;

	/*
	* Automatic fire
	*
	*/
	
	FTimerHandle FireTimer;

	

	bool bCanFire = true;

	void StartFireTimer();
	void FireTimerFinished();

	bool CanFire();

	// Carried ammo for the currently equipped ammo
	UPROPERTY(ReplicatedUsing = OnRep_CarriedAmmo)
	int32 CarriedAmmo;

	UFUNCTION()
	void OnRep_CarriedAmmo();

	// TMap's cannot be replicated
	TMap<EWeaponType, int32> CarriedAmmoMap;

	UPROPERTY(EditAnywhere)
    int32 AssaultRifleAmmoInit;

	UPROPERTY(EditAnywhere)
	int32 StartingRocketAmmo = 0;

	UPROPERTY(EditAnywhere)
	int32 StartingPistolAmmo = 0.f;

	UPROPERTY(EditAnywhere, ReplicatedUsing = OnRep_Grenades)
	int32 NumGrenades = 0.f;

	UPROPERTY(EditAnywhere)
	int32 MaxGrenades;

	UPROPERTY(EditAnywhere)
	int32 StartingSMGAmmo = 0.f;

	UPROPERTY(EditAnywhere)
	int32 StartingShotgunAmmo = 0.f;

	UPROPERTY(EditAnywhere)
	int32 StartingSniperAmmo = 0.f;

	UPROPERTY(EditAnywhere)
	int32 StartingGrenadeLauncherAmmo = 0.f;
    
    void InitializeAmmoStore();

	UPROPERTY(ReplicatedUsing = OnRep_CombatState)
	ECombatState CombatState;

	UFUNCTION()
	void OnRep_CombatState();

	UFUNCTION()
	void OnRep_Grenades();

	void UpdateHUDGrenades();

	UPROPERTY(ReplicatedUsing = OnRep_HoldingFlag)
	bool bHoldingFlag;

	UFUNCTION()
	void OnRep_HoldingFlag();

	UPROPERTY()
	AWeapon* TheFlag;
		
};




