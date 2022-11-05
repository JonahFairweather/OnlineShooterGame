// Fill out your copyright notice in the Description page of Project Settings.


#include "HitscanWeapon.h"

#include "Blaster/BlasterCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"
#include "WeaponTypes.h"

void AHitscanWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);
	//Now we need to perform a line trace

	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if(OwnerPawn == nullptr) return;

	AController* InstigatorController = OwnerPawn->GetController();

	const USkeletalMeshSocket* MuzzlePoint = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if(MuzzlePoint)
	{
		FTransform SocketTransform = MuzzlePoint->GetSocketTransform(GetWeaponMesh()); // Start

		FVector Start = SocketTransform.GetLocation();
		
		
		FHitResult FireHit;
		WeaponTraceHit(Start, HitTarget, FireHit);
		
		ACharacter* BlasterCharacter = Cast<ACharacter>(FireHit.GetActor());
		if(BlasterCharacter && InstigatorController) // If we have a valid controller, we are on the server and the hit was a blaster character
			{
			float DamageDealt = FireHit.BoneName.ToString() == FString("head") ? HeadshotDamage : damage;
				bool bCauseAuthDamage = !bUseServerSideRewind || OwnerPawn->IsLocallyControlled();
				if(HasAuthority()){
					BlasterOwnerCharacter = BlasterOwnerCharacter == nullptr ? Cast<ABlasterCharacter>(OwnerPawn) : BlasterOwnerCharacter;
					if(BlasterOwnerCharacter->IsLocallyControlled())
					{
						
						
						UGameplayStatics::ApplyDamage(BlasterCharacter,
								  DamageDealt,
								  InstigatorController,
								  this,
								  UDamageType::StaticClass()
								  );
					}
					if(!BlasterOwnerCharacter->IsLocallyControlled() && !bUseServerSideRewind)
					{
						UGameplayStatics::ApplyDamage(BlasterCharacter,
								  DamageDealt,
								  InstigatorController,
								  this,
								  UDamageType::StaticClass()
								  );
					}
				  
				}else if(!HasAuthority() && bUseServerSideRewind)
				{
					BlasterOwnerCharacter = BlasterOwnerCharacter == nullptr ? Cast<ABlasterCharacter>(OwnerPawn) : BlasterOwnerCharacter;
					BlasterOwnerController = BlasterOwnerController == nullptr ? Cast<ABlasterPlayerController>(InstigatorController) : BlasterOwnerController;
					ABlasterCharacter* BlasterCharacter1 = Cast<ABlasterCharacter>(BlasterCharacter);
					if(BlasterOwnerCharacter->IsLocallyControlled() && BlasterOwnerCharacter->GetLagCompensation() && BlasterOwnerController && BlasterCharacter1)
					{
						BlasterOwnerCharacter->GetLagCompensation()->ServerScoreRequest(BlasterCharacter1,
							Start,
							HitTarget,
							BlasterOwnerController->GetServerTime() - BlasterOwnerController->SingleTripTime,
							this);
					}
				}
			
					
			}
		if(ImpactParticles)
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, FireHit.ImpactPoint,
				FireHit.ImpactNormal.Rotation());
		}
		if(HitSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, HitSound, FireHit.ImpactPoint);
		}
		
		if(MuzzleFlash)
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), MuzzleFlash,
				SocketTransform);
		}
		if(FireSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
		}
	}
}



void AHitscanWeapon::WeaponTraceHit(const FVector& TraceStart, const FVector& HitTarget, FHitResult& OutHit)
{
	
	UWorld* World = GetWorld();
	if(World)
	{
		FVector End =  TraceStart + (HitTarget - TraceStart) * 1.25f;
		
		World->LineTraceSingleByChannel(
				OutHit,
				TraceStart,
				End,
				ECollisionChannel::ECC_Visibility);
		FVector BeamEnd = End;
		
		if(OutHit.bBlockingHit)
		{
			BeamEnd = OutHit.ImpactPoint;
			
		}else
		{
			OutHit.ImpactPoint = End;
		}
		if(BeamParticles)
		{
			UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(
				World,
				BeamParticles,
				TraceStart,
				FRotator::ZeroRotator,
				true
				);
			if(Beam)
			{
				Beam->SetVectorParameter(FName("target"), OutHit.ImpactPoint);
				Beam->SetBeamSourcePoint(0, TraceStart, 0);
				Beam->SetBeamEndPoint(0, BeamEnd);
			}
		}
	}
}
