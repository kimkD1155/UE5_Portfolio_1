// Fill out your copyright notice in the Description page of Project Settings.


#include "KangPlayerGameModeBase.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "EngineUtils.h"
#include "KangPlayerState.h"
#include "KangGameState.h"
#include "UpgradeType.h"
#include "../Manager/EnemyManager.h"
#include "../Props/Barricade.h"

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

	if (UEnemyManager* EM = GetEnemyManager())
	{
		EM->OnEnemyCountChanged.AddUObject(this, &AKangPlayerGameModeBase::HandleEnemyCountChanged);
	}

	// 레벨에 배치된 바리케이드를 모아 파괴 이벤트를 구독하고 살아있는 수를 센다.
	AliveBarricadeCount = 0;
	for (TActorIterator<ABarricade> It(GetWorld()); It; ++It)
	{
		ABarricade* Barricade = *It;
		if (Barricade && !Barricade->IsDestroyed())
		{
			++AliveBarricadeCount;
			Barricade->OnBarricadeDestroyed.AddDynamic(this, &AKangPlayerGameModeBase::HandleBarricadeDestroyed);
		}
	}

	StartPhase(bStartWithDay ? EGamePhase::Day : EGamePhase::Night);

	GetWorldTimerManager().SetTimer(PhaseTickHandle, this, &AKangPlayerGameModeBase::TickPhase,
		PhaseTickInterval, true);
}

void AKangPlayerGameModeBase::StartPhase(EGamePhase NewPhase)
{
	AKangGameState* GS = GetKangGameState();
	if (!GS) return;

	PhaseElapsed = 0.f;

	switch (NewPhase)
	{
	case EGamePhase::Day:
		CurrentPhaseDuration = DayDuration;
		GS->SetSpawningActive(false);
		break;

	case EGamePhase::Night:
		CurrentPhaseDuration = NightDuration;
		GS->SetPhase(NewPhase);
		GS->UpdatePhaseTime(CurrentPhaseDuration);
		GS->SetSpawningActive(true); // 국면 전환을 스포너가 본 뒤에 스폰 창을 연다
		return;

	case EGamePhase::GameOver:
		CurrentPhaseDuration = 0.f;
		GS->SetSpawningActive(false);
		GS->SetPhase(NewPhase);
		GetWorldTimerManager().ClearTimer(PhaseTickHandle);
		UE_LOG(LogTemp, Log, TEXT("[GameMode] GAME OVER — DAY %d"), GS->GetDayNumber());
		return;
	}

	GS->SetPhase(NewPhase);
	GS->UpdatePhaseTime(CurrentPhaseDuration);
}

void AKangPlayerGameModeBase::TickPhase()
{
	AKangGameState* GS = GetKangGameState();
	if (!GS || GS->IsGameOver()) return;

	PhaseElapsed += PhaseTickInterval;
	const float Remaining = CurrentPhaseDuration - PhaseElapsed;
	GS->UpdatePhaseTime(Remaining);

	if (Remaining > 0.f) return;

	switch (GS->GetCurrentPhase())
	{
	case EGamePhase::Day:
		StartPhase(EGamePhase::Night);
		break;

	case EGamePhase::Night:
		// 스폰 창을 닫고, 남은 좀비가 없으면 즉시 낮으로. 있으면 HandleEnemyCountChanged 가 마무리.
		if (GS->IsSpawningActive())
		{
			GS->SetSpawningActive(false);
			UE_LOG(LogTemp, Log, TEXT("[GameMode] Night spawn window closed — clearing remaining enemies"));
		}
		if (UEnemyManager* EM = GetEnemyManager())
		{
			if (EM->GetEnemyCount() == 0)
			{
				AdvanceToDay();
			}
		}
		else
		{
			AdvanceToDay();
		}
		break;

	default:
		break;
	}
}

void AKangPlayerGameModeBase::HandleEnemyCountChanged(int32 NewCount)
{
	AKangGameState* GS = GetKangGameState();
	if (!GS) return;

	// 밤 스폰 창이 닫힌 뒤 마지막 좀비가 죽으면 낮으로 전환
	if (GS->GetCurrentPhase() == EGamePhase::Night && !GS->IsSpawningActive() && NewCount == 0)
	{
		AdvanceToDay();
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

void AKangPlayerGameModeBase::HandleBarricadeDestroyed(ABarricade* /*Barricade*/)
{
	AliveBarricadeCount = FMath::Max(0, AliveBarricadeCount - 1);

	AKangGameState* GS = GetKangGameState();
	if (AliveBarricadeCount == 0 && GS && GS->GetCurrentPhase() == EGamePhase::Night)
	{
		UE_LOG(LogTemp, Log, TEXT("[GameMode] All barricades destroyed"));
		TriggerGameOver();
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
