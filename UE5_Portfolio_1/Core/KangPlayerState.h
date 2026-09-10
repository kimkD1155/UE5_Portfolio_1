// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "UpgradeType.h"
#include "KangPlayerState.generated.h"

class UUpgradeTable;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCoinChanged, int32, CurrentCoin);
// 업그레이드 레벨이 바뀔 때 브로드캐스트. 무기/바리케이드/체력 컴포넌트가 구독해 스탯을 재계산한다.
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnUpgradesChanged);

/**
 * 런(run) 진행도의 보관소: 코인 + 업그레이드 레벨.
 * 폰이 죽거나 리스폰돼도 유지되어야 하므로 PlayerState 가 맡는다.
 */
UCLASS()
class UE5_PORTFOLIO_1_API AKangPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	// ── 코인 ─────────────────────────────────────────────
	UPROPERTY(BlueprintAssignable)
	FOnCoinChanged OnCoinChanged;

	UFUNCTION(BlueprintCallable)
	void AddCoin(int32 Amount);

	UFUNCTION(BlueprintCallable)
	bool SpendCoin(int32 Amount);

	UFUNCTION(BlueprintPure)
	int32 GetCoin() const { return Coin; }

	// ── 업그레이드 ───────────────────────────────────────
	UPROPERTY(BlueprintAssignable)
	FOnUpgradesChanged OnUpgradesChanged;

	UPROPERTY(EditDefaultsOnly, Category = "Upgrade")
	TObjectPtr<UUpgradeTable> UpgradeTable;

	UFUNCTION(BlueprintPure, Category = "Upgrade")
	int32 GetUpgradeLevel(EUpgradeType Type) const;

	// 배율형 소비처용: 1 + Level * PerLevelValue. (재생형은 GetUpgradeLevel 로 직접 계산)
	UFUNCTION(BlueprintPure, Category = "Upgrade")
	float GetUpgradeMultiplier(EUpgradeType Type) const;

	// 다음 레벨 비용 (최대치면 0)
	UFUNCTION(BlueprintPure, Category = "Upgrade")
	int32 GetUpgradeCost(EUpgradeType Type) const;

	UFUNCTION(BlueprintPure, Category = "Upgrade")
	bool IsUpgradeMaxed(EUpgradeType Type) const;

	UFUNCTION(BlueprintPure, Category = "Upgrade")
	UUpgradeTable* GetUpgradeTable() const { return UpgradeTable; }

	// 비용을 지불하고 1레벨 올린다. 최대치이거나 코인이 부족하면 false.
	UFUNCTION(BlueprintCallable, Category = "Upgrade")
	bool TryPurchaseUpgrade(EUpgradeType Type);

	// 비용 무시하고 레벨을 더한다 (디버그/치트). 최대치로 클램프.
	UFUNCTION(BlueprintCallable, Category = "Upgrade")
	void AddUpgradeLevel(EUpgradeType Type, int32 Delta = 1);

private:
	UPROPERTY(VisibleAnywhere)
	int32 Coin = 100;

	UPROPERTY(VisibleAnywhere)
	TMap<EUpgradeType, int32> UpgradeLevels;
};
