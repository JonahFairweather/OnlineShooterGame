// Fill out your copyright notice in the Description page of Project Settings.


#include "LagCompensationComponent.h"

#include "Blaster/Blaster.h"
#include "Blaster/BlasterCharacter.h"
#include "Blaster/Weapon/Shotgun.h"
#include "Components/BoxComponent.h"
#include "Containers/IntrusiveDoubleLinkedList.h"
#include "Kismet/GameplayStatics.h"


ULagCompensationComponent::ULagCompensationComponent()
{
	
	PrimaryComponentTick.bCanEverTick = true;

	
}

void ULagCompensationComponent::ShowFramePackage(const FFramePackage& Package, const FColor& Color)
{
	for(auto BoxInformation : Package.HitBoxInfo)
	{
		DrawDebugBox(GetWorld(),
			BoxInformation.Value.Location,
			BoxInformation.Value.BoxExtent,FQuat(BoxInformation.Value.BoxRotation),
			Color,
			false, 1.f);
	}
}

FServerSideRewindResult ULagCompensationComponent::ServerSideRewind(ABlasterCharacter* HitCharacter, const FVector_NetQuantize& TraceStart,
	const FVector_NetQuantize& HitLocation, float HitTime)
{
	FFramePackage FrameToCheck = GetFrameToCheck(HitCharacter, HitTime);

	return ConfirmHit(FrameToCheck, HitCharacter, TraceStart, HitLocation);
}

FFramePackage ULagCompensationComponent::GetFrameToCheck(ABlasterCharacter* HitCharacter, float HitTime)
{
	bool bReturnEarly = HitCharacter == nullptr
	|| HitCharacter->GetLagCompensation() == nullptr
	|| HitCharacter->GetLagCompensation()->FrameHitBoxHistory.GetTail() == nullptr
	|| HitCharacter->GetLagCompensation()->FrameHitBoxHistory.GetHead() == nullptr;

	if(bReturnEarly) return FFramePackage(); //Something is not right
	bool bShouldInterpolate = true;
	FFramePackage FrameToCheck;
	
	const TDoubleLinkedList<FFramePackage>& OtherCharacterHistory = HitCharacter->GetLagCompensation()->FrameHitBoxHistory;
	//Frame History of the hit character

	const float OldestHistoryTime = OtherCharacterHistory.GetTail()->GetValue().Time;
	if(OldestHistoryTime > HitTime)
	{
		return FFramePackage(); //A Request was made too far in the past, information is no longer stored 
	}
	if(OldestHistoryTime == HitTime)
	{
		bShouldInterpolate = false;
		FrameToCheck = OtherCharacterHistory.GetTail()->GetValue();
	}
	const float NewestTime = OtherCharacterHistory.GetHead()->GetValue().Time;
	if(HitTime >= NewestTime)
	{
		bShouldInterpolate = false;
		FrameToCheck = OtherCharacterHistory.GetHead()->GetValue();
	}
	TDoubleLinkedList<FFramePackage>::TDoubleLinkedListNode* Younger = OtherCharacterHistory.GetHead();
	TDoubleLinkedList<FFramePackage>::TDoubleLinkedListNode* Older = OtherCharacterHistory.GetHead();

	while(Older->GetValue().Time > HitTime) // While the older pointer is still greater than the younger pointer
		{
		Older = Older->GetNextNode();
		if(Older->GetValue().Time > HitTime)
		{
			Younger->GetNextNode();
		}
		//Both pointers should point at the same node UNLESS we have found the two frames which HitTime is between
		}
	if(Older->GetValue().Time == HitTime)
	{
		bShouldInterpolate = false;
		FrameToCheck = Older->GetValue();
	}

	if(bShouldInterpolate)
	{
		//We need to interpolate between the younger and older frame data
		FrameToCheck = InterpBetweenFrames(Older->GetValue(), Younger->GetValue(), HitTime);
	}
	FrameToCheck.Character = HitCharacter;
	return FrameToCheck;
}

FServerSideShotgunResult ULagCompensationComponent::ShotgunServerSideRewind(
	const TArray<ABlasterCharacter*>& HitCharacters, const FVector_NetQuantize& TraceStart,
	const TArray<FVector_NetQuantize>& TraceEnd, float HitTime, AShotgun* DamageCauser)
{
	TArray<FFramePackage> FramesToCheck;
	for(ABlasterCharacter* HitCharacter : HitCharacters)
	{
		FFramePackage FrameToCheck = GetFrameToCheck(HitCharacter, HitTime);
		
		FramesToCheck.Add(FrameToCheck);
		
	}
	UE_LOG(LogTemp, Warning, TEXT("ITS REWIND TIME"))
	UE_LOG(LogTemp, Warning, TEXT("Checking %d frames"), FramesToCheck.Num());
	UE_LOG(LogTemp, Warning, TEXT("Checking %d Characters"), HitCharacters.Num());

	return ShotgunConfirmHit(FramesToCheck, TraceStart, TraceEnd, DamageCauser);
	//Get where each character was at this given time and store it in the array of frames to check

	//Now we need to confirm the hits on each character
}



void ULagCompensationComponent::ServerScoreRequest_Implementation(ABlasterCharacter* HitCharacter,
                                                                  const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& TraceEnd, float HitTime, AWeapon* DamageCauser)
{
	FServerSideRewindResult RewindResult = ServerSideRewind(HitCharacter, TraceStart, TraceEnd, HitTime);
	if(Character && HitCharacter && DamageCauser && RewindResult.bHitConfirmed)
	{
		if(RewindResult.bHeadShot)
		{
			UGameplayStatics::ApplyDamage(HitCharacter, DamageCauser->GetHeadshotDamage(),
			Character->Controller, DamageCauser, UDamageType::StaticClass());
		}else
		{
			UGameplayStatics::ApplyDamage(HitCharacter, DamageCauser->GetDamage(),
			Character->Controller, DamageCauser, UDamageType::StaticClass());
		}
		
	}
}


void ULagCompensationComponent::ServerShotgunScoreRequest_Implementation(const TArray<ABlasterCharacter*>& HitCharacters,
	const FVector_NetQuantize& TraceStart, const TArray<FVector_NetQuantize>& TraceEnds, float HitTime,
	AShotgun* DamageCauser)
{
	FServerSideShotgunResult ShotgunFireResult = ShotgunServerSideRewind(HitCharacters, TraceStart, TraceEnds, HitTime, DamageCauser);
	for(ABlasterCharacter* BlasterCharacter : HitCharacters)
	{
		if(Character && BlasterCharacter && DamageCauser)
		{
			float Damage = 0.f;
			if(ShotgunFireResult.Damage.Contains(BlasterCharacter))
			{
				Damage = ShotgunFireResult.Damage[BlasterCharacter];
			}
			UE_LOG(LogTemp, Warning, TEXT("Applying %f Damage Through Server Shotgun Request"), Damage);
			UGameplayStatics::ApplyDamage(BlasterCharacter, Damage,
				Character->Controller, DamageCauser, UDamageType::StaticClass());
		}
	}
}

FServerSideRewindResult ULagCompensationComponent::ProjectileServerSideRewind(ABlasterCharacter* HitCharacter,
	const FVector_NetQuantize& TraceStart, const FVector_NetQuantize100& InitialVelocity, float HitTime)
{
	FFramePackage FrameToCheck = GetFrameToCheck(HitCharacter, HitTime);
	return ProjectileConfirmHit(FrameToCheck, HitCharacter, TraceStart, InitialVelocity, HitTime);
}

FServerSideRewindResult ULagCompensationComponent::ProjectileConfirmHit(FFramePackage FrameToCheck, ABlasterCharacter* HitCharacter, 
		const FVector_NetQuantize& TraceStart, 
		const FVector_NetQuantize100& InitialVelocity,
		float HitTime)
{
	if (HitCharacter == nullptr) return FServerSideRewindResult();
	FServerSideRewindResult result;
	FFramePackage CurrentFrame; //The player's current frame package
	CacheBoxPositions(HitCharacter, CurrentFrame);
	MoveBoxes(HitCharacter, FrameToCheck);
	EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::NoCollision);
	// Enable Collision for the head first
	UBoxComponent* HeadBox = HitCharacter->HitCollisionBoxes[FName("head")];
	HeadBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	HeadBox->SetCollisionResponseToChannel(ECC_HitBox, ECollisionResponse::ECR_Block);
	UWorld* World = GetWorld();
	FPredictProjectilePathParams PathParams;
	PathParams.bTraceWithCollision = true;
	PathParams.MaxSimTime = MaxRecordTime;
	PathParams.LaunchVelocity = InitialVelocity;
	PathParams.StartLocation = TraceStart;
	PathParams.SimFrequency = 15.f;
	PathParams.ProjectileRadius = 5.f;
	PathParams.TraceChannel = ECC_HitBox;
	PathParams.ActorsToIgnore.Add(GetOwner());
	

	FPredictProjectilePathResult PathResult;

	UGameplayStatics::PredictProjectilePath(this, PathParams, PathResult);
	if(PathResult.HitResult.bBlockingHit)
	{
		//We hit the head
		ResetHitBoxes(HitCharacter, CurrentFrame);
		EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);
		result.bHeadShot = true;
		result.bHitConfirmed = true;
		return result;
	}
	else
	{
		//No headshot, check the rest of the boxes
		for(auto& HitBoxPair : HitCharacter->HitCollisionBoxes)
		{
			if(HitBoxPair.Value != nullptr)
			{
				HitBoxPair.Value->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
				HitBoxPair.Value->SetCollisionResponseToChannel(ECC_HitBox, ECollisionResponse::ECR_Block);
			}
		}
		UGameplayStatics::PredictProjectilePath(this, PathParams, PathResult);
		if(PathResult.HitResult.bBlockingHit)
		{
			//We hit the head
			
			ResetHitBoxes(HitCharacter, CurrentFrame);
			EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);
			result.bHeadShot = false;
			result.bHitConfirmed = true;
			return result;
		}
	}
	
	ResetHitBoxes(HitCharacter, CurrentFrame);
	EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);
	return FServerSideRewindResult{ false, false };
}

void ULagCompensationComponent::ProjectileServerScoreRequest_Implementation(ABlasterCharacter* HitCharacter,
	const FVector_NetQuantize& TraceStart, const FVector_NetQuantize100& InitialVelocity, float HitTime)
{
	FServerSideRewindResult Confirm = ProjectileServerSideRewind(HitCharacter, TraceStart, InitialVelocity, HitTime);
	if(Character && HitCharacter && Confirm.bHitConfirmed && Character->GetEquippedWeapon())
	{
		float DamageDealt = Confirm.bHeadShot ? Character->GetEquippedWeapon()->GetHeadshotDamage() : Character->GetEquippedWeapon()->GetDamage();
		UGameplayStatics::ApplyDamage(HitCharacter, DamageDealt,
			Character->Controller, Character->GetEquippedWeapon(), UDamageType::StaticClass());
	}
}

void ULagCompensationComponent::BeginPlay()
{
	Super::BeginPlay();

	FFramePackage Package;
	SaveFramePackage(Package);
	
	
}
void ULagCompensationComponent::SaveFramePackage(FFramePackage& Package)
{
	Character = Character == nullptr ? Cast<ABlasterCharacter>(GetOwner()) : Character;
	if(Character)
	{
		Package.Time = GetWorld()->GetTimeSeconds();
		Package.Character = Character;
		for(auto& BoxPair : Character->HitCollisionBoxes)
		{
			FBoxInformation BoxInformation;
			BoxInformation.Location = BoxPair.Value->GetComponentLocation();
			BoxInformation.BoxRotation = BoxPair.Value->GetComponentRotation();
			BoxInformation.BoxExtent = BoxPair.Value->GetScaledBoxExtent();
			Package.HitBoxInfo.Add(BoxPair.Key, BoxInformation);
		}
	}
}

void ULagCompensationComponent::SaveFrameData()
{
	if(Character == nullptr || !Character->HasAuthority()) return;
	FFramePackage ThisFrame;
	if(FrameHitBoxHistory.Num() <= 1)
	{
		SaveFramePackage(ThisFrame);
		FrameHitBoxHistory.AddHead(ThisFrame);
	}else
	{
		float HistoryLength = FrameHitBoxHistory.GetHead()->GetValue().Time - FrameHitBoxHistory.GetTail()->GetValue().Time;
		while(HistoryLength > MaxRecordTime)
		{
			FrameHitBoxHistory.RemoveNode(FrameHitBoxHistory.GetTail());
			HistoryLength = FrameHitBoxHistory.GetHead()->GetValue().Time - FrameHitBoxHistory.GetTail()->GetValue().Time;
		}
		SaveFramePackage(ThisFrame);
		FrameHitBoxHistory.AddHead(ThisFrame);
	}
}

FFramePackage ULagCompensationComponent::InterpBetweenFrames(const FFramePackage& Older, const FFramePackage& Newer,
	float HitTime)
{
	const float Distance = Newer.Time - Older.Time;
	const float InterpFraction = FMath::Clamp((HitTime - Older.Time) / Distance, 0.f, 1.f);
	//The amount we are to the newer frame
	FFramePackage InterpolatedFramePackage;
	InterpolatedFramePackage.Time = HitTime;
	for(auto& HitPair : Newer.HitBoxInfo)
	{
		const FName& BoxName = HitPair.Key;
		//The name of the box we are interpolating for

		const FBoxInformation& OlderBox = Older.HitBoxInfo[BoxName];
		const FBoxInformation& YoungerBox = Newer.HitBoxInfo[BoxName];

		FBoxInformation InterpolatedBoxInformation;
		InterpolatedBoxInformation.Location = FMath::VInterpTo(OlderBox.Location, YoungerBox.Location, 1.f, InterpFraction);
		InterpolatedBoxInformation.BoxRotation = FMath::RInterpTo(OlderBox.BoxRotation, YoungerBox.BoxRotation, 1.f, InterpFraction);
		InterpolatedBoxInformation.BoxExtent = OlderBox.BoxExtent;
		InterpolatedFramePackage.HitBoxInfo.Add(BoxName, InterpolatedBoxInformation);
	}

	return InterpolatedFramePackage;
}

FServerSideRewindResult ULagCompensationComponent::ConfirmHit(const FFramePackage& Package,
	ABlasterCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation)
{
	if (HitCharacter == nullptr) return FServerSideRewindResult();
	FServerSideRewindResult result;
	FFramePackage CurrentFrame; //The player's current frame package
	CacheBoxPositions(HitCharacter, CurrentFrame);
	MoveBoxes(HitCharacter, Package);
	EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::NoCollision);
	// Enable Collision for the head first
	UBoxComponent* HeadBox = HitCharacter->HitCollisionBoxes[FName("head")];
	HeadBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	HeadBox->SetCollisionResponseToChannel(ECC_HitBox, ECollisionResponse::ECR_Block);
	FHitResult ConfirmHitResult;
	const FVector TraceEnd = TraceStart + (HitLocation - TraceStart) * 1.25f;
	UWorld* World = GetWorld();
	if(World)
	{
		World->LineTraceSingleByChannel(ConfirmHitResult, TraceStart, TraceEnd, ECC_HitBox);
		if(ConfirmHitResult.bBlockingHit) // We hit the head, return early
		{
			ResetHitBoxes(HitCharacter, CurrentFrame);
			EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);
			result.bHeadShot = true;
			result.bHitConfirmed = true;
			return result;
		}else{
		//This algorithm is good for tracing against the headbox{
			// No headshot, check the rest of the boxes;
			for(auto& HitBoxPair : HitCharacter->HitCollisionBoxes)
			{
				if(HitBoxPair.Value != nullptr)
				{
					HitBoxPair.Value->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
					HitBoxPair.Value->SetCollisionResponseToChannel(ECC_HitBox, ECollisionResponse::ECR_Block);
				}
			}
			World->LineTraceSingleByChannel(ConfirmHitResult, TraceStart, TraceEnd, ECC_HitBox);
			if(ConfirmHitResult.bBlockingHit) // We hit the head, return early
				{
				ResetHitBoxes(HitCharacter, CurrentFrame);
				EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);
				result.bHeadShot = false;
				result.bHitConfirmed = true;
				return result;
				}
			return FServerSideRewindResult{ false, false };
				
		}
	}
	return FServerSideRewindResult{ false, false };
}

FServerSideShotgunResult ULagCompensationComponent::ShotgunConfirmHit(const TArray<FFramePackage>& FramesToCheck,
	const FVector_NetQuantize& TraceStart, const TArray<FVector_NetQuantize>& TraceEnd, AShotgun* DamageCauser)
{
	float BaseDamage = DamageCauser->GetDamage();
	float HeadshotDamage = DamageCauser->GetHeadshotDamage();
	//For each frame we are checking we need to see if on the server at this time
	FServerSideShotgunResult ShotgunResult;
	TArray<FFramePackage> CurrentFrames;
	
	for(auto Frame : FramesToCheck)
	{
		FFramePackage CurrentFrame;
		CurrentFrame.Character = Frame.Character;
		ABlasterCharacter* HitCharacter = Frame.Character;
		if(HitCharacter == nullptr)
		{
			return FServerSideShotgunResult();
		}
		CacheBoxPositions(HitCharacter, CurrentFrame);
		MoveBoxes(HitCharacter, Frame);
		EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::NoCollision);
		//Move the frame's character to the location, disable the mesh and save the current location.
		
		CurrentFrames.Add(CurrentFrame);
	}
	for(auto Frame : FramesToCheck)
	{
		ABlasterCharacter* HitCharacter = Frame.Character;
		UBoxComponent* HeadBox = HitCharacter->HitCollisionBoxes[FName("head")];
		HeadBox->SetCollisionResponseToChannel(ECC_HitBox, ECollisionResponse::ECR_Block);
		HeadBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	}

	UWorld* World = GetWorld();
	TArray<FVector_NetQuantize> NoHeadshot;
	for(auto& HitSpot : TraceEnd)
	{
		FHitResult ConfirmHitResult;
		const FVector ToTraceEnd = TraceStart + (HitSpot - TraceStart) * 1.25f;
		if(World)
		{
			World->LineTraceSingleByChannel(ConfirmHitResult, TraceStart, ToTraceEnd, ECC_HitBox);
			ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(ConfirmHitResult.GetActor());
			if(BlasterCharacter)
			{
				float Damage;
				float DistanceBetween = (ConfirmHitResult.ImpactPoint - TraceStart).Length();
				float p = -(0.75f/2000.f) * (DistanceBetween - 600.f) + 1;
				float DamagePercent = FMath::Clamp(p, 0.25f, 1.f);
				Damage = FMath::RoundToInt32(HeadshotDamage * DamagePercent);
			    if(ShotgunResult.Damage.Contains(BlasterCharacter))
			    {
				    ShotgunResult.Damage[BlasterCharacter] += Damage;
			    	
			    }else
			    {
				    ShotgunResult.Damage.Emplace(BlasterCharacter, Damage);
			    	
			    }
			}else
			{
				NoHeadshot.AddUnique(HitSpot);
			}
		}
	}
	//Check for Headshots

	for(auto Frame : FramesToCheck)
	{
		for(auto& HitBoxPair : Frame.Character->HitCollisionBoxes)
		{
			if(HitBoxPair.Value != nullptr)
			{
				HitBoxPair.Value->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
				HitBoxPair.Value->SetCollisionResponseToChannel(ECC_HitBox, ECollisionResponse::ECR_Block);
			}
		}
		UBoxComponent* HeadBox = Frame.Character->HitCollisionBoxes[FName("head")];
		HeadBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	for(auto& HitSpot : NoHeadshot)
	{
		FHitResult ConfirmHitResult;
		const FVector ToTraceEnd = TraceStart + (HitSpot - TraceStart) * 1.25f;
		if(World)
		{
			World->LineTraceSingleByChannel(ConfirmHitResult, TraceStart, ToTraceEnd, ECC_HitBox);
			ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(ConfirmHitResult.GetActor());
			if(BlasterCharacter)
			{
				
				float Damage;
				float DistanceBetween = (ConfirmHitResult.ImpactPoint - TraceStart).Length();
				float p = -(0.75f/2000.f) * (DistanceBetween - 600.f) + 1;
				float DamagePercent = FMath::Clamp(p, 0.25f, 1.f);
				Damage = FMath::RoundToInt32(BaseDamage * DamagePercent);
				if(ShotgunResult.Damage.Contains(BlasterCharacter))
				{
					ShotgunResult.Damage[BlasterCharacter] += Damage;
					UE_LOG(LogTemp, Warning, TEXT("Bodyshot! Adding %f Damage"), Damage);
				}else
				{
					ShotgunResult.Damage.Emplace(BlasterCharacter, Damage);
					UE_LOG(LogTemp, Warning, TEXT("Bodyshot! Adding %f Damage"), Damage);
				}
			}
		}
	}
	//Check bodyshots

	for(auto Frame : CurrentFrames)
	{
		ResetHitBoxes(Frame.Character, Frame);
		EnableCharacterMeshCollision(Frame.Character, ECollisionEnabled::QueryAndPhysics);
	}

	return ShotgunResult;

}

void ULagCompensationComponent::CacheBoxPositions(ABlasterCharacter* HitCharacter, FFramePackage& OutFramePackage)
{
	if (HitCharacter == nullptr) return;
	for (auto& HitBoxPair : HitCharacter->HitCollisionBoxes)
	{
		if (HitBoxPair.Value != nullptr)
		{
			FBoxInformation BoxInfo;
			BoxInfo.Location = HitBoxPair.Value->GetComponentLocation();
			BoxInfo.BoxRotation = HitBoxPair.Value->GetComponentRotation();
			BoxInfo.BoxExtent = HitBoxPair.Value->GetScaledBoxExtent();
			OutFramePackage.HitBoxInfo.Add(HitBoxPair.Key, BoxInfo);
		}
	}
}

void ULagCompensationComponent::MoveBoxes(ABlasterCharacter* HitCharacter, const FFramePackage& InFramePackage)
{
	if (HitCharacter == nullptr) return;
	for (auto& HitBoxPair :  HitCharacter->HitCollisionBoxes)
	{
		if (HitBoxPair.Value != nullptr)
		{
			HitBoxPair.Value->SetWorldLocation(InFramePackage.HitBoxInfo[HitBoxPair.Key].Location);
			HitBoxPair.Value->SetWorldRotation(InFramePackage.HitBoxInfo[HitBoxPair.Key].BoxRotation);
			HitBoxPair.Value->SetBoxExtent(InFramePackage.HitBoxInfo[HitBoxPair.Key].BoxExtent);
		}
		
	}
}

void ULagCompensationComponent::ResetHitBoxes(ABlasterCharacter* HitCharacter, const FFramePackage& InFramePackage)
{
	if (HitCharacter == nullptr) return;
	for (auto& HitBoxPair :  HitCharacter->HitCollisionBoxes)
	{
		if (HitBoxPair.Value != nullptr)
		{
			HitBoxPair.Value->SetWorldLocation(InFramePackage.HitBoxInfo[HitBoxPair.Key].Location);
			HitBoxPair.Value->SetWorldRotation(InFramePackage.HitBoxInfo[HitBoxPair.Key].BoxRotation);
			HitBoxPair.Value->SetBoxExtent(InFramePackage.HitBoxInfo[HitBoxPair.Key].BoxExtent);
			HitBoxPair.Value->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
		
	}
}

void ULagCompensationComponent::EnableCharacterMeshCollision(ABlasterCharacter* HitCharacter,
	ECollisionEnabled::Type NewType)
{
	if(HitCharacter == nullptr || HitCharacter->GetMesh() == nullptr) return;
	HitCharacter->GetMesh()->SetCollisionEnabled(NewType);
}

void ULagCompensationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	/*
	 * Saving the frame history 
	 */
	SaveFrameData();
	
	
}

