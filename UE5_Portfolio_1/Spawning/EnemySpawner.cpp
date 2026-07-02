// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemySpawner.h"
#include "Components/BoxComponent.h"
#include "../Character/EnemyCharacter.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values
AEnemySpawner::AEnemySpawner()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
}

// Called when the game starts or when spawned
void AEnemySpawner::BeginPlay()
{
	Super::BeginPlay();

	CurrentSpawnInterval = InitialSpawnInterval;

	// 스폰 타이머 시작
	GetWorldTimerManager().SetTimer(SpawnTimerHandle, this, &AEnemySpawner::SpawnEnemy, CurrentSpawnInterval, true);
	GetWorldTimerManager().SetTimer(DifficultyTimerHandle, this, &AEnemySpawner::UpdateDifficulty, DifficultyUpInterval, true);
}

// Called every frame
void AEnemySpawner::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AEnemySpawner::SpawnEnemy()
{
	if (!EnemyClass) return;

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	GetWorld()->SpawnActor<ACharacter>(EnemyClass, GetActorLocation(), GetActorRotation(), SpawnParams);
}

void AEnemySpawner::UpdateDifficulty()
{
	CurrentSpawnInterval = FMath::Max(CurrentSpawnInterval - SpawnIntervalDecrement, MinSpawnInterval);
	GetWorldTimerManager().SetTimer(SpawnTimerHandle, this, &AEnemySpawner::SpawnEnemy, CurrentSpawnInterval, true);

	UE_LOG(LogTemp, Warning, TEXT("Spawn interval: %.1f"), CurrentSpawnInterval);
}