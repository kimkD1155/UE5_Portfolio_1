// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyManager.h"
#include "GameFramework/Character.h"

void UEnemyManager::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);
	ActiveEnemies.Reset();
}

void UEnemyManager::RegisterEnemy(ACharacter* Enemy)
{
	if (!Enemy) return;

	ActiveEnemies.AddUnique(Enemy);

	// 적이 Destroy()될 때 자동으로 해제되도록 바인딩
	Enemy->OnDestroyed.AddDynamic(this, &UEnemyManager::UnregisterEnemy);

	OnEnemyCountChanged.Broadcast(ActiveEnemies.Num());
}

void UEnemyManager::UnregisterEnemy(AActor* Enemy)
{
	ACharacter* EnemyChar = Cast<ACharacter>(Enemy);
	if (!EnemyChar) return;

	if (ActiveEnemies.Remove(EnemyChar) > 0)
	{
		OnEnemyCountChanged.Broadcast(ActiveEnemies.Num());
	}
}