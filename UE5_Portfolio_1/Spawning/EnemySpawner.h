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
	// 스폰 구역을 에디터에서 박스로 시각화/조절
	UPROPERTY(VisibleAnywhere)
	UBoxComponent* SpawnZone;

	// 스폰할 적 클래스 (BP에서 지정)
	UPROPERTY(EditAnywhere, Category = "Spawn")
	TSubclassOf<AEnemyCharacter> EnemyClass;

	UPROPERTY(EditAnywhere, Category = "Spawn")
	int32 SpawnCount = 1;

	void SpawnEnemies();
};
