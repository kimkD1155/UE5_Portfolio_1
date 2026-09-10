// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "GamePhase.h"
#include "KangGameState.generated.h"

// 국면이 바뀔 때 브로드캐스트 (HUD, 수색 지점, 낮/밤 비주얼 BP 등이 구독)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPhaseChanged, EGamePhase, NewPhase);
// 국면 잔여 시간이 갱신될 때 브로드캐스트 (약 1초 간격). HUD 타이머 표시용
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPhaseTimeChanged, float, Remaining);
// 스폰 창(밤 타이머) 이 열리고 닫힐 때 브로드캐스트. 스포너가 구독한다.
// 국면(Night)과 분리한 이유: 밤 타이머가 끝나도 남은 좀비를 정리할 때까지는 Night 국면이 유지되므로,
// "스폰을 멈추는 시점"은 국면 전환과 다른 신호여야 한다.
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
	FOnPhaseTimeChanged OnPhaseTimeChanged;

	UPROPERTY(BlueprintAssignable, Category = "Phase")
	FOnSpawningActiveChanged OnSpawningActiveChanged;

	UFUNCTION(BlueprintPure, Category = "Phase")
	EGamePhase GetCurrentPhase() const { return CurrentPhase; }

	UFUNCTION(BlueprintPure, Category = "Phase")
	int32 GetDayNumber() const { return DayNumber; }

	UFUNCTION(BlueprintPure, Category = "Phase")
	float GetPhaseTimeRemaining() const { return PhaseTimeRemaining; }

	UFUNCTION(BlueprintPure, Category = "Phase")
	bool IsGameOver() const { return CurrentPhase == EGamePhase::GameOver; }

	UFUNCTION(BlueprintPure, Category = "Phase")
	bool IsSpawningActive() const { return bSpawningActive; }

	// ── 아래는 GameMode 만 호출한다 ──────────────────────────────
	void SetPhase(EGamePhase NewPhase);
	void SetDayNumber(int32 NewDay) { DayNumber = NewDay; }
	// 잔여 시간을 갱신하고, 정수 초가 바뀐 경우에만 OnPhaseTimeChanged 를 브로드캐스트한다.
	void UpdatePhaseTime(float NewRemaining);
	void SetSpawningActive(bool bActive);

private:
	UPROPERTY(VisibleAnywhere, Category = "Phase")
	EGamePhase CurrentPhase = EGamePhase::None;

	UPROPERTY(VisibleAnywhere, Category = "Phase")
	int32 DayNumber = 1;

	UPROPERTY(VisibleAnywhere, Category = "Phase")
	float PhaseTimeRemaining = 0.f;

	UPROPERTY(VisibleAnywhere, Category = "Phase")
	bool bSpawningActive = false;

	int32 LastBroadcastSecond = -1;
};
