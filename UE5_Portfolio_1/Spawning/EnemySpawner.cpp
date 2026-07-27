// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemySpawner.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Character.h"


// Sets default values
AEnemySpawner::AEnemySpawner()
{
	PrimaryActorTick.bCanEverTick = false;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	SpawnArea = CreateDefaultSubobject<UBoxComponent>(TEXT("SpawnArea"));
	SpawnArea->SetupAttachment(RootComponent);
	SpawnArea->SetBoxExtent(FVector(200.f, 200.f, 100.f));
	SpawnArea->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AEnemySpawner::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called when the game starts or when spawned
void AEnemySpawner::BeginPlay()
{
	Super::BeginPlay();

	CurrentSpawnInterval = InitialSpawnInterval;

	float RandomDelay = FMath::RandRange(0.f, InitialSpawnInterval);
	GetWorldTimerManager().SetTimer(SpawnTimerHandle, this, &AEnemySpawner::SpawnEnemy, RandomDelay, false);
	GetWorldTimerManager().SetTimer(DifficultyTimerHandle, this, &AEnemySpawner::UpdateDifficulty, DifficultyUpInterval, true);
}

TSubclassOf<ACharacter> AEnemySpawner::SelectEnemyClass()
{
    if (EnemySpawnInfos.Num() == 0) return nullptr;

    // 전체 가중치 합산
    float TotalWeight = 0.f;
    for (const FEnemySpawnInfo& Info : EnemySpawnInfos)
    {
        TotalWeight += Info.SpawnWeight;
    }

    // 랜덤 값 뽑기
    float RandValue = FMath::RandRange(0.f, TotalWeight);
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
        FMath::RandRange(Origin.X - Extent.X, Origin.X + Extent.X),
        FMath::RandRange(Origin.Y - Extent.Y, Origin.Y + Extent.Y),
        Origin.Z
    );
}

void AEnemySpawner::SpawnEnemy()
{
    TSubclassOf<ACharacter> SelectedClass = SelectEnemyClass();
    if (!SelectedClass) return;

    FVector SpawnLocation = GetRandomSpawnLocation();
    FRotator SpawnRotation = GetActorRotation();

    FActorSpawnParameters Params;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
    GetWorld()->SpawnActor<ACharacter>(SelectedClass, SpawnLocation, SpawnRotation, Params);

    float RandomInterval = FMath::RandRange(CurrentSpawnInterval * 0.5f, CurrentSpawnInterval * 1.5f);
    GetWorldTimerManager().SetTimer(SpawnTimerHandle, this, &AEnemySpawner::SpawnEnemy, RandomInterval, false);
}

void AEnemySpawner::UpdateDifficulty()
{
    CurrentSpawnInterval = FMath::Max(CurrentSpawnInterval - SpawnIntervalDecrement, MinSpawnInterval);
    UE_LOG(LogTemp, Warning, TEXT("Spawn interval: %.1f"), CurrentSpawnInterval);
}