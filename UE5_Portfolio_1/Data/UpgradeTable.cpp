// Fill out your copyright notice in the Description page of Project Settings.


#include "UpgradeTable.h"

int32 UUpgradeTable::GetCost(EUpgradeType Type, int32 CurrentLevel) const
{
	const FUpgradeDef* Def = Upgrades.Find(Type);
	if (!Def) return 0;

	return FMath::RoundToInt(Def->BaseCost * FMath::Pow(Def->CostGrowth, static_cast<float>(FMath::Max(0, CurrentLevel))));
}

int32 UUpgradeTable::GetMaxLevel(EUpgradeType Type) const
{
	const FUpgradeDef* Def = Upgrades.Find(Type);
	return Def ? Def->MaxLevel : 0;
}

float UUpgradeTable::GetPerLevelValue(EUpgradeType Type) const
{
	const FUpgradeDef* Def = Upgrades.Find(Type);
	return Def ? Def->PerLevelValue : 0.f;
}

FText UUpgradeTable::GetDisplayName(EUpgradeType Type) const
{
	const FUpgradeDef* Def = Upgrades.Find(Type);
	return Def ? Def->DisplayName : FText::GetEmpty();
}
