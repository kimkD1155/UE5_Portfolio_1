// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../Character/EnemyCharacter.h"
#include "ZombieRanged.generated.h"


class AEnemyProjectile;
/**
 * 
 */
UCLASS()
class UE5_PORTFOLIO_1_API AZombieRanged : public AEnemyCharacter
{
	GENERATED_BODY()

public:
	AZombieRanged();

	UFUNCTION(BlueprintCallable, Category = "Attack")
	void ShootProjectile();

	UFUNCTION(BlueprintCallable, Category = "Attack")
	void SpawnProjectile();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, Category = "Ranged")
	TSubclassOf<AEnemyProjectile> ProjectileClass;

	UPROPERTY(EditDefaultsOnly, Category = "Ranged")
	float ShootRange = 1500.f;

	UPROPERTY(EditDefaultsOnly, Category = "Attack")
	FName ProjectileSpawnSocket = TEXT("AttackHitBoxSocket"); // 손/무기 소켓 이름

	UPROPERTY()
	TWeakObjectPtr<AEnemyProjectile> HeldProjectile;
};