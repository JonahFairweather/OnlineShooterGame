// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Projectile.h"

void AMyWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);
	
	APawn* InstigatorPawn = Cast<APawn>(GetOwner());
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName(FName("MuzzleFlash"));
	UWorld* World = GetWorld();
	
	if (MuzzleFlashSocket) {
		FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		//From muzzle flash socket to hit location from TraceUnderCrosshairs
		FVector ToTarget = HitTarget - SocketTransform.GetLocation();
		FRotator TargetRotation = ToTarget.Rotation();

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = GetOwner();
		SpawnParams.Instigator = InstigatorPawn;

		AProjectile* SpawnedProjectile = nullptr;

		if(bUseServerSideRewind)
		{
			if(InstigatorPawn->HasAuthority())
			{
				if(InstigatorPawn->IsLocallyControlled())
				{
					//Server host, replicated projectile
					SpawnedProjectile = World->SpawnActor<AProjectile>(ProjectileClass,
					SocketTransform.GetLocation(),
					TargetRotation,
					SpawnParams
					);
					SpawnedProjectile->bUseServerSideRewind = false;
					SpawnedProjectile->damage = GetDamage();
					SpawnedProjectile->HeadshotDamage = HeadshotDamage;
				}else
				{
					//Server proxy, do not spawn replicated projectile, no ssr
					SpawnedProjectile = World->SpawnActor<AProjectile>(ServerSideRewindProjectileClass,
					SocketTransform.GetLocation(),
					TargetRotation,
					SpawnParams
					);
					SpawnedProjectile->bUseServerSideRewind = false;
				}
			}else
			{
				if(InstigatorPawn->IsLocallyControlled())
				{
					// Client fire, spawn non-replicated projectile with SSR
					SpawnedProjectile = World->SpawnActor<AProjectile>(ServerSideRewindProjectileClass,
					SocketTransform.GetLocation(),
					TargetRotation,
					SpawnParams
					);
					SpawnedProjectile->bUseServerSideRewind = true;
					SpawnedProjectile->InitialLocation = SocketTransform.GetLocation();
					SpawnedProjectile->InitialVelocity = SpawnedProjectile->GetActorForwardVector() * SpawnedProjectile->InitialSpeed;
					SpawnedProjectile->damage = GetDamage();
					SpawnedProjectile->HeadshotDamage = HeadshotDamage;
				}else
				{
					//Simulated proxy
					SpawnedProjectile = World->SpawnActor<AProjectile>(ServerSideRewindProjectileClass,
					SocketTransform.GetLocation(),
					TargetRotation,
					SpawnParams
					);
					SpawnedProjectile->bUseServerSideRewind = false;
				}
			}
		}else
		{
			if(InstigatorPawn->HasAuthority())
			{
				//Server host or server authoritative version of client, replicated projectile
				SpawnedProjectile = World->SpawnActor<AProjectile>(ProjectileClass,
				SocketTransform.GetLocation(),
				TargetRotation,
				SpawnParams
				);
				SpawnedProjectile->bUseServerSideRewind = false;
				SpawnedProjectile->damage = GetDamage();
				SpawnedProjectile->HeadshotDamage = HeadshotDamage;
			}
		}
		
		
	}
}
