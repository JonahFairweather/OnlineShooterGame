// Fill out your copyright notice in the Description page of Project Settings.


#include "Shotgun.h"


#include "Blaster/BlasterCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Sound/SoundCue.h"

void AShotgun::FireShotgun(const TArray<FVector_NetQuantize>& HitTargets)
{
	AWeapon::Fire(FVector());
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if(OwnerPawn == nullptr) return;
	AController* InstigatorController = OwnerPawn->GetController();

	const USkeletalMeshSocket* MuzzlePoint = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if(MuzzlePoint)
	{
		const FTransform SocketTransform = MuzzlePoint->GetSocketTransform(GetWeaponMesh()); // Start

		const FVector Start = SocketTransform.GetLocation();

		TMap<ABlasterCharacter*, int32> HitMap; // Maps hit character to damage taken
		for(FVector_NetQuantize HitTarget : HitTargets)
		{
			FHitResult FireHit;
			WeaponTraceHit(Start, HitTarget, FireHit);
			ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(FireHit.GetActor());
			if(BlasterCharacter) // If we have a valid controller, we are on the server and the hit was a blaster character
				{
					float BaseDamage = FireHit.BoneName.ToString() == FString("head") ? HeadshotDamage : damage;
					float DistanceBetween = (FireHit.ImpactPoint - Start).Length();
					float p = -(0.75f/MinDamageEndpoint) * (DistanceBetween - MaxDamageRange) + 1;
					float DamagePercent = FMath::Clamp(p, 0.25f, 1.f);
					int32 DamageDealt = ApplyDamageFalloff ? FMath::RoundToInt32(BaseDamage * DamagePercent) : damage;
				
					if(HitMap.Contains(BlasterCharacter))
					{
						HitMap[BlasterCharacter] += DamageDealt;
					}
					else
					{
					HitMap.Emplace(BlasterCharacter, DamageDealt);
					}
				}
			if(ImpactParticles)
			{
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, FireHit.ImpactPoint,
					FireHit.ImpactNormal.Rotation());
			}
			if(HitSound)
			{
				UGameplayStatics::PlaySoundAtLocation(this, HitSound, FireHit.ImpactPoint, .5f, FMath::RandRange(-.5f, .5f));
			}
		}
		TArray<ABlasterCharacter*> HitCharacters;
		bool bCauseAuthDamage = !bUseServerSideRewind || OwnerPawn->IsLocallyControlled();
		for(auto HitPair : HitMap)
		{
			
			HitCharacters.AddUnique(HitPair.Key);
			UE_LOG(LogTemp, Warning, TEXT("A character has been hit!"));
			if(HitPair.Key && HasAuthority() && InstigatorController && InstigatorController->IsLocalController())
			{
				
				
				UE_LOG(LogTemp, Warning, TEXT("Applying Damage Through Server Authority Shot"));
				UGameplayStatics::ApplyDamage(HitPair.Key,
					HitPair.Value,
					InstigatorController,
					this, UDamageType::StaticClass());
				
			}
			if(HitPair.Key && HasAuthority() && InstigatorController && !InstigatorController->IsLocalController() && !bUseServerSideRewind)
			{
					UE_LOG(LogTemp, Warning, TEXT("Applying Damage Through Server Authority Shot"));
					UGameplayStatics::ApplyDamage(HitPair.Key,
					HitPair.Value,
					InstigatorController,
					this, UDamageType::StaticClass());
			}
		} // Apply damage on the server
		if(!HasAuthority() && bUseServerSideRewind)
		{
			UE_LOG(LogTemp, Warning, TEXT("%d characters hit on this blast"), HitCharacters.Num());
			BlasterOwnerCharacter = BlasterOwnerCharacter == nullptr ? Cast<ABlasterCharacter>(OwnerPawn) : BlasterOwnerCharacter;
			BlasterOwnerController = BlasterOwnerController == nullptr ? Cast<ABlasterPlayerController>(InstigatorController) : BlasterOwnerController;
			if(BlasterOwnerCharacter->IsLocallyControlled() && BlasterOwnerCharacter->GetLagCompensation() && BlasterOwnerController && BlasterOwnerCharacter->IsLocallyControlled())
			{
				BlasterOwnerCharacter->GetLagCompensation()->ServerShotgunScoreRequest(HitCharacters, Start,
					HitTargets,BlasterOwnerController->GetServerTime() - BlasterOwnerController->SingleTripTime,
					this);
			}
		}
	}
}

void AShotgun::ShotgunTraceEndWithScatter(const FVector& HitTarget, TArray<FVector_NetQuantize>& HitTargets)
{
	const USkeletalMeshSocket* MuzzlePoint = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if(MuzzlePoint == nullptr) return;
	
	const FTransform SocketTransform = MuzzlePoint->GetSocketTransform(GetWeaponMesh()); // Start

	const FVector TraceStart = SocketTransform.GetLocation();

	const FVector ToTargetNormalized = (HitTarget - TraceStart).GetSafeNormal();

	const FVector SphereCenter = TraceStart + ToTargetNormalized * DistanceToSphere;
	
	for(uint32 i = 0; i < NumOfPellets; i++)
	{
		FVector RandVec = UKismetMathLibrary::RandomUnitVector() * FMath::FRandRange(0, SphereRadius);

		FVector EndLoc = SphereCenter + RandVec;

		FVector ToEndLoc = EndLoc - TraceStart;
		ToEndLoc = TraceStart + ToEndLoc * TRACE_LENGTH / ToEndLoc.Size();

		HitTargets.Add(ToEndLoc);
	}
}
