// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "../Core/UpgradeType.h"
#include "UpgradeTable.generated.h"

// 업그레이드 1종의 수치 정의. 로직이 아니라 데이터라서 DataAsset 으로 분리한다
// (기존 UWeaponAnimSet 과 동일한 패턴).
USTRUCT(BlueprintType)
struct FUpgradeDef
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText DisplayName;

	// 배율형(공격력/공격속도/바리케이드 체력): 레벨당 +PerLevelValue  (0.15 = 레벨당 +15%)
	// 재생형(체력 재생량): 레벨당 +PerLevelValue HP/초
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "0.0"))
	float PerLevelValue = 0.15f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "1"))
	int32 MaxLevel = 5;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "0"))
	int32 BaseCost = 50;

	// 다음 레벨 비용 = BaseCost * CostGrowth^(현재 레벨)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "1.0"))
	float CostGrowth = 1.6f;
};

/**
 * 업그레이드 5종의 수치표. BP 로 인스턴스를 만들어 AKangPlayerState 에 지정한다.
 */
UCLASS(BlueprintType)
class UE5_PORTFOLIO_1_API UUpgradeTable : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Upgrade")
	TMap<EUpgradeType, FUpgradeDef> Upgrades;

	// CurrentLevel 에서 다음 레벨로 올리는 비용. 정의가 없으면 0.
	int32 GetCost(EUpgradeType Type, int32 CurrentLevel) const;

	int32 GetMaxLevel(EUpgradeType Type) const;

	float GetPerLevelValue(EUpgradeType Type) const;

	FText GetDisplayName(EUpgradeType Type) const;
};
