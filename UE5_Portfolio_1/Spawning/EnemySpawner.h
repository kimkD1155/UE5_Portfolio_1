// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EnemySpawner.generated.h"

class UBoxComponent;
class AEnemyCharacter;

UCLASS()
class UE5_PORTFOLIO_1_API AEnemySpawner : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AEnemySpawner();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

protected:

	// 스폰할 적 클래스 (BP에서 지정)
	UPROPERTY(EditAnywhere, Category = "Spawn")
	TSubclassOf<AEnemyCharacter> EnemyClass;

	// 초기 스폰 간격 (초)
	UPROPERTY(EditDefaultsOnly, Category = "Spawn")
	float InitialSpawnInterval = 2.0f;

	// 최소 스폰 간격 (초)
	UPROPERTY(EditDefaultsOnly, Category = "Spawn")
	float MinSpawnInterval = 1.0f;

	// 스폰 간격 감소량 (난이도 업 시마다)
	UPROPERTY(EditDefaultsOnly, Category = "Spawn")
	float SpawnIntervalDecrement = 0.5f;

	// 난이도 업 주기 (초)
	UPROPERTY(EditDefaultsOnly, Category = "Spawn")
	float DifficultyUpInterval = 30.0f;



	float CurrentSpawnInterval;

	FTimerHandle SpawnTimerHandle;
	FTimerHandle DifficultyTimerHandle;

	void SpawnEnemy();
	void UpdateDifficulty();

};
