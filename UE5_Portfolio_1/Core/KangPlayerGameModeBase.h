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

	// UBarricadeManager::OnAllDestroyed 에 바인딩 — 공유 체력 풀이 0이 되면 전멸 판정
	UFUNCTION()
	void HandleBarricadesDestroyed();

	// 일시정지 메뉴(HUDComponent)가 호출. 스포너 + 아군 공격 타이머 + 밤→낮 지연 타이머를 전부 멈춘다.
	// (SetGamePaused 만으로는 FTimerManager 타이머가 안 멈추기 때문에 각 소유자에게 직접 알려야 한다.)
	// 주의: 실측 결과 UGameplayStatics::SetGamePaused 는 위젯이 자체적으로 돌리는 FTimerManager
	// 타이머까지 멈춰버린다 — Day 안내판처럼 "화면에 뜬 타이머로 자동으로 끝나야 하는" 위젯과는
	// 절대 같이 쓰면 안 된다 (안내가 영원히 안 사라짐). 그런 경우엔 SetWaveSpawningPaused 만 쓸 것.
	void SetGamePaused(bool bPaused);

	UFUNCTION(BlueprintPure, Category = "Phase")
	bool IsGamePaused() const { return bIsGamePaused; }

	// 스포너만 개별적으로 멈춘다 (SetGamePaused와 별개, 월드 전체를 멈추지 않음) — Day 안내처럼
	// 안내판 자신의 타이머는 계속 돌아가야 하는 상황에서 웨이브 스폰만 막고 싶을 때 쓴다.
	void SetWaveSpawningPaused(bool bPaused);

	// 상점 위젯의 "전투 시작" 버튼 등에서 호출 — 낮 동안에만 플레이어가 직접 밤으로 넘어간다.
	UFUNCTION(BlueprintCallable, Category = "Phase")
	void RequestStartNight();

protected:
	// 0 = 무한 모드. >0 이면 그 일수를 넘긴 낮 전환 시점에 게임 클리어.
	UPROPERTY(EditDefaultsOnly, Category = "Phase")
	int32 TargetDays = 0;

	// 이번 밤에 스폰할 좀비 총수 = BaseEnemiesPerWave + (일차-1) * EnemiesPerWaveIncrementPerDay
	UPROPERTY(EditDefaultsOnly, Category = "Phase")
	int32 BaseEnemiesPerWave = 8;

	UPROPERTY(EditDefaultsOnly, Category = "Phase")
	int32 EnemiesPerWaveIncrementPerDay = 2;

	// 이번 웨이브의 좀비를 전부 잡은 뒤 낮으로 전환되기까지의 대기 시간(초)
	UPROPERTY(EditDefaultsOnly, Category = "Phase")
	float NightToDayDelay = 3.f;

	void StartPhase(EGamePhase NewPhase);
	int32 ComputeWaveEnemyCount(int32 DayNumber) const;
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

	// AKangGameState::OnSpawningActiveChanged 바인딩 — 좀비 전멸과 스폰 창이 닫히는 시점이
	// 서로 다른 타이머로 독립적으로 발생하므로, 어느 쪽이 나중에 일어나든 놓치지 않게 양쪽에서
	// 같은 조건(CheckNightComplete)을 다시 확인한다.
	UFUNCTION()
	void HandleSpawningActiveChanged(bool bActive);

	// 밤 종료 조건(좀비 전멸 + 스폰 창 닫힘)을 확인하고, 충족되면 낮으로의 지연 전환 타이머를 건다.
	void CheckNightComplete();

	AKangGameState* GetKangGameState() const;
	UEnemyManager* GetEnemyManager() const;

	// 마지막 좀비를 잡은 뒤 낮으로 전환하기까지의 지연 타이머
	FTimerHandle NightToDayDelayHandle;

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
