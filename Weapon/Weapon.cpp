// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Blaster/BlasterCharacter.h"
#include "Animation/AnimationAsset.h"
#include "Net/UnrealNetwork.h"
#include "BulletShell.h"
#include "Blaster/Blaster.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values
AWeapon::AWeapon()
{
 	
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	SetReplicateMovement(true);
	

	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	WeaponMesh->SetupAttachment(RootComponent);
	SetRootComponent(WeaponMesh);

	WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	WeaponMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_PURPLE);
	WeaponMesh->MarkRenderStateDirty();
	EnableCustomDepth(true);


	AreaSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AreaSphere"));
	AreaSphere->SetupAttachment(RootComponent);
	AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	InteractableSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractableSphere"));
	InteractableSphere->SetupAttachment(RootComponent);
	InteractableSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	InteractableSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	PickupWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("PikcupWidget"));
	PickupWidget->SetupAttachment(RootComponent);

}


void AWeapon::BeginPlay()
{
	Super::BeginPlay();

	if (PickupWidget) {
		PickupWidget->SetVisibility(false);
	}
	
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	AreaSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
	AreaSphere->OnComponentBeginOverlap.AddDynamic(this, &AWeapon::OnSphereOverlap);
	AreaSphere->OnComponentEndOverlap.AddDynamic(this, &AWeapon::OnSphereEndOverlap);

	InteractableSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	InteractableSphere->SetCollisionResponseToChannel(ECC_Interactable, ECollisionResponse::ECR_Block);
	InitialPosition = GetActorLocation();
	if (HasAuthority()) {
		
	}
}

void AWeapon::HandleWeaponEquipped()
{
	ShowPickupWidget(false);
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	InteractableSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	InteractableSphere->SetCollisionResponseToChannel(ECC_Interactable, ECR_Ignore);
	WeaponMesh->SetSimulatePhysics(false);
	WeaponMesh->SetEnableGravity(false);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	if(WeaponType == EWeaponType::EWT_SubmachineGun)
	{
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		WeaponMesh->SetEnableGravity(true);
		WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	}
	EnableCustomDepth(false);

	BlasterOwnerCharacter = BlasterOwnerCharacter == nullptr ? Cast<ABlasterCharacter>(GetOwner()) : BlasterOwnerCharacter;
	
	if(BlasterOwnerCharacter)
	{
		BlasterOwnerController = BlasterOwnerController == nullptr ? Cast<ABlasterPlayerController>(BlasterOwnerCharacter->Controller) : BlasterOwnerController;
		if(BlasterOwnerController && HasAuthority() && !BlasterOwnerController->HighPingDelegate.IsBound() && bUseServerSideRewind)
		{
			BlasterOwnerController->HighPingDelegate.AddDynamic(this, &AWeapon::OnPingTooHigh);
		}
	}
	
}

void AWeapon::HandleWeaponDropped()
{
	
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	
	InteractableSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	InteractableSphere->SetCollisionResponseToChannel(ECC_Interactable, ECollisionResponse::ECR_Block);
	WeaponMesh->SetSimulatePhysics(true);
	WeaponMesh->SetEnableGravity(true);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	WeaponMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_BLUE);
	EnableCustomDepth(true);
	BlasterOwnerCharacter = BlasterOwnerCharacter == nullptr ? Cast<ABlasterCharacter>(GetOwner()) : BlasterOwnerCharacter;
    	
    if(BlasterOwnerCharacter)
    {
    	BlasterOwnerController = BlasterOwnerController == nullptr ? Cast<ABlasterPlayerController>(BlasterOwnerCharacter->Controller) : BlasterOwnerController;
    	if(BlasterOwnerController && HasAuthority() && BlasterOwnerController->HighPingDelegate.IsBound() && bUseServerSideRewind)
    	{
    		BlasterOwnerController->HighPingDelegate.RemoveDynamic(this, &AWeapon::OnPingTooHigh);
    	}
    }

}

void AWeapon::HandleWeaponSecondary()
{
	ShowPickupWidget(false);
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponMesh->SetSimulatePhysics(false);
	WeaponMesh->SetEnableGravity(false);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	InteractableSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	InteractableSphere->SetCollisionResponseToChannel(ECC_Interactable, ECR_Ignore);
	if(WeaponType == EWeaponType::EWT_SubmachineGun)
	{
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		WeaponMesh->SetEnableGravity(true);
		WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	}
	WeaponMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_TAN);
	WeaponMesh->MarkRenderStateDirty();
	

	if(BlasterOwnerCharacter)
	{
		BlasterOwnerController = BlasterOwnerController == nullptr ? Cast<ABlasterPlayerController>(BlasterOwnerCharacter->Controller) : BlasterOwnerController;
		if(BlasterOwnerController && HasAuthority() && BlasterOwnerController->HighPingDelegate.IsBound() && bUseServerSideRewind)
		{
			BlasterOwnerController->HighPingDelegate.RemoveDynamic(this, &AWeapon::OnPingTooHigh);
		}
	}
}

void AWeapon::OnWeaponState()
{
	if(WeaponState == EWeaponState::EWS_Dropped)
	{
		HandleWeaponDropped();
		PrimaryActorTick.bCanEverTick = false;
		bIsPickup = false;
	}else if(WeaponState == EWeaponState::EWS_Equipped)
	{
		HandleWeaponEquipped();
		PrimaryActorTick.bCanEverTick = true;
		bIsPickup = false;
	}else if(WeaponState == EWeaponState::EWS_Initial)
	{
		PrimaryActorTick.bCanEverTick = false;
		bIsPickup = true;
	}else if(WeaponState == EWeaponState::EWS_Secondary)
	{
		HandleWeaponSecondary();
		PrimaryActorTick.bCanEverTick = false;
		bIsPickup = false;
	}
	ResetInnacuracy();
}

void AWeapon::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(OtherActor);
	if (BlasterCharacter) {
		if(WeaponType == EWeaponType::EWT_Flag)
		{
			if(BlasterCharacter->GetTeam() == Team) return;
		}
		if(BlasterCharacter->IsHoldingFlag()) return;
		BlasterCharacter->AddToInteractables(this);
	}
}

void AWeapon::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(OtherActor);
	if (BlasterCharacter) {
		if(WeaponType == EWeaponType::EWT_Flag)
		{
			if(BlasterCharacter->GetTeam() == Team) return;
		}
		if(BlasterCharacter->IsHoldingFlag()) return;
		BlasterCharacter->RemoveFromInteractables(this);
	}
}

void AWeapon::OnPingTooHigh(bool bHighPing)
{
	bUseServerSideRewind = !bHighPing;
}

void AWeapon::SetWeaponState(EWeaponState State)
{
	WeaponState = State;
	
	OnWeaponState();
	
}

void AWeapon::BecameOverlappingInteractable()
{
	IInteractables::BecameOverlappingInteractable();
	// Called on this weapon when it's sphere is overlapping with a blaster character and that blaster character is looking at this weapon
	ShowPickupWidget(true);
}

void AWeapon::StoppedBeingOverlappingInteractable()
{
	IInteractables::StoppedBeingOverlappingInteractable();
	ShowPickupWidget(false);
}

void AWeapon::InteractedWith(AActor* Interactor)
{
	IInteractables::InteractedWith(Interactor);
	ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(Interactor);
	if(BlasterCharacter)
	{
		BlasterCharacter->SetOverlappingWeapon(this);
	}
}

void AWeapon::EnableCustomDepth(bool bEnable)
{
	if(WeaponMesh)
	{
		WeaponMesh->SetRenderCustomDepth(bEnable);
	}
}

void AWeapon::OnRep_WeaponState()
{
	OnWeaponState();
}

void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	SphereRadius = FMath::FInterpTo(SphereRadius, MinimumSphereRaduis, DeltaTime, ResetAccurarySpeed);
	PickupMovement(DeltaTime);
	
}

void AWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(AWeapon, WeaponState);
	DOREPLIFETIME_CONDITION(AWeapon, bUseServerSideRewind, COND_OwnerOnly)
}

void AWeapon::Fire(const FVector& HitTarget)
{
	if (FireAnimation) {
		WeaponMesh->PlayAnimation(FireAnimation, false);
	}
	if(IncreaseScatterOverTime)
	{
		SphereRadius = FMath::Clamp(SphereRadius + FireSpreadIncrease, MinimumSphereRaduis, MaximumInnacuracy);
	}
	if (BulletCasing) {
		const USkeletalMeshSocket* AmmoEjectSocket = WeaponMesh->GetSocketByName(FName("AmmoEject"));
		
		if (AmmoEjectSocket) {
			FTransform SocketTransform = AmmoEjectSocket->GetSocketTransform(WeaponMesh);
				FActorSpawnParameters SpawnParams;
				UWorld* World = GetWorld();
				if (World) {
					World->SpawnActor<ABulletShell>(BulletCasing,
						SocketTransform.GetLocation(),
						SocketTransform.GetRotation().Rotator(),
						SpawnParams
						);
				}
		}
	}
	SpendRound();
}

void AWeapon::Dropped()
{
	SetWeaponState(EWeaponState::EWS_Dropped);
	FDetachmentTransformRules DetatchRules(EDetachmentRule::KeepWorld, true);
	WeaponMesh->DetachFromComponent(DetatchRules);
	SetOwner(nullptr);
	BlasterOwnerCharacter = nullptr;
	BlasterOwnerController = nullptr;
}

void AWeapon::SpendRound()
{
	CurrentAmmo = FMath::Clamp(CurrentAmmo - 1, 0, MaxAmmo);
	SetHUDAmmo();
	if(HasAuthority())
	{
		ClientUpdateAmmo(CurrentAmmo);
	}else
	{
		Sequence++;
	}
}

void AWeapon::ClientUpdateAmmo_Implementation(int32 ServerAmmo)
{
	if(HasAuthority()) return;
	CurrentAmmo = ServerAmmo;
	--Sequence;
	CurrentAmmo -= Sequence;
	SetHUDAmmo();
}

void AWeapon::ClientAddAmmo_Implementation(int32 AmmoToAdd)
{
	if(HasAuthority()) return;
	CurrentAmmo = FMath::Clamp(CurrentAmmo + AmmoToAdd, 0, MaxAmmo);
	BlasterOwnerCharacter = BlasterOwnerCharacter == nullptr ? Cast<ABlasterCharacter>(GetOwner()) : BlasterOwnerCharacter;
	if(BlasterOwnerCharacter && BlasterOwnerCharacter->GetCombatComponent() && IsFull())
	{
		BlasterOwnerCharacter->GetCombatComponent()->JumpToShotgunEnd();
	}
	SetHUDAmmo();
}

void AWeapon::IncreaseAmmo(int32 increase)
{
	CurrentAmmo = FMath::Clamp(CurrentAmmo + increase, 0, MaxAmmo);
	SetHUDAmmo();
	ClientAddAmmo(increase);
}

void AWeapon::SetHUDAmmo()
{
	BlasterOwnerCharacter = BlasterOwnerCharacter == nullptr ? Cast<ABlasterCharacter>(GetOwner()) : BlasterOwnerCharacter;
	if(!BlasterOwnerCharacter || !BlasterOwnerCharacter->GetCombatComponent() || BlasterOwnerCharacter->GetCombatComponent()->EquippedWeapon != this) return;
	if(BlasterOwnerCharacter)
	{
		BlasterOwnerController = BlasterOwnerController == nullptr ? Cast<ABlasterPlayerController>(BlasterOwnerCharacter->Controller) : BlasterOwnerController;
		if(BlasterOwnerCharacter && BlasterOwnerController)
		{
			BlasterOwnerController->SetHUDAmmo(CurrentAmmo);
		}
	}
}

bool AWeapon::IsEmpty()
{
	return CurrentAmmo <= 0;
}

bool AWeapon::IsFull()
{
	return CurrentAmmo == MaxAmmo;
}

FVector AWeapon::TraceEndWithScatter(const FVector& HitTarget)
{
	// Make a vector from the trace start to the hit target
	const USkeletalMeshSocket* MuzzlePoint = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if(MuzzlePoint == nullptr) return FVector();
	
	const FTransform SocketTransform = MuzzlePoint->GetSocketTransform(GetWeaponMesh()); // Start

	const FVector TraceStart = SocketTransform.GetLocation();
	

	const FVector ToTargetNormalized = (HitTarget - TraceStart).GetSafeNormal();

	const FVector SphereCenter = TraceStart + ToTargetNormalized * DistanceToSphere;

	const FVector RandVec = UKismetMathLibrary::RandomUnitVector() * FMath::FRandRange(0, SphereRadius);

	const FVector EndLoc = SphereCenter + RandVec;

	const FVector ToEndLoc = EndLoc - TraceStart;

	
	//DrawDebugSphere(GetWorld(), EndLoc, 4.f, 12, FColor::Orange, true);
	
	

	return FVector(TraceStart + ToEndLoc * TRACE_LENGTH / ToEndLoc.Size());

	//Make an invisible sphere 
}
void AWeapon::OnRep_Owner()
{
	Super::OnRep_Owner();

	SetHUDAmmo();
	if(Owner == nullptr)
	{
		BlasterOwnerCharacter = nullptr;
		BlasterOwnerController = nullptr;
	}else
	{
		SetHUDAmmo();
		
	}
}

void AWeapon::ResetInnacuracy()
{
	SphereRadius = MinimumSphereRaduis;
}

void AWeapon::PickupMovement(float DeltaTime)
{
	if(!bIsPickup) return;
	if(WeaponMesh)
	{
		if(WeaponMesh->GetComponentLocation().Z - InitialPosition.Z <= 0)
		{
			bGoUp = true;
		}
		
		WeaponMesh->AddLocalRotation(FRotator(0.f, BaseTurnRate * DeltaTime, 0.f));
		WeaponMesh->AddLocalOffset(FVector(0.f, 0.f, !bGoUp ? -1 * DeltaTime * BaseLevitateRate : DeltaTime * BaseLevitateRate));
		if(WeaponMesh->GetComponentLocation().Z - InitialPosition.Z > 40)
		{
			bGoUp = false;
		}
	}
}

void AWeapon::ShowPickupWidget(bool bShowWidget)
{
	if (PickupWidget) {
		PickupWidget->SetVisibility(bShowWidget);
	}
}

