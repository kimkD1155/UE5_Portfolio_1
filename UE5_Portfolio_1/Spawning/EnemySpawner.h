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

    void SpawnEnemy();
    void UpdateDifficulty();
    TSubclassOf<ACharacter> SelectEnemyClass(); // 가중치 기반 랜덤 선택
    FVector GetRandomSpawnLocation(); // 박스 내 랜덤 위치

    UPROPERTY(VisibleAnywhere, Category = "Spawn")
    UBoxComponent* SpawnArea;

    UPROPERTY(EditAnywhere, Category = "Spawn")
    TArray<FEnemySpawnInfo> EnemySpawnInfos;

    UPROPERTY(EditDefaultsOnly, Category = "Spawn")
    float InitialSpawnInterval = 5.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Spawn")
    float MinSpawnInterval = 1.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Spawn")
    float SpawnIntervalDecrement = 0.5f;

    UPROPERTY(EditDefaultsOnly, Category = "Spawn")
    float DifficultyUpInterval = 30.0f;

    float CurrentSpawnInterval;

    FTimerHandle SpawnTimerHandle;
    FTimerHandle DifficultyTimerHandle;
};
