// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "AllyManager.generated.h"

/**
 *
 */
class AAllySpawnPoint;
class AAllyBase;

UCLASS()
class UE5_PORTFOLIO_1_API UAllyManager : public UWorldSubsystem
{
	GENERATED_BODY()

public:
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;

    AAllySpawnPoint* GetAvailableSpawnPoint();

    // 아군이 스폰된 직후 호출 (BeginPlay). 상점 UI 가 "동료 목록"을 조회하는 용도.
    void RegisterAlly(AAllyBase* Ally);

    // 아군 액터가 파괴될 때 자동 호출 (OnDestroyed 바인딩)
    UFUNCTION()
    void UnregisterAlly(AActor* Ally);

    TArray<AAllyBase*> GetActiveAllies() const { return ActiveAllies; }

private:
    UPROPERTY()
    TArray<AAllySpawnPoint*> SpawnPoints;

    UPROPERTY()
    TArray<AAllyBase*> ActiveAllies;

    int32 NextIndex = 0;

};
