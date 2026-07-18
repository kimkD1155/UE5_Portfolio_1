// Fill out your copyright notice in the Description page of Project Settings.


#include "KangPlayerState.h"

void AKangPlayerState::AddCoin(int32 Amount)
{
	if (Amount <= 0) return;
	Coin += Amount;
	OnCoinChanged.Broadcast(Coin);
	UE_LOG(LogTemp, Warning, TEXT("Coin added: %d, Total: %d"), Amount, Coin);
}

bool AKangPlayerState::SpendCoin(int32 Amount)
{
	if (Amount <= 0 || Coin < Amount)
	{
		UE_LOG(LogTemp, Warning, TEXT("Not enough coin: %d / %d"), Coin, Amount);
		return false;
	}
	Coin -= Amount;
	OnCoinChanged.Broadcast(Coin);
	UE_LOG(LogTemp, Warning, TEXT("Coin spent: %d, Remaining: %d"), Amount, Coin);
	return true;
}