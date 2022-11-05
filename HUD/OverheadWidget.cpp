// Fill out your copyright notice in the Description page of Project Settings.


#include "OverheadWidget.h"
#include "Components/TextBlock.h"
#include "GameFramework/PlayerState.h"

void UOverheadWidget::SetDisplayText(FString TextToDisplay)
{
	if (DisplayText) {
		DisplayText->SetText(FText::FromString(TextToDisplay));
	}

}

void UOverheadWidget::ShowPlayerNetRole(APawn* InPawn)
{

	ENetRole LocalRole = InPawn->GetRemoteRole();
	//APlayerState* PlayerState = InPawn->GetPlayerState();
	FString PlayerName = "John";
	/*if (PlayerState) {
		 PlayerName = PlayerState->GetPlayerName();
	}*/
	
	
	

	FString Role;

	switch (LocalRole) {
	case ENetRole::ROLE_Authority:
		Role = FString("Authority");
		break;
	case ENetRole::ROLE_AutonomousProxy:
		Role = FString("Autonomous Proxy");
		break;
	case ENetRole::ROLE_SimulatedProxy:
		Role = FString("Simulated Proxy");
		break;
	case ENetRole::ROLE_None:
		Role = FString("None");
		break;
	}
	FString RemoteRoleString;
	if (!PlayerName.IsEmpty()) {
		RemoteRoleString = FString::Printf(TEXT("Local Role: %s\nPlayer Name: %s"), *Role, *PlayerName);
	}
	else {
		RemoteRoleString = FString::Printf(TEXT("Local Role: %s"), *Role);
	}
	
	SetDisplayText(RemoteRoleString);
}

void UOverheadWidget::OnLevelRemovedFromWorld(ULevel* InLevel, UWorld* InWorld)
{
	RemoveFromParent();
	Super::OnLevelRemovedFromWorld(InLevel, InWorld);
}