// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemySpawner.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Character.h"
#include "../Manager/EnemyManager.h"
#include "../Manager/ActorPoolSubsystem.h"
#include "../Core/KangGameState.h"

AEnemySpawner::AEnemySpawner()
{
	PrimaryActorTick.bCanEverTick = false;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	SpawnArea = CreateDefaultSubobject<UBoxComponent>(TEXT("SpawnArea"));
	SpawnArea->SetupAttachment(RootComponent);
	SpawnArea->SetBoxExtent(FVector(200.f, 200.f, 100.f));
	SpawnArea->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AEnemySpawner::BeginPlay()
{
	Super::BeginPlay();

	CurrentSpawnInterval = InitialSpawnInterval;

	if (AKangGameState* GS = GetWorld()->GetGameState<AKangGameState>())
	{
		GS->OnSpawningActiveChanged.AddDynamic(this, &AEnemySpawner::HandleSpawningActiveChanged);

		// BeginPlay 순서상 GameMode 가 이미 스폰 창을 열어둔 경우를 대비해 현재 상태를 반영한다.
		if (GS->IsSpawningActive())
		{
			StartSpawning();
		}
	}
}

void AEnemySpawner::HandleSpawningActiveChanged(bool bActive)
{
	if (bActive)
	{
		StartSpawning();
	}
	else
	{
		StopSpawning();
	}
}

void AEnemySpawner::StartSpawning()
{
	// 밤이 시작된 날짜로 난이도(스폰 간격) 계산
	int32 DayNumber = 1;
	if (AKangGameState* GS = GetWorld()->GetGameState<AKangGameState>())
	{
		DayNumber = GS->GetDayNumber();
	}
	CurrentSpawnInterval = FMath::Max(
		InitialSpawnInterval - (DayNumber - 1) * SpawnIntervalDecrementPerDay,
		MinSpawnInterval);

	const float FirstDelay = FMath::FRandRange(0.f, CurrentSpawnInterval);
	GetWorldTimerManager().SetTimer(SpawnTimerHandle, this, &AEnemySpawner::SpawnEnemy, FirstDelay, false);
}

void AEnemySpawner::StopSpawning()
{
	GetWorldTimerManager().ClearTimer(SpawnTimerHandle);
}

void AEnemySpawner::SetSpawningPaused(bool bPaused)
{
	if (bPaused)
	{
		GetWorldTimerManager().PauseTimer(SpawnTimerHandle);
	}
	else
	{
		GetWorldTimerManager().UnPauseTimer(SpawnTimerHandle);
	}
}

void AEnemySpawner::SpawnEnemy()
{
	AKangGameState* GS = GetWorld()->GetGameState<AKangGameState>();

	// 스폰 창이 닫혔으면 재예약하지 않는다 (StopSpawning 의 ClearTimer 와 이중 안전장치)
	if (GS && !GS->IsSpawningActive()) return;

	// 이번 웨이브의 스폰 수량을 다 채웠으면 스폰 창을 닫는다. 시간이 아니라 마릿수로 웨이브가 끝난다.
	// (다른 스포너들도 OnSpawningActiveChanged 를 통해 함께 멈춘다.)
	if (GS && GS->GetEnemiesRemainingToSpawn() <= 0)
	{
		UE_LOG(LogTemp, Log, TEXT("[Spawner] %s: wave quota exhausted — closing spawn window"), *GetName());
		GS->SetSpawningActive(false);
		return;
	}

	TSubclassOf<ACharacter> SelectedClass = SelectEnemyClass();
	if (SelectedClass)
	{
		// Destroy/SpawnActor 대신 풀에서 재사용 — 밤이 길어질수록 스폰/파괴 비용을 아낀다.
		UActorPoolSubsystem* Pool = GetWorld()->GetSubsystem<UActorPoolSubsystem>();
		ACharacter* SpawnedEnemy = Pool
			? Pool->Acquire<ACharacter>(SelectedClass, GetRandomSpawnLocation(), GetActorRotation())
			: nullptr;

		if (SpawnedEnemy)
		{
			if (UEnemyManager* Manager = GetWorld()->GetSubsystem<UEnemyManager>())
			{
				Manager->RegisterEnemy(SpawnedEnemy);
			}

			// 실제로 좀비가 세상에 등장했을 때만 수량을 차감한다 (스폰 시도가 아니라 성공에 걸어야
			// 스폰이 가끔 실패해도 웨이브가 목표 마릿수를 다 채우고 끝난다).
			if (GS)
			{
				GS->ConsumeSpawnQuota();
				UE_LOG(LogTemp, Log, TEXT("[Spawner] %s: spawned %s — %d left to spawn this wave"),
					*GetName(), *SelectedClass->GetName(), GS->GetEnemiesRemainingToSpawn());
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[Spawner] %s: Pool->Acquire failed for %s — will retry next tick, quota untouched"),
				*GetName(), *SelectedClass->GetName());
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[Spawner] %s: SelectEnemyClass returned none — is EnemySpawnInfos empty?"), *GetName());
	}

	const float NextInterval = FMath::FRandRange(CurrentSpawnInterval * 0.5f, CurrentSpawnInterval * 1.5f);
	GetWorldTimerManager().SetTimer(SpawnTimerHandle, this, &AEnemySpawner::SpawnEnemy, NextInterval, false);
}

TSubclassOf<ACharacter> AEnemySpawner::SelectEnemyClass()
{
	if (EnemySpawnInfos.Num() == 0) return nullptr;

	float TotalWeight = 0.f;
	for (const FEnemySpawnInfo& Info : EnemySpawnInfos)
	{
		TotalWeight += Info.SpawnWeight;
	}

	float RandValue = FMath::FRandRange(0.f, TotalWeight);
	float AccumulatedWeight = 0.f;

	for (const FEnemySpawnInfo& Info : EnemySpawnInfos)
	{
		AccumulatedWeight += Info.SpawnWeight;
		if (RandValue <= AccumulatedWeight)
		{
			return Info.EnemyClass;
		}
	}

	return EnemySpawnInfos.Last().EnemyClass;
}

FVector AEnemySpawner::GetRandomSpawnLocation()
{
	FVector Origin = SpawnArea->GetComponentLocation();
	FVector Extent = SpawnArea->GetScaledBoxExtent();

	return FVector(
		FMath::FRandRange(Origin.X - Extent.X, Origin.X + Extent.X),
		FMath::FRandRange(Origin.Y - Extent.Y, Origin.Y + Extent.Y),
		Origin.Z
	);
}
