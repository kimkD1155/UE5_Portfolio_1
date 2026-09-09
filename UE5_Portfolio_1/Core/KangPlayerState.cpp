// Fill out your copyright notice in the Description page of Project Settings.


#include "KangPlayerState.h"

void AKangPlayerState::AddCoin(int32 Amount)
{
	if (Amount <= 0) return;
	Coin += Amount;
	OnCoinChanged.Broadcast(Coin);
	
}

bool AKangPlayerState::SpendCoin(int32 Amount)
{
	if (Amount <= 0 || Coin < Amount)
	{
		
		return false;
	}
	Coin -= Amount;
	OnCoinChanged.Broadcast(Coin);
	
	return true;
}