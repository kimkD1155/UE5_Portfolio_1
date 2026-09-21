// Fill out your copyright notice in the Description page of Project Settings.


#include "KangPlayerGameModeBase.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "EngineUtils.h"
#include "Engine/Engine.h"
#include "KangPlayerState.h"
#include "KangGameState.h"
#include "UpgradeType.h"
#include "../Manager/EnemyManager.h"
#include "../Manager/AllyManager.h"
#include "../Manager/BarricadeManager.h"
#include "../Spawning/EnemySpawner.h"
#include "../Character/Ally/AllyBase.h"
#include "SaveGameSubsystem.h"

AKangPlayerGameModeBase::AKangPlayerGameModeBase()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/Character/Blueprint/BP_KangPlayer"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;
	PlayerStateClass = AKangPlayerState::StaticClass();
	GameStateClass = AKangGameState::StaticClass();
}

void AKangPlayerGameModeBase::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogTemp, Log, TEXT("[GameMode] BeginPlay: %s in world %s"), *GetName(), *GetWorld()->GetMapName());

	if (UEnemyManager* EM = GetEnemyManager())
	{
		EM->OnEnemyCountChanged.AddUObject(this, &AKangPlayerGameModeBase::HandleEnemyCountChanged);
	}

	// 마지막 좀비가 죽는 시점과 스포너가 웨이브 소진을 감지해 스폰 창을 닫는 시점은 서로 다른
	// 독립적인 타이머로 발생한다 — 좀비가 스폰 창이 닫히기 "전에" 죽을 수도 있다. 좀비 카운트
	// 변화만 구독하면 그 경우 전환 조건(카운트 0 + 스폰 창 닫힘)이 동시에 성립하는 순간을
	// 영영 못 보고 넘어가버리므로, 스폰 창이 닫히는 이벤트에서도 같은 조건을 다시 확인한다.
	if (AKangGameState* GS = GetKangGameState())
	{
		GS->OnSpawningActiveChanged.AddDynamic(this, &AKangPlayerGameModeBase::HandleSpawningActiveChanged);
	}

	// 바리케이드는 이제 여러 조각이 하나의 공유 체력 풀을 쓴다 — 개별로 셀 필요 없이
	// 풀이 바닥나는 순간(전부 동시 파괴) 한 번만 알려주면 된다.
	if (UBarricadeManager* BM = GetWorld()->GetSubsystem<UBarricadeManager>())
	{
		BM->OnAllDestroyed.AddDynamic(this, &AKangPlayerGameModeBase::HandleBarricadesDestroyed);
	}

	AnnouncePhase(TEXT("🎮 게임 시작"), FColor::White, /*Key=*/9100);

	// 메인 메뉴에서 "Load" 로 들어왔으면 저장된 일차부터 시작한다.
	// (코인/업그레이드는 AKangPlayerState::BeginPlay 가 같은 플래그를 보고 자기 몫을 반영함)
	if (USaveGameSubsystem* Save = GetGameInstance() ? GetGameInstance()->GetSubsystem<USaveGameSubsystem>() : nullptr)
	{
		if (Save->IsLoadRequested())
		{
			if (AKangGameState* GS = GetKangGameState())
			{
				GS->SetDayNumber(Save->GetSavedDayNumber());
			}
		}
	}

	// New Game / Load 모두 항상 밤부터 시작한다. 낮은 상점에서 "전투 시작"을 눌러야 넘어간다.
	StartPhase(EGamePhase::Night);
}

void AKangPlayerGameModeBase::AnnouncePhase(const FString& Message, const FColor& Color, int32 Key, float Duration) const
{
#if ENABLE_DRAW_DEBUG
	if (bShowPhaseAnnouncements && GEngine)
	{
		GEngine->AddOnScreenDebugMessage(Key, Duration, Color, Message);
	}
#endif
}

void AKangPlayerGameModeBase::StartPhase(EGamePhase NewPhase)
{
	AKangGameState* GS = GetKangGameState();
	if (!GS) return;

	// 새 국면을 시작하면 대기 중이던 밤→낮 지연 전환은 취소한다.
	GetWorldTimerManager().ClearTimer(NightToDayDelayHandle);

	switch (NewPhase)
	{
	case EGamePhase::Day:
		GS->SetSpawningActive(false);
		AnnouncePhase(FString::Printf(TEXT("☀ DAY %d 시작 — 상점을 이용하세요"), GS->GetDayNumber()), FColor::Cyan, /*Key=*/9101);
		break;

	case EGamePhase::Night:
	{
		const int32 WaveCount = ComputeWaveEnemyCount(GS->GetDayNumber());
		GS->SetPhase(NewPhase);
		GS->SetEnemiesRemainingToSpawn(WaveCount);
		GS->SetSpawningActive(true); // 국면 전환을 스포너가 본 뒤에 스폰 창을 연다
		AnnouncePhase(FString::Printf(TEXT("🌙 NIGHT %d 시작 — 방어하세요 (총 %d마리)"), GS->GetDayNumber(), WaveCount), FColor::Purple, /*Key=*/9101);

		int32 SpawnerCount = 0;
		for (TActorIterator<AEnemySpawner> It(GetWorld()); It; ++It) { ++SpawnerCount; }

		UE_LOG(LogTemp, Log, TEXT("[GameMode] Night %d started — wave quota = %d (Base=%d, Increment=%d), %d EnemySpawner(s) in level"),
			GS->GetDayNumber(), WaveCount, BaseEnemiesPerWave, EnemiesPerWaveIncrementPerDay, SpawnerCount);
		return;
	}

	case EGamePhase::GameOver:
		GS->SetSpawningActive(false);
		GS->SetPhase(NewPhase);
		AnnouncePhase(FString::Printf(TEXT("💀 게임 종료 — DAY %d 에서 전멸"), GS->GetDayNumber()), FColor::Red, /*Key=*/9101, /*Duration=*/8.f);
		UE_LOG(LogTemp, Log, TEXT("[GameMode] GAME OVER — DAY %d"), GS->GetDayNumber());
		if (USaveGameSubsystem* Save = GetGameInstance() ? GetGameInstance()->GetSubsystem<USaveGameSubsystem>() : nullptr)
		{
			Save->SaveBestDay(GS->GetDayNumber());
		}
		return;
	}

	GS->SetPhase(NewPhase);
}

int32 AKangPlayerGameModeBase::ComputeWaveEnemyCount(int32 DayNumber) const
{
	return BaseEnemiesPerWave + FMath::Max(0, DayNumber - 1) * EnemiesPerWaveIncrementPerDay;
}

void AKangPlayerGameModeBase::RequestStartNight()
{
	const AKangGameState* GS = GetKangGameState();
	if (!GS || GS->GetCurrentPhase() != EGamePhase::Day) return;

	StartPhase(EGamePhase::Night);
}

void AKangPlayerGameModeBase::HandleEnemyCountChanged(int32 /*NewCount*/)
{
	CheckNightComplete();
}

void AKangPlayerGameModeBase::HandleSpawningActiveChanged(bool bActive)
{
	// 스폰 창이 닫히는 시점에도 이미 좀비가 다 죽어있을 수 있다 — 마지막 좀비가 스폰 창이
	// 닫히기 "전에" 죽어서 HandleEnemyCountChanged 호출 시점엔 SpawningActive 가 아직
	// true였던 경우, 그때는 조건이 안 맞아 전환 타이머가 안 걸렸으므로 여기서 다시 확인한다.
	if (!bActive)
	{
		CheckNightComplete();
	}
}

void AKangPlayerGameModeBase::CheckNightComplete()
{
	AKangGameState* GS = GetKangGameState();
	UEnemyManager* EM = GetEnemyManager();
	if (!GS || !EM) return;

	// 이번 웨이브 스폰이 끝나고(스폰 창이 닫히고) 마지막 좀비까지 죽으면,
	// 잠깐 대기했다가 낮으로 전환한다 (즉시 전환하면 승리감을 느낄 새 없이 화면이 바뀜).
	if (GS->GetCurrentPhase() == EGamePhase::Night && !GS->IsSpawningActive() && EM->GetEnemyCount() == 0
		&& !GetWorldTimerManager().IsTimerActive(NightToDayDelayHandle))
	{
		UE_LOG(LogTemp, Log, TEXT("[GameMode] Night %d ending: enemy count hit 0 while spawn window closed (remaining quota=%d) — advancing to Day in %.1fs"),
			GS->GetDayNumber(), GS->GetEnemiesRemainingToSpawn(), NightToDayDelay);
		GetWorldTimerManager().SetTimer(NightToDayDelayHandle, this,
			&AKangPlayerGameModeBase::AdvanceToDay, NightToDayDelay, false);
	}
}

void AKangPlayerGameModeBase::AdvanceToDay()
{
	AKangGameState* GS = GetKangGameState();
	if (!GS) return;

	const int32 NextDay = GS->GetDayNumber() + 1;
	GS->SetDayNumber(NextDay);

	if (TargetDays > 0 && NextDay > TargetDays)
	{
		// 무한 모드에서는 TargetDays 가 0 이라 이 경로를 타지 않는다. 확장 대비로만 둔다.
		UE_LOG(LogTemp, Log, TEXT("[GameMode] Reached target day %d"), TargetDays);
	}

	StartPhase(EGamePhase::Day);
}

void AKangPlayerGameModeBase::TriggerGameOver()
{
	AKangGameState* GS = GetKangGameState();
	if (GS && GS->IsGameOver()) return;

	StartPhase(EGamePhase::GameOver);
}

void AKangPlayerGameModeBase::NotifyPlayerDied()
{
	UE_LOG(LogTemp, Log, TEXT("[GameMode] Player died"));
	TriggerGameOver();
}

void AKangPlayerGameModeBase::HandleBarricadesDestroyed()
{
	const AKangGameState* GS = GetKangGameState();
	if (GS && GS->GetCurrentPhase() == EGamePhase::Night)
	{
		UE_LOG(LogTemp, Log, TEXT("[GameMode] Shared barricade HP pool depleted — all destroyed"));
		TriggerGameOver();
	}
}

void AKangPlayerGameModeBase::SetGamePaused(bool bPaused)
{
	if (bIsGamePaused == bPaused) return;
	bIsGamePaused = bPaused;

	UGameplayStatics::SetGamePaused(GetWorld(), bPaused);

	// SetGamePaused 는 Tick 만 멈추고 FTimerManager 타이머는 그대로 굴러가므로
	// (스포너 스폰 간격, 밤→낮 지연, 아군 자동 공격) 타이머를 각자 직접 멈춰야 한다.
	if (bPaused)
	{
		GetWorldTimerManager().PauseTimer(NightToDayDelayHandle);
	}
	else
	{
		GetWorldTimerManager().UnPauseTimer(NightToDayDelayHandle);
	}

	SetWaveSpawningPaused(bPaused);

	if (UAllyManager* AllyMgr = GetWorld()->GetSubsystem<UAllyManager>())
	{
		for (AAllyBase* Ally : AllyMgr->GetActiveAllies())
		{
			if (Ally) Ally->SetPaused(bPaused);
		}
	}
}

void AKangPlayerGameModeBase::SetWaveSpawningPaused(bool bPaused)
{
	TArray<AActor*> Spawners;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AEnemySpawner::StaticClass(), Spawners);
	for (AActor* SpawnerActor : Spawners)
	{
		if (AEnemySpawner* Spawner = Cast<AEnemySpawner>(SpawnerActor))
		{
			Spawner->SetSpawningPaused(bPaused);
		}
	}
}

AKangGameState* AKangPlayerGameModeBase::GetKangGameState() const
{
	return GetGameState<AKangGameState>();
}

UEnemyManager* AKangPlayerGameModeBase::GetEnemyManager() const
{
	return GetWorld() ? GetWorld()->GetSubsystem<UEnemyManager>() : nullptr;
}

// ── 디버그 콘솔 명령 ─────────────────────────────────────────

void AKangPlayerGameModeBase::ForceNight()
{
	StartPhase(EGamePhase::Night);
}

void AKangPlayerGameModeBase::ForceDay()
{
	StartPhase(EGamePhase::Day);
}

void AKangPlayerGameModeBase::AddCoins(int32 Amount)
{
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
	{
		if (AKangPlayerState* PS = PC->GetPlayerState<AKangPlayerState>())
		{
			PS->AddCoin(Amount);
		}
	}
}

void AKangPlayerGameModeBase::GiveUpgrade(int32 UpgradeIndex)
{
	// 0=PlayerDamage 1=PlayerFireRate 2=BarricadeHealth 3=HealthRegen 4=AllyDamage
	const EUpgradeType Type = static_cast<EUpgradeType>(FMath::Clamp(UpgradeIndex, 0, 4));
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
	{
		if (AKangPlayerState* PS = PC->GetPlayerState<AKangPlayerState>())
		{
			PS->AddUpgradeLevel(Type, 1);
			UE_LOG(LogTemp, Log, TEXT("[GameMode] GiveUpgrade %d -> level %d"),
				UpgradeIndex, PS->GetUpgradeLevel(Type));
		}
	}
}
