// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemySpawner.h"
#include "Components/BoxComponent.h"
#include "../Character/EnemyCharacter.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values
AEnemySpawner::AEnemySpawner()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SpawnZone = CreateDefaultSubobject<UBoxComponent>(TEXT("SpawnZone"));
	RootComponent = SpawnZone;
	SpawnZone->SetBoxExtent(FVector(500.f, 500.f, 50.f));
	SpawnZone->SetCollisionEnabled(ECollisionEnabled::NoCollision); // 시각화 + 범위 계산용
}

// Called when the game starts or when spawned
void AEnemySpawner::BeginPlay()
{
	Super::BeginPlay();
	SpawnEnemies();
}

// Called every frame
void AEnemySpawner::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AEnemySpawner::SpawnEnemies()
{
	if (!EnemyClass) return;

	const FVector Origin = SpawnZone->GetComponentLocation();
	const FVector Extent = SpawnZone->GetScaledBoxExtent();

	for (int32 i = 0; i < SpawnCount; ++i)
	{
		// 박스 범위 안에서 랜덤 위치 계산
		FVector RandomLocation = FVector(
			Origin.X + FMath::RandRange(-Extent.X, Extent.X),
			Origin.Y + FMath::RandRange(-Extent.Y, Extent.Y),
			Origin.Z
		);

		FRotator SpawnRotation = FRotator(0.f, FMath::RandRange(0.f, 360.f), 0.f);

		GetWorld()->SpawnActor<AEnemyCharacter>(EnemyClass, RandomLocation, SpawnRotation);
	}
}