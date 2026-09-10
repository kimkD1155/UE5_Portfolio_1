// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemySpawner.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Character.h"
#include "../Manager/EnemyManager.h"
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

void AEnemySpawner::SpawnEnemy()
{
	// 스폰 창이 닫혔으면 재예약하지 않는다 (StopSpawning 의 ClearTimer 와 이중 안전장치)
	if (AKangGameState* GS = GetWorld()->GetGameState<AKangGameState>())
	{
		if (!GS->IsSpawningActive()) return;
	}

	TSubclassOf<ACharacter> SelectedClass = SelectEnemyClass();
	if (SelectedClass)
	{
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		ACharacter* SpawnedEnemy = GetWorld()->SpawnActor<ACharacter>(
			SelectedClass, GetRandomSpawnLocation(), GetActorRotation(), Params);

		if (SpawnedEnemy)
		{
			if (UEnemyManager* Manager = GetWorld()->GetSubsystem<UEnemyManager>())
			{
				Manager->RegisterEnemy(SpawnedEnemy);
			}
		}
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
