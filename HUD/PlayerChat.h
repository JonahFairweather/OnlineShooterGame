// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/HorizontalBox.h"
#include "Components/TextBlock.h"
#include "PlayerChat.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API UPlayerChat : public UUserWidget
{
	GENERATED_BODY()

private:

	


public:
	void AddPlayerChat(FString PlayerName, FString Message);

	UPROPERTY(meta = (BindWidget))
	UHorizontalBox* ChatBox;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* ChatText;
};
