// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatComponent.h"
#include "Blaster/Weapon/Weapon.h"
#include "Blaster/BlasterCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Components/SphereComponent.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "DrawDebugHelpers.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
//#include "Blaster/HUD/BlasterHUD.h"
#include "Camera/CameraComponent.h"
#include "Sound/SoundCue.h"
#include "TimerManager.h"
#include "Blaster/Blaster.h"
#include "Blaster/Weapon/Shotgun.h"


UCombatComponent::UCombatComponent()
{
	
	PrimaryComponentTick.bCanEverTick = true;

	BaseWalkSpeed = 600.f;
	AimWalkSpeed = 450.f;
	
}

void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UCombatComponent, EquippedWeapon);
	DOREPLIFETIME(UCombatComponent, bAiming);
	DOREPLIFETIME(UCombatComponent, CombatState);
	DOREPLIFETIME_CONDITION(UCombatComponent, CarriedAmmo, COND_OwnerOnly);
	DOREPLIFETIME(UCombatComponent, NumGrenades);
	DOREPLIFETIME(UCombatComponent, SecondaryWeapon);
	DOREPLIFETIME(UCombatComponent, bHoldingFlag);
	DOREPLIFETIME(UCombatComponent, FireRateFactor);
}


// Called when the game starts
void UCombatComponent::BeginPlay()
{
	
	Super::BeginPlay();
	if (Character) {
		Character->GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;

		if (Character->FollowCamera) {
			DefaultFOV = Character->FollowCamera->FieldOfView;
			CurrentFOV = DefaultFOV;
		}
		if(Character->HasAuthority())
		{
			InitializeAmmoStore();
		}
		CombatState = ECombatState::ECS_Unoccupied;
	}

	
	
	
}

void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	
	if (Character && Character->IsLocallyControlled()) {
		TraceUnderCrosshairs(HitResult, IHitResult);
		HitTarget = HitResult.ImpactPoint;
		SetHUDCrosshairs(DeltaTime);
		InterpFOV(DeltaTime);
	}
	
}

void UCombatComponent::SetHUDCrosshairs(float DeltaTime)
{
	if (Character == nullptr || Character->Controller == nullptr) return;

	Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;

	if (Controller) {
		HUD = HUD == nullptr ? Cast<ABlasterHUD>(Controller->GetHUD()) : HUD;
		if (HUD) {
	
			if (EquippedWeapon) {
				HUDPackage.CrosshairsCenter = EquippedWeapon->CrosshairsCentre;
				HUDPackage.CrosshairsLeft = EquippedWeapon->CrosshairsLeft;
				HUDPackage.CrosshairsRight = EquippedWeapon->CrosshairsRight;
				HUDPackage.CrosshairsBottom = EquippedWeapon->CrosshairsBottom;
				HUDPackage.CrosshairsTop = EquippedWeapon->CrosshairsTop;
				
			}
			else {
				HUDPackage.CrosshairsCenter = nullptr;
				HUDPackage.CrosshairsLeft = nullptr;
				HUDPackage.CrosshairsRight = nullptr;
				HUDPackage.CrosshairsBottom = nullptr;
				HUDPackage.CrosshairsTop = nullptr;
			}
			// Calculate Spread

			FVector2D WalkSpeedRange(0.f, Character->GetCharacterMovement()->MaxWalkSpeed);
			FVector2D VelocityMultiplierRange(0.f, 1.f);
			FVector Velocity = Character->GetVelocity();
			Velocity.Z = 0.f;

			CrosshairVelocityFactor = FMath::GetMappedRangeValueClamped(WalkSpeedRange, VelocityMultiplierRange, Velocity.Size());

			if (Character->GetCharacterMovement()->IsFalling()) {
				CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 2.25f, DeltaTime, 10.f);
			}
			else {
				CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 0.f, DeltaTime, 30.f);
			}
			if (bAiming) {
				CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.2f, DeltaTime, 30.f);
			}
			else {
				CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.f, DeltaTime, 30.f);
			}

			CrosshairShootingFactor = FMath::FInterpTo(CrosshairShootingFactor, 0.f, DeltaTime, 30.f);

			HUDPackage.CrosshairSpread = CrosshairVelocityFactor +
				CrosshairInAirFactor -
				CrosshairAimFactor +
				CrosshairShootingFactor +
			    CrosshairEnemyFactor;

			HUD->SetHUDPackage(HUDPackage);
		}
	}
}


void UCombatComponent::PlayWeaponEquipSound(AWeapon* WeaponToEquip)
{
	if(Character && WeaponToEquip && WeaponToEquip->EquipSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, WeaponToEquip->EquipSound, Character->GetActorLocation());
	}
}

void UCombatComponent::ReloadIfEmpty()
{
	if(EquippedWeapon->IsEmpty())
	{
		Reload();
	}
}

void UCombatComponent::EquipWeapon(AWeapon* WeaponToEquip) {
	if (Character == nullptr || WeaponToEquip == nullptr) {
		return;
	}
	if(CombatState != ECombatState::ECS_Unoccupied)
	{
		return;
	}
	if(WeaponToEquip->GetWeaponType() == EWeaponType::EWT_Flag)
	{
		Character->GetCharacterMovement()->bOrientRotationToMovement = true;
		Character->bUseControllerRotationYaw = true;
		bHoldingFlag = true;
		WeaponToEquip->SetWeaponState(EWeaponState::EWS_Equipped);
		AttachFlag(WeaponToEquip);
		Character->Crouch();
		WeaponToEquip->SetOwner(Character);
		TheFlag = WeaponToEquip;
		if(Controller)
		{
			 Controller->FlagTipText();
		}
		
	}
	else
	{
		if(EquippedWeapon != nullptr && SecondaryWeapon == nullptr)
		{
			EquipSecondaryWeapon(WeaponToEquip);
		}else
		{
			EquipPrimaryWeapon(WeaponToEquip);
		}
		Character->GetCharacterMovement()->bOrientRotationToMovement = false;
		Character->bUseControllerRotationYaw = true;
	}
	ReloadIfEmpty();
}

void UCombatComponent::SwapWeapons()
{
	if(Character->GetCombatState() != ECombatState::ECS_Unoccupied || Character == nullptr) return;
	Character->PlaySwapMontage();
	CombatState = ECombatState::ECS_SwappingWeapons;
	Character->bFinishedSwapping = false;
	if(SecondaryWeapon) SecondaryWeapon->EnableCustomDepth(false);
	AWeapon* TempWeapon = EquippedWeapon;
	EquippedWeapon = SecondaryWeapon;
	SecondaryWeapon = TempWeapon;
	
	
}

void UCombatComponent::EquipSecondaryWeapon(AWeapon* WeaponToEquip)
{
	SecondaryWeapon = WeaponToEquip;
	if(SecondaryWeapon == nullptr) return;
	SecondaryWeapon->SetWeaponState(EWeaponState::EWS_Secondary);
	SecondaryWeapon->SetOwner(Character);
	PlayWeaponEquipSound(SecondaryWeapon);
	AttachActorToBackpack(WeaponToEquip);
	SecondaryWeapon->SetOwner(Character);
	
}

bool UCombatComponent::ShouldSwapWeapons()
{
	return (EquippedWeapon != nullptr && SecondaryWeapon != nullptr);
}

void UCombatComponent::EquipPrimaryWeapon(AWeapon* WeaponToEquip)
{
	
	DropEquippedWeapon();

	EquippedWeapon = WeaponToEquip;
	if(EquippedWeapon == nullptr) return;
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);

	AttachActorToRightHand(EquippedWeapon);

	EquippedWeapon->SetOwner(Character);
	EquippedWeapon->SetHUDAmmo();
	EquippedWeapon->EnableCustomDepth(false);

	UpdateCarriedAmmo();
	
	PlayWeaponEquipSound(WeaponToEquip);
}

void UCombatComponent::DropEquippedWeapon()
{
	if(EquippedWeapon != nullptr)
	{
		EquippedWeapon->Dropped();
	}
}

void UCombatComponent::AttachActorToRightHand(AActor* ActorToAttatch)
{
	if(Character == nullptr || ActorToAttatch == nullptr || Character->GetMesh() == nullptr) return;
	const USkeletalMeshSocket* WeaponSocket = Character->GetMesh()->GetSocketByName(FName("RightHandSocket"));
	
	if (WeaponSocket) {
		WeaponSocket->AttachActor(ActorToAttatch, Character->GetMesh());
	}
}

void UCombatComponent::AttachActorToLeftHand(AActor* ActorToAttach)
{
	if(Character == nullptr || ActorToAttach == nullptr || Character->GetMesh() == nullptr || EquippedWeapon == nullptr) return;
	bool bUsePistolSocket = EquippedWeapon->GetWeaponType() == EWeaponType::EWT_Pistol || EquippedWeapon->GetWeaponType() == EWeaponType::EWT_SubmachineGun;
	
	FName SocketName = bUsePistolSocket ? FName("PistolSocket") : FName("LeftHandSocket");
	
	const USkeletalMeshSocket* WeaponSocket = Character->GetMesh()->GetSocketByName(SocketName);
	
	if (WeaponSocket) {
		WeaponSocket->AttachActor(ActorToAttach, Character->GetMesh());
	}
}

void UCombatComponent::AttachFlag(AWeapon* Flag)
{
	if(Character == nullptr || Flag == nullptr || Character->GetMesh() == nullptr) return;
	const USkeletalMeshSocket* WeaponSocket = Character->GetMesh()->GetSocketByName(FName("FlagSocket"));
	
	if (WeaponSocket) {
		WeaponSocket->AttachActor(Flag, Character->GetMesh());
	}
}

void UCombatComponent::AttachActorToBackpack(AActor* ActorToAttach)
{
	if(Character == nullptr || ActorToAttach == nullptr || Character->GetMesh() == nullptr) return;
	
	
	FName SocketName = FName("SecondarySocket");
	
	const USkeletalMeshSocket* WeaponSocket = Character->GetMesh()->GetSocketByName(SocketName);
	
	if (WeaponSocket) {
		WeaponSocket->AttachActor(ActorToAttach, Character->GetMesh());
	}
}

void UCombatComponent::UpdateCarriedAmmo()
{
	if(EquippedWeapon == nullptr) return;
	if(CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
	}

	Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
	if(Controller)
	{
		Controller->SetHUDCarriedAmmo(CarriedAmmo);
	}
}

void UCombatComponent::Reload()
{
	if(CarriedAmmo > 0 && CombatState == ECombatState::ECS_Unoccupied && EquippedWeapon && !EquippedWeapon->IsFull() && !bLocallyReloading)
	{
		ServerReload();
		HandleReload();
		bLocallyReloading = true;
	}
}

void UCombatComponent::ShotgunShellReload()
{
	if(Character && Character->HasAuthority())
	{
		UpdateShotgunAmmoValues();
	}
}

void UCombatComponent::UpdateShotgunAmmoValues()
{
	if(Character == nullptr || EquippedWeapon == nullptr) return;
	if(CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		CarriedAmmoMap[EquippedWeapon->GetWeaponType()] -= 1;
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
	}
	Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
	if(Controller)
	{
		Controller->SetHUDCarriedAmmo(CarriedAmmo);
	}
	EquippedWeapon->IncreaseAmmo(1);
	bCanFire = true;
	if(EquippedWeapon->IsFull() || CarriedAmmo == 0)
	{
		JumpToShotgunEnd();
	}
}

void UCombatComponent::JumpToShotgunEnd()
{
	UAnimInstance* AnimInstance = Character->GetMesh()->GetAnimInstance();
	if(AnimInstance && Character->GetReloadMontage())
	{
		AnimInstance->Montage_JumpToSection(FName("ShotgunEnd"));
	}
}

void UCombatComponent::ThrowGrenadeFinished()
{
	CombatState = ECombatState::ECS_Unoccupied;
	if(Character)
	{
		Character->ShowGrenade(false);
		if(Character && Character->IsLocallyControlled())
		{
			ServerTossGrenade(HitTarget);
		}
		
	}
	AttachActorToRightHand(EquippedWeapon);
}

void UCombatComponent::PickupAmmo(EWeaponType AmmoType, int32 AmmoAmmount)
{
	if(CarriedAmmoMap.Contains(AmmoType))
	{
		CarriedAmmoMap[AmmoType] += AmmoAmmount;
		UpdateCarriedAmmo();
	}else
	{
		CarriedAmmoMap.Emplace(AmmoType, AmmoAmmount);
		UpdateCarriedAmmo();
	}
	
	if(EquippedWeapon && EquippedWeapon->IsEmpty() && AmmoType == EquippedWeapon->GetWeaponType())
	{
		Reload();
	}
}

void UCombatComponent::OnRep_Aiming()
{
	if(Character && Character->IsLocallyControlled())
	{
		bAiming = bAimButtonPressed;
	}
}

void UCombatComponent::ServerReload_Implementation()
{
	if(Character == nullptr || EquippedWeapon == nullptr) return;

	if(!Character->IsLocallyControlled())
	{
		HandleReload();
	}
	
	CombatState = ECombatState::ECS_Reloading;
	
}



void UCombatComponent::FinishReloading()
{
	if(Character == nullptr) return;
	bLocallyReloading = false;
	if(Character->HasAuthority())
	{
		CombatState = ECombatState::ECS_Unoccupied;
		int32 ReloadAmount = AmountToReload();
		if(CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
		{
			CarriedAmmoMap[EquippedWeapon->GetWeaponType()] -= ReloadAmount;
			CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
		}
		EquippedWeapon->IncreaseAmmo(ReloadAmount);
		Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
		if(Controller)
		{
			Controller->SetHUDCarriedAmmo(CarriedAmmo);
		}
	}
	if(bFireButtonPressed)
	{
		Fire();
	}
	
}

void UCombatComponent::FinishSwap()
{
	if(Character && Character->HasAuthority())
	{
		CombatState = ECombatState::ECS_Unoccupied;
	}
	if(Character)
	{
		Character->bFinishedSwapping = true;
	}
	if(SecondaryWeapon) SecondaryWeapon->EnableCustomDepth(true);
}

void UCombatComponent::FinishSwapAttachWeapons()
{
	if(Character && Character->HasAuthority())
	{
		PlayWeaponEquipSound(EquippedWeapon);
		EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
		SecondaryWeapon->SetWeaponState(EWeaponState::EWS_Secondary);
		EquippedWeapon->SetHUDAmmo();
		UpdateCarriedAmmo();
		AttachActorToRightHand(EquippedWeapon);
		AttachActorToBackpack(SecondaryWeapon);
	}
	
	
}

void UCombatComponent::OnRep_CombatState()
{
	
	switch(CombatState)
	{
	case ECombatState::ECS_Reloading:
		if(Character && !Character->IsLocallyControlled()) HandleReload();
		break;
	case ECombatState::ECS_Unoccupied:
		if(bFireButtonPressed)
		{
			Fire();
		}
		break;
	case ECombatState::ECS_ThrowingGrenade:
		if(Character && !Character->IsLocallyControlled())
		{
			Character->ShowGrenade(true);
			Character->PlayThrowGrenadeMontage();
			AttachActorToLeftHand(EquippedWeapon);
		}
		break;
	case ECombatState::ECS_SwappingWeapons:
		if(Character && !Character->IsLocallyControlled())
		{
			Character->PlaySwapMontage();
		}
	}	
}

void UCombatComponent::OnRep_Grenades()
{
	UpdateHUDGrenades();
}

void UCombatComponent::UpdateHUDGrenades()
{
	Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
	if(Controller)
	{
		Controller->SetHUDGrenadesText(NumGrenades);
	}
}

void UCombatComponent::OnRep_HoldingFlag()
{
	if(bHoldingFlag && Character && Character->IsLocallyControlled())
	{
		Character->Crouch();
		if(Controller)
		{
			Controller->FlagTipText();
		}
	}
	if(!bHoldingFlag && Controller)
	{
		Controller->StopFlagTipText();
	}
}

void UCombatComponent::HandleReload()
{
	if(Character)
	{
		Character->PlayReloadMontage();
	}
	
	
}

int32 UCombatComponent::AmountToReload()
{
	if(EquippedWeapon == nullptr) return 0;

	int32 RoomInMag = EquippedWeapon->MaxAmmo1() - EquippedWeapon->CurrentAmmo1();

	if(CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		int32 AmountCarried = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
		int32 Least = FMath::Min(RoomInMag, AmountCarried);
		return FMath::Clamp(RoomInMag, 0, Least);
	}
	return 0;
}

void UCombatComponent::ThrowGrenade()
{
	if(CombatState != ECombatState::ECS_Unoccupied || NumGrenades <= 0)
	{
		return;
	}
	CombatState = ECombatState::ECS_ThrowingGrenade;
	if(Character)
	{
		Character->PlayThrowGrenadeMontage();
		Character->ShowGrenade(true);
		AttachActorToLeftHand(EquippedWeapon);
	}
	if(Character && !Character->HasAuthority())
	{
		ServerThrowGrenade();
	}
	if(Character && Character->HasAuthority())
	{
		NumGrenades = FMath::Clamp(NumGrenades - 1, 0, MaxGrenades);
		UpdateHUDGrenades();
	}
	
}

void UCombatComponent::ServerTossGrenade_Implementation(const FVector_NetQuantize& Target)
{
	if(Character && GrenadeClass && Character->GetGrenadeMesh())
	{
		const FVector StartingLocation = Character->GetGrenadeMesh()->GetComponentLocation();
		FVector ToTarget = Target - StartingLocation;
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = Character;
		SpawnParams.Instigator = Character;
		UWorld* World = GetWorld();
		if(World)
		{
			World->SpawnActor<AProjectile>(GrenadeClass, StartingLocation, ToTarget.Rotation(),
				SpawnParams);
		}
	}
}

void UCombatComponent::ServerThrowGrenade_Implementation()
{
	if(NumGrenades <= 0) return;
	CombatState = ECombatState::ECS_ThrowingGrenade;
	if(Character)
	{
		Character->ShowGrenade(true);
		AttachActorToLeftHand(EquippedWeapon);
		Character->PlayThrowGrenadeMontage();
	}
	NumGrenades = FMath::Clamp(NumGrenades - 1, 0, MaxGrenades);
}

void UCombatComponent::OnRep_EquippedWeapon() {
	
	if (EquippedWeapon && Character) {
		EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);

		AttachActorToRightHand(EquippedWeapon);

		PlayWeaponEquipSound(EquippedWeapon);
		
		Character->GetCharacterMovement()->bOrientRotationToMovement = false;
		Character->bUseControllerRotationYaw = true;
		EquippedWeapon->EnableCustomDepth(false);
		EquippedWeapon->SetHUDAmmo();
		
	}
}

void UCombatComponent::OnRep_SecondaryWeapon()
{
	if(Character && SecondaryWeapon)
	{
		AttachActorToBackpack(SecondaryWeapon);
		PlayWeaponEquipSound(SecondaryWeapon);
		SecondaryWeapon->SetWeaponState(EWeaponState::EWS_Secondary);
	}
	
}

void UCombatComponent::SetAiming(bool bIsAiming) {
	if(Character == nullptr || EquippedWeapon == nullptr) return;
	
	bAiming = bIsAiming;
	if(Character->IsLocallyControlled())
	{
		bAimButtonPressed = bIsAiming;
	}
	
	ServerSetAiming(bIsAiming);
	if (Character) {
		Character->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed;
	}

	if(Character->IsLocallyControlled() && EquippedWeapon->GetWeaponType() == EWeaponType::EWT_SniperRifle)
	{
		Character->ShowSniperScopeWidget(bIsAiming);
	}
}

void UCombatComponent::ServerSetAiming_Implementation(bool bIsAiming)
{
	bAiming = bIsAiming;
	if (Character) {
		Character->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed;
	}
}

void UCombatComponent::OnRep_CarriedAmmo()
{
	Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
	if(Controller)
	{
		Controller->SetHUDCarriedAmmo(CarriedAmmo);
	}
	bool bJumpToShotgunEnd = CombatState == ECombatState::ECS_Reloading && EquippedWeapon != nullptr
	&& EquippedWeapon->GetWeaponType() == EWeaponType::EWT_Shotgun && CarriedAmmo == 0;
	if(bJumpToShotgunEnd)
	{
		JumpToShotgunEnd();
	}
	
}

void UCombatComponent::FireButtonPressed(bool bPressed)
{
	bFireButtonPressed = bPressed;
	if (bFireButtonPressed) {
		Fire();
	}
}

void UCombatComponent::Fire()
{
	if(EquippedWeapon == nullptr) return;
	
	if (CanFire()) {
		
		bCanFire = false;
		if (EquippedWeapon) {
			CrosshairShootingFactor = 0.2f;
			switch(EquippedWeapon->FireType)
			{
			case EFireType::EFT_Projectile:
				FireProjectileWeapon();
				break;
			case EFireType::EFT_HitScan:
				FireHitScanWeapon();
				break;
			case EFireType::EFT_Shotgun:
				FireShotgun();
				break;
			}
		}
		StartFireTimer();
	}
}

void UCombatComponent::FireProjectileWeapon()
{
	if(!Character->HasAuthority() && EquippedWeapon)
	{
		HitTarget = EquippedWeapon->bUseScatter ? EquippedWeapon->TraceEndWithScatter(HitTarget) : HitTarget;
		LocalFire(HitTarget);
	}
	ServerFire(HitTarget);
}

void UCombatComponent::FireHitScanWeapon()
{
	
	if(EquippedWeapon && Character)
	{
		HitTarget = EquippedWeapon->bUseScatter ? EquippedWeapon->TraceEndWithScatter(HitTarget) : HitTarget;
		if(!Character->HasAuthority())
		{
			LocalFire(HitTarget);
		}
		ServerFire(HitTarget);
	}
}

void UCombatComponent::FireShotgun()
{
    AShotgun* Shotgun = Cast<AShotgun>(EquippedWeapon);
    if(Shotgun && Character)
    {
    	TArray<FVector_NetQuantize> PelletDestinations;
    	Shotgun->ShotgunTraceEndWithScatter(HitTarget, PelletDestinations);
    	if(!Character->HasAuthority())
    	{
    		LocalShotgunFire(PelletDestinations);
    	}
    	ServerShotgunFire(PelletDestinations);
    }
}
void UCombatComponent::MulticastShotgunFire_Implementation(const TArray<FVector_NetQuantize>& TraceHitTargets)
{
	if(Character && Character->IsLocallyControlled() && !Character->HasAuthority()) return; // If this character is an autonomous proxy
	LocalShotgunFire(TraceHitTargets);
}

void UCombatComponent::ServerShotgunFire_Implementation(const TArray<FVector_NetQuantize>& TraceHitTargets)
{
	MulticastShotgunFire(TraceHitTargets);
}

void UCombatComponent::LocalFire(const FVector_NetQuantize& TraceHitTarget)
{
	if (EquippedWeapon == nullptr) return;
	if (Character && CombatState == ECombatState::ECS_Unoccupied){
		
		Character->PlayFireMontage(bAiming);
		
		EquippedWeapon->Fire(TraceHitTarget);
	}
	
}

void UCombatComponent::LocalShotgunFire(const TArray<FVector_NetQuantize>& TraceHitTargets)
{
	AShotgun* Shotgun = Cast<AShotgun>(EquippedWeapon);
	if(Shotgun == nullptr || Character == nullptr) return;
	if(CombatState == ECombatState::ECS_Reloading || CombatState == ECombatState::ECS_Unoccupied)
	{
		Character->PlayFireMontage(bAiming);
		Shotgun->FireShotgun(TraceHitTargets);
		CombatState = ECombatState::ECS_Unoccupied;
		
	}
}

void UCombatComponent::MulticastFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	if(Character && Character->IsLocallyControlled() && !Character->HasAuthority()) return;
	LocalFire(TraceHitTarget);
}

void UCombatComponent::ServerFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	MulticastFire(TraceHitTarget);
}

void UCombatComponent::StartFireTimer()
{
	if (EquippedWeapon == nullptr || Character == nullptr) return;

	Character->GetWorldTimerManager().SetTimer(FireTimer, this, &UCombatComponent::FireTimerFinished, EquippedWeapon->FireDelay/FireRateFactor);
}

void UCombatComponent::FireTimerFinished()
{
	if (EquippedWeapon == nullptr) return;

	bCanFire = true;
	if (bFireButtonPressed && EquippedWeapon->bAutomatic && !Character->GetCharacterMovement()->IsFalling()) {
		
		Fire();
	}
	ReloadIfEmpty();
}

bool UCombatComponent::CanFire()
{
	if(EquippedWeapon == nullptr)
	{
		return false;
	}

	if(!EquippedWeapon->IsEmpty() && bCanFire && EquippedWeapon->GetWeaponType() == EWeaponType::EWT_Shotgun && CombatState == ECombatState::ECS_Reloading)
	{
		return true;
	}
	return !EquippedWeapon->IsEmpty() && bCanFire && CombatState == ECombatState::ECS_Unoccupied && !bLocallyReloading;
}

void UCombatComponent::InitializeAmmoStore()
{
	CarriedAmmoMap.Emplace(EWeaponType::EWT_AssaultRifle, AssaultRifleAmmoInit);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_RocketLauncher, StartingRocketAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_Pistol, StartingPistolAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_SubmachineGun, StartingSMGAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_Shotgun, StartingShotgunAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_SniperRifle, StartingSniperAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_GrenadeLauncher, StartingGrenadeLauncherAmmo);
}


void UCombatComponent::InterpFOV(float DeltaTime)
{
	if (EquippedWeapon == nullptr) return;

	if (bAiming) {
		CurrentFOV = FMath::FInterpTo(CurrentFOV, EquippedWeapon->GetZoomedFOV(), DeltaTime, EquippedWeapon->GetZoomInterpSpeed());
	}
	else {
		CurrentFOV = FMath::FInterpTo(CurrentFOV, DefaultFOV, DeltaTime, ZoomInterpSpeed);
	}
	if (Character && Character->FollowCamera) {
		Character->FollowCamera->SetFieldOfView(CurrentFOV);
	}
}

void UCombatComponent::TraceUnderCrosshairs(FHitResult& TraceHitResult, FHitResult& InteractableHitResult)
{
	FVector2D ViewportSize;
	if (GEngine && GEngine->GameViewport) {
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}
	if(Character && Character->GetMesh())
	{
		Character->GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECR_Ignore);
	}

	FVector2D CrosshairLocation(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);
	FVector CrosshairWorldPosition;
	FVector CrosshairWorldDirection;
	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(
		UGameplayStatics::GetPlayerController(this, 0),
		CrosshairLocation,
		CrosshairWorldPosition,
		CrosshairWorldDirection
	);
	if (TraceHitResult.GetActor() && TraceHitResult.GetActor()->Implements<UInteractWithCrosshairsInterface>()) {
		
		HUDPackage.CrosshairsColor = FLinearColor::Red;
		CrosshairEnemyFactor = -0.2f;
	}
	else {

		HUDPackage.CrosshairsColor = FLinearColor::White;
		HUDPackage.CrosshairSpread = 0.f;
	}

	
	if (bScreenToWorld) {
		FVector Start = CrosshairWorldPosition + CrosshairWorldDirection * 50.f;

		if (Character)
		{
			float DistanceToCharacter = (Character->GetActorLocation() - Start).Size();
			Start += CrosshairWorldDirection * DistanceToCharacter;
			
		}

		FVector End = Start + CrosshairWorldDirection * TRACE_LENGTH;
		
		if(!GetWorld()->LineTraceSingleByChannel(TraceHitResult, Start, End, ECollisionChannel::ECC_Visibility))
		{
			TraceHitResult.ImpactPoint = End;
		}
		if(!GetWorld()->LineTraceSingleByChannel(InteractableHitResult, Start, TraceHitResult.ImpactPoint, ECC_Interactable))
		{
			InteractableHitResult.ImpactPoint = End;
		}
		
		IInteractables* LineOfSightInteractable = Cast<IInteractables>(InteractableHitResult.GetActor());
		if(LineOfSightInteractable)
		{
			Character->SetOverlappingInteractable(LineOfSightInteractable);
		}else
		{
			Character->SetOverlappingInteractable(nullptr);
		}
	
	}
	Character->GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECR_Block);


}


