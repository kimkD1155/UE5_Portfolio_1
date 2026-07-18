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

void AEnemySpawner::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called when the game starts or when spawned
void AEnemySpawner::BeginPlay()
{
	Super::BeginPlay();

	CurrentSpawnInterval = InitialSpawnInterval; // 스폰 간격 초기화

	// 랜덤 딜레이로 첫 스폰 (모든 스포너가 동시에 스폰하지 않도록)
	float RandomDelay = FMath::RandRange(0.f, InitialSpawnInterval);
	GetWorldTimerManager().SetTimer(SpawnTimerHandle, this, &AEnemySpawner::SpawnEnemy, RandomDelay, false);

	// 일정 주기마다 난이도 상승 함수 호출 (반복 타이머)
	GetWorldTimerManager().SetTimer(DifficultyTimerHandle, this, &AEnemySpawner::UpdateDifficulty, DifficultyUpInterval, true);
}

void AEnemySpawner::SpawnEnemy()
{
	if (!EnemyClass) return; // 스폰할 적 클래스가 지정되지 않았으면 중단

	FActorSpawnParameters SpawnParams;
	// 스폰 위치에 다른 액터와 겹쳐도 가능한 위치를 조정해서라도 항상 스폰되도록 설정
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	GetWorld()->SpawnActor<ACharacter>(EnemyClass, GetActorLocation(), GetActorRotation(), SpawnParams);

	// 다음 스폰까지의 간격을 현재 간격 기준 0.5~1.5배 랜덤으로 설정 (일정하지 않게)
	float RandomInterval = FMath::RandRange(CurrentSpawnInterval * 0.5f, CurrentSpawnInterval * 1.5f);
	GetWorldTimerManager().SetTimer(SpawnTimerHandle, this, &AEnemySpawner::SpawnEnemy, RandomInterval, false);
}

void AEnemySpawner::UpdateDifficulty()
{
	// 스폰 간격을 점점 줄여서 난이도 상승 (최소값 이하로는 내려가지 않음)
	CurrentSpawnInterval = FMath::Max(CurrentSpawnInterval - SpawnIntervalDecrement, MinSpawnInterval);
	UE_LOG(LogTemp, Warning, TEXT("Spawn interval: %.1f"), CurrentSpawnInterval); // 현재 스폰 간격 로그 출력
}