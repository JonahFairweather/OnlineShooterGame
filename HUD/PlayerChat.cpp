// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerChat.h"

void UPlayerChat::AddPlayerChat(FString PlayerName, FString Message)
{
	FString ChatString = FString::Printf(TEXT("%s: %s"), *PlayerName, *Message);
	ChatText->SetText(FText::FromString(ChatString));
}
