// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "UpgradeType.h"
#include "SaveGameSubsystem.generated.h"

class UKangSaveGame;
class AWeaponBase;

/**
 * 코인/업그레이드/일차 진행도를 디스크에 영속시키는 창구. 전부 수동 저장/불러오기다
 * (값이 바뀔 때마다 자동 저장하지 않는다 — 플레이어가 Pause 메뉴에서 명시적으로 Save 를 눌러야 기록됨).
 *
 * EnemyManager/AllyManager 같은 WorldSubsystem 은 레벨이 바뀌면 사라지지만,
 * 세이브 데이터는 메인메뉴 <-> 게임플레이 레벨 전환에도 살아있어야 하므로
 * GameInstanceSubsystem 으로 둔다.
 */
UCLASS()
class UE5_PORTFOLIO_1_API USaveGameSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	// Pause 메뉴의 Save 버튼에서 HUDComponent::SaveGame() 이 호출.
	void SaveRun(int32 InCoin, const TMap<EUpgradeType, int32>& InUpgradeLevels, int32 InDayNumber,
		const TArray<TSubclassOf<AWeaponBase>>& InUnlockedWeapons);

	// GameMode 가 게임오버 시점에 호출. 최고 기록은 계속 자동 갱신(하이스코어 성격이라 수동 저장과 별개).
	void SaveBestDay(int32 DayReached);

	// 메인 메뉴의 Start/Load 버튼이 Open Level 하기 전에 호출.
	// true = 다음에 열리는 게임플레이 레벨이 저장된 코인/업그레이드/일차를 반영한다.
	// false = 새 게임으로 시작 (기본값들 그대로).
	UFUNCTION(BlueprintCallable, Category = "Save")
	void SetLoadRequested(bool bRequested) { bLoadRequested = bRequested; }

	UFUNCTION(BlueprintPure, Category = "Save")
	bool IsLoadRequested() const { return bLoadRequested; }

	// 메인 메뉴에서 Load 버튼을 회색 처리할지 판단하는 용도.
	UFUNCTION(BlueprintPure, Category = "Save")
	bool HasSaveFile() const;

	int32 GetSavedCoin() const;
	TMap<EUpgradeType, int32> GetSavedUpgradeLevels() const;
	int32 GetSavedDayNumber() const;
	TArray<TSubclassOf<AWeaponBase>> GetSavedUnlockedWeapons() const;

	// 메인 메뉴 위젯에서 "최고 기록: Day N" 표시용.
	UFUNCTION(BlueprintPure, Category = "Save")
	int32 GetBestDayReached() const;

private:
	void LoadOrCreate();

	UPROPERTY()
	TObjectPtr<UKangSaveGame> LoadedSave = nullptr;

	bool bLoadRequested = false;
};
