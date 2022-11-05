// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/EditableTextBox.h"
#include "PlayerChatBox.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API UPlayerChatBox : public UUserWidget
{
	GENERATED_BODY()
public:
	UPROPERTY(meta = (BindWidget))
	UEditableTextBox* PlayerTextBox;
	
};
