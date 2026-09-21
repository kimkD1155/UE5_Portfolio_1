// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "GamePhase.h"
#include "KangGameState.generated.h"

// 국면이 바뀔 때 브로드캐스트 (HUD, 낮/밤 비주얼 BP 등이 구독)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPhaseChanged, EGamePhase, NewPhase);
// 스폰 창(밤 웨이브)이 열리고 닫힐 때 브로드캐스트. 스포너가 구독한다.
// 국면(Night)과 분리한 이유: 이번 웨이브 스폰을 다 마쳐도 남은 좀비를 정리할 때까지는
// Night 국면이 유지되므로, "스폰을 멈추는 시점"은 국면 전환과 다른 신호여야 한다.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSpawningActiveChanged, bool, bActive);

/**
 * 게임 루프의 "상태"를 담는 조회용 홀더.
 * 규칙과 전이는 AKangPlayerGameModeBase 가 담당하고, 이 클래스는 값 + 델리게이트만 노출한다.
 * (GameMode = 권위/규칙, GameState = 조회/이벤트 라는 UE 프레임워크의 역할 분리를 따른다.)
 */
UCLASS()
class UE5_PORTFOLIO_1_API AKangGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category = "Phase")
	FOnPhaseChanged OnPhaseChanged;

	UPROPERTY(BlueprintAssignable, Category = "Phase")
	FOnSpawningActiveChanged OnSpawningActiveChanged;

	UFUNCTION(BlueprintPure, Category = "Phase")
	EGamePhase GetCurrentPhase() const { return CurrentPhase; }

	UFUNCTION(BlueprintPure, Category = "Phase")
	int32 GetDayNumber() const { return DayNumber; }

	UFUNCTION(BlueprintPure, Category = "Phase")
	bool IsGameOver() const { return CurrentPhase == EGamePhase::GameOver; }

	UFUNCTION(BlueprintPure, Category = "Phase")
	bool IsSpawningActive() const { return bSpawningActive; }

	// 이번 웨이브에서 아직 스폰되지 않은 좀비 수 (HUD 잔여 웨이브 표시 등에 사용 가능)
	UFUNCTION(BlueprintPure, Category = "Phase")
	int32 GetEnemiesRemainingToSpawn() const { return EnemiesRemainingToSpawn; }

	// ── 아래는 GameMode 만 호출한다 ──────────────────────────────
	void SetPhase(EGamePhase NewPhase);
	void SetDayNumber(int32 NewDay) { DayNumber = NewDay; }
	void SetSpawningActive(bool bActive);
	// 새 웨이브 시작 시 이번 밤에 스폰할 총량을 설정한다.
	void SetEnemiesRemainingToSpawn(int32 Count) { EnemiesRemainingToSpawn = FMath::Max(0, Count); }

	// ── 아래는 EnemySpawner 만 호출한다 ──────────────────────────
	// 좀비를 실제로 스폰·등록하는 데 성공했을 때만 호출 — 이번 웨이브 잔여 수량을 1 줄인다.
	// (스폰 시도 자체가 아니라 "성공"에 걸어야 한다. 시도 시점에 미리 깎으면, 스폰이 실패하는
	// 스포너가 있을 때 실제로는 몬스터가 몇 마리 안 나왔는데도 수량만 조용히 다 소진되어
	// 웨이브가 절반도 못 채우고 끝나버리는 버그가 생긴다.)
	void ConsumeSpawnQuota() { EnemiesRemainingToSpawn = FMath::Max(0, EnemiesRemainingToSpawn - 1); }

private:
	UPROPERTY(VisibleAnywhere, Category = "Phase")
	EGamePhase CurrentPhase = EGamePhase::None;

	UPROPERTY(VisibleAnywhere, Category = "Phase")
	int32 DayNumber = 1;

	UPROPERTY(VisibleAnywhere, Category = "Phase")
	bool bSpawningActive = false;

	UPROPERTY(VisibleAnywhere, Category = "Phase")
	int32 EnemiesRemainingToSpawn = 0;
};
