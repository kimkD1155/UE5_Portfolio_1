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

	// 일시정지 메뉴(HUDComponent)가 호출. 국면 타이머 + 스포너 + 아군 공격 타이머를 전부 멈춘다.
	// (SetGamePaused 만으로는 FTimerManager 타이머가 안 멈추기 때문에 각 소유자에게 직접 알려야 한다.)
	void SetGamePaused(bool bPaused);

	UFUNCTION(BlueprintPure, Category = "Phase")
	bool IsGamePaused() const { return bIsGamePaused; }

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

	// PIE 디버그: 게임 시작 / 낮·밤 전환 / 게임오버 시 화면에 큼직하게 안내 문구를 띄운다.
	// 진짜 UI(UPhaseWidget/UResultWidget)를 아직 BP로 안 만들었어도 루프가 눈에 보이게.
	// Shipping 빌드에서는 #if ENABLE_DRAW_DEBUG 로 컴파일 제외.
	UPROPERTY(EditDefaultsOnly, Category = "Debug")
	bool bShowPhaseAnnouncements = true;

	// Key 를 다르게 주면 서로 다른 문구가 화면에서 동시에 겹쳐 보인다 (같은 Key 는 최신 것으로 갱신됨).
	void AnnouncePhase(const FString& Message, const FColor& Color, int32 Key, float Duration = 4.f) const;

	// UEnemyManager::OnEnemyCountChanged (비-다이나믹 멀티캐스트) 바인딩 대상
	void HandleEnemyCountChanged(int32 NewCount);

	AKangGameState* GetKangGameState() const;
	UEnemyManager* GetEnemyManager() const;

	FTimerHandle PhaseTickHandle;
	float PhaseElapsed = 0.f;
	float CurrentPhaseDuration = 0.f;

	int32 AliveBarricadeCount = 0;

	bool bIsGamePaused = false;

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
