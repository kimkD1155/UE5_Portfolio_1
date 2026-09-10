// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EnemySpawner.generated.h"

class UBoxComponent;
class ACharacter;

USTRUCT(BlueprintType)
struct FEnemySpawnInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<ACharacter> EnemyClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float SpawnWeight = 1.0f; // 가중치 (높을수록 더 자주 스폰)
};

/**
 * 밤(전투) 국면에만 좀비를 스폰한다.
 * 스폰 창의 개폐는 AKangGameState::OnSpawningActiveChanged 를 구독해 결정하고,
 * 난이도(스폰 간격)는 밤이 시작될 때의 DayNumber 로 계산한다.
 */
UCLASS()
class UE5_PORTFOLIO_1_API AEnemySpawner : public AActor
{
	GENERATED_BODY()

public:
	AEnemySpawner();

protected:
	virtual void BeginPlay() override;

	// AKangGameState::OnSpawningActiveChanged (다이나믹) 바인딩
	UFUNCTION()
	void HandleSpawningActiveChanged(bool bActive);

	void StartSpawning();
	void StopSpawning();

	void SpawnEnemy();
	TSubclassOf<ACharacter> SelectEnemyClass(); // 가중치 기반 랜덤 선택
	FVector GetRandomSpawnLocation();           // 박스 내 랜덤 위치

	UPROPERTY(VisibleAnywhere, Category = "Spawn")
	UBoxComponent* SpawnArea;

	UPROPERTY(EditAnywhere, Category = "Spawn")
	TArray<FEnemySpawnInfo> EnemySpawnInfos;

	// DayNumber 1 일 때의 스폰 간격(초)
	UPROPERTY(EditDefaultsOnly, Category = "Spawn")
	float InitialSpawnInterval = 5.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Spawn")
	float MinSpawnInterval = 1.0f;

	// 밤이 하루 지날 때마다 스폰 간격을 이만큼 줄인다
	UPROPERTY(EditDefaultsOnly, Category = "Spawn")
	float SpawnIntervalDecrementPerDay = 0.5f;

	float CurrentSpawnInterval = 5.0f;

	FTimerHandle SpawnTimerHandle;
};
