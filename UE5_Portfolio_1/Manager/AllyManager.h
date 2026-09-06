// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "AllyManager.generated.h"

/**
 * 
 */
class AAllySpawnPoint;

UCLASS()
class UE5_PORTFOLIO_1_API UAllyManager : public UWorldSubsystem
{
	GENERATED_BODY()

public:
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;

    AAllySpawnPoint* GetAvailableSpawnPoint();

private:
    UPROPERTY()
    TArray<AAllySpawnPoint*> SpawnPoints;

    int32 NextIndex = 0;
	
};
