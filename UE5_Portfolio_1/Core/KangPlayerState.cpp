// Fill out your copyright notice in the Description page of Project Settings.


#include "KangPlayerState.h"
#include "../Data/UpgradeTable.h"

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

int32 AKangPlayerState::GetUpgradeLevel(EUpgradeType Type) const
{
	const int32* Found = UpgradeLevels.Find(Type);
	return Found ? *Found : 0;
}

float AKangPlayerState::GetUpgradeMultiplier(EUpgradeType Type) const
{
	if (!UpgradeTable) return 1.f;
	return 1.f + GetUpgradeLevel(Type) * UpgradeTable->GetPerLevelValue(Type);
}

int32 AKangPlayerState::GetUpgradeCost(EUpgradeType Type) const
{
	if (!UpgradeTable || IsUpgradeMaxed(Type)) return 0;
	return UpgradeTable->GetCost(Type, GetUpgradeLevel(Type));
}

bool AKangPlayerState::IsUpgradeMaxed(EUpgradeType Type) const
{
	if (!UpgradeTable) return true;
	return GetUpgradeLevel(Type) >= UpgradeTable->GetMaxLevel(Type);
}

bool AKangPlayerState::TryPurchaseUpgrade(EUpgradeType Type)
{
	if (!UpgradeTable || IsUpgradeMaxed(Type)) return false;

	const int32 Cost = UpgradeTable->GetCost(Type, GetUpgradeLevel(Type));
	if (!SpendCoin(Cost)) return false;

	UpgradeLevels.FindOrAdd(Type)++;
	OnUpgradesChanged.Broadcast();
	return true;
}

void AKangPlayerState::AddUpgradeLevel(EUpgradeType Type, int32 Delta)
{
	const int32 Max = UpgradeTable ? UpgradeTable->GetMaxLevel(Type) : 99;
	int32& Level = UpgradeLevels.FindOrAdd(Type);
	Level = FMath::Clamp(Level + Delta, 0, Max);
	OnUpgradesChanged.Broadcast();
}
