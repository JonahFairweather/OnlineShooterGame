

#include "ControlPath.h"


AControlPath::AControlPath()
{
 	
	PrimaryActorTick.bCanEverTick = false;

}

FVector AControlPath::GetPatrolPoint(int32 const Index) const
{
	if(Index < 0 || Index > Data.Num() - 1) return FVector();
	return Data[Index];
}

int32 AControlPath::Num() const
{
	return Data.Num();
}


