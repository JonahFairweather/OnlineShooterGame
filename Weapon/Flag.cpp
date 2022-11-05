// Fill out your copyright notice in the Description page of Project Settings.


#include "Flag.h"

#include "WeaponTypes.h"
#include "Blaster/Blaster.h"
#include "Blaster/BlasterCharacter.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"

AFlag::AFlag()
{
	FlagMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Flag Mesh"));
	SetRootComponent(FlagMesh);
	
	GetAreaSphere()->SetupAttachment(FlagMesh);
	GetWeaponMesh()->SetupAttachment(FlagMesh);

	PickupWidget->SetupAttachment(FlagMesh);

	
	GetInteractableSphere()->SetupAttachment(FlagMesh);
	GetInteractableSphere()->SetupAttachment(RootComponent);
}

void AFlag::ResetLocation()
{
	
	BlasterOwnerCharacter = Cast<ABlasterCharacter>(GetOwner());
	if(BlasterOwnerCharacter)
	{
		BlasterOwnerCharacter->SetHoldingFlag(false);
		BlasterOwnerCharacter->SetOverlappingWeapon(nullptr);
		BlasterOwnerCharacter->UnCrouch();
		
	}
	if(BlasterOwnerController)
	{
		BlasterOwnerController->StopFlagTipText();
	}
	if(HasAuthority())
	{
		SetActorTransform(InitialTransform);
		FDetachmentTransformRules DetachRules(EDetachmentRule::KeepWorld, true);
		FlagMesh->DetachFromComponent(DetachRules);
		SetOwner(nullptr);
		GetAreaSphere()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		GetAreaSphere()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
		InteractableSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		InteractableSphere->SetCollisionResponseToChannel(ECC_Interactable, ECollisionResponse::ECR_Block);
		InteractableSphere->SetWorldLocation(GetActorLocation());
		BlasterOwnerCharacter = nullptr;
		BlasterOwnerController = nullptr;
		SetWeaponState(EWeaponState::EWS_Initial);
	}
	
	
}

void AFlag::Dropped()
{
	SetWeaponState(EWeaponState::EWS_Dropped);
	FDetachmentTransformRules DetatchRules(EDetachmentRule::KeepWorld, true);
	FlagMesh->DetachFromComponent(DetatchRules);
	BlasterOwnerCharacter = Cast<ABlasterCharacter>(GetOwner());
	if(BlasterOwnerCharacter)
	{
		BlasterOwnerCharacter->SetHoldingFlag(false);
	}
	if(BlasterOwnerController)
	{
		BlasterOwnerController->StopFlagTipText();
	}
	SetOwner(nullptr);
	BlasterOwnerCharacter = nullptr;
	BlasterOwnerController = nullptr;
}

void AFlag::BeginPlay()
{
	Super::BeginPlay();
	InitialTransform = GetActorTransform();
	InteractableSphere->SetupAttachment(FlagMesh);
	InteractableSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	InteractableSphere->SetCollisionResponseToChannel(ECC_Interactable, ECollisionResponse::ECR_Block);
}

void AFlag::HandleWeaponEquipped()
{
	ShowPickupWidget(false);
	GetAreaSphere()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	FlagMesh->SetSimulatePhysics(false);
	FlagMesh->SetEnableGravity(false);
	FlagMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	FlagMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Overlap);
	InteractableSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	InteractableSphere->SetCollisionResponseToChannel(ECC_Interactable, ECR_Ignore);
	
	
	EnableCustomDepth(false);
}

void AFlag::HandleWeaponDropped()
{
	
	GetAreaSphere()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	
	InteractableSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	InteractableSphere->SetCollisionResponseToChannel(ECC_Interactable, ECollisionResponse::ECR_Block);
	FlagMesh->SetSimulatePhysics(true);
	FlagMesh->SetEnableGravity(true);
	FlagMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	FlagMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_BLUE);
	EnableCustomDepth(true);
	
}
