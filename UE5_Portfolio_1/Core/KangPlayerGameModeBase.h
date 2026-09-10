// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GamePhase.h"
#include "KangPlayerGameModeBase.generated.h"

class AKangGameState;
class UEnemyManager;

/**
 * 낮/밤 국면 FSM 의 권위자.
 * 국면 타이머를 굴리고, 전이 조건(시간 만료, 밤 정리 완료)과
 * 패배 조건(바리케이드 전멸, 플레이어 사망)을 판정한다.
 * "상태"는 AKangGameState 가 들고 있고 이 클래스는 규칙만 담당한다.
 */
UCLASS()
class UE5_PORTFOLIO_1_API AKangPlayerGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	AKangPlayerGameModeBase();

	virtual void BeginPlay() override;

	// AKangPlayerCharacter::HandleDeath 에서 호출 — 즉시 게임오버
	void NotifyPlayerDied();

	// ABarricade::OnBarricadeDestroyed 에 바인딩 (Phase C)
	UFUNCTION()
	void HandleBarricadeDestroyed(class ABarricade* Barricade);

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Phase")
	float DayDuration = 60.f;

	UPROPERTY(EditDefaultsOnly, Category = "Phase")
	float NightDuration = 90.f;

	// 0 = 무한 모드. >0 이면 그 일수를 넘긴 낮 전환 시점에 게임 클리어.
	UPROPERTY(EditDefaultsOnly, Category = "Phase")
	int32 TargetDays = 0;

	UPROPERTY(EditDefaultsOnly, Category = "Phase")
	bool bStartWithDay = true;

	// 국면 타이머 갱신 주기
	UPROPERTY(EditDefaultsOnly, Category = "Phase")
	float PhaseTickInterval = 0.25f;

	void StartPhase(EGamePhase NewPhase);
	void TickPhase();
	void AdvanceToDay();
	void TriggerGameOver();

	// UEnemyManager::OnEnemyCountChanged (비-다이나믹 멀티캐스트) 바인딩 대상
	void HandleEnemyCountChanged(int32 NewCount);

	AKangGameState* GetKangGameState() const;
	UEnemyManager* GetEnemyManager() const;

	FTimerHandle PhaseTickHandle;
	float PhaseElapsed = 0.f;
	float CurrentPhaseDuration = 0.f;

	int32 AliveBarricadeCount = 0;

	// ── 디버그 콘솔 명령 (~ 콘솔에서 호출) ─────────────────────
	UFUNCTION(Exec)
	void ForceNight();

	UFUNCTION(Exec)
	void ForceDay();

	UFUNCTION(Exec)
	void AddCoins(int32 Amount);

	// 0=공격력 1=공격속도 2=바리케이드체력 3=체력재생 4=아군공격력
	UFUNCTION(Exec)
	void GiveUpgrade(int32 UpgradeIndex);
};
