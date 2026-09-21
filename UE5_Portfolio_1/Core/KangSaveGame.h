// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "UpgradeType.h"
#include "KangSaveGame.generated.h"

class AWeaponBase;

/**
 * 세이브 슬롯 하나에 실제로 저장되는 데이터.
 * 여기 필드를 늘리는 게 곧 "무엇을 영구 저장할지" 를 정하는 것이다.
 */
UCLASS()
class UE5_PORTFOLIO_1_API UKangSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY()
	int32 Coin = 100;

	UPROPERTY()
	TMap<EUpgradeType, int32> UpgradeLevels;

	// 한 번이라도 구매해 해금한 무기 목록 — 다음 Day부터 무료로 재구매할 수 있게 해준다.
	UPROPERTY()
	TArray<TSubclassOf<AWeaponBase>> UnlockedWeapons;

	// 지금까지 살아남은 최고 일수. 메인 메뉴에 표시하는 용도. (게임오버 시 자동 갱신)
	UPROPERTY()
	int32 BestDayReached = 0;

	// 마지막으로 수동 저장했을 때의 진행 일차. Load 시 이 날짜부터 시작한다.
	UPROPERTY()
	int32 SavedDayNumber = 1;
};
