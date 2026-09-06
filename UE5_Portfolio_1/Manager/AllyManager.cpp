// Fill out your copyright notice in the Description page of Project Settings.


#include "AllyManager.h"
#include "Kismet/GameplayStatics.h"
#include "../Ally/AllySpawnPoint.h"

void UAllyManager::OnWorldBeginPlay(UWorld& InWorld)
{
    Super::OnWorldBeginPlay(InWorld);

    TArray<AActor*> Found;
    UGameplayStatics::GetAllActorsOfClass(&InWorld, AAllySpawnPoint::StaticClass(), Found);
    for (AActor* A : Found)
    {
        SpawnPoints.Add(Cast<AAllySpawnPoint>(A));
    }
}

AAllySpawnPoint* UAllyManager::GetAvailableSpawnPoint()
{
    if (SpawnPoints.Num() == 0) return nullptr;

    for (int32 i = 0; i < SpawnPoints.Num(); ++i)
    {
        int32 Index = (NextIndex + i) % SpawnPoints.Num();
        if (!SpawnPoints[Index]->bIsOccupied)
        {
            NextIndex = (Index + 1) % SpawnPoints.Num();
            return SpawnPoints[Index];
        }
    }
    return nullptr; // 5개 다 찬 상태
}