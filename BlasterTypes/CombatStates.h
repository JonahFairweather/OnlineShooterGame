#pragma once
#include "UObject/ObjectMacros.h"

UENUM(BlueprintType)
enum class ECombatState : uint8
{
	ECS_Reloading UMETA(DisplayName = "Reloading"),
	ECS_Unoccupied UMETA(DisplayName = "Unoccupied"),
	ECS_ThrowingGrenade UMETA(DisplayName = "Throwing Grenade"),
	ECS_SwappingWeapons UMETA(DisplayName = "Swapping Weapons"),
	EWT_MAX UMETA(DisplayName = "DefaultMAX")
};