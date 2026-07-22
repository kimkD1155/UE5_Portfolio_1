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

	void ShootProjectile();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, Category = "Ranged")
	TSubclassOf<AEnemyProjectile> ProjectileClass;

	UPROPERTY(EditDefaultsOnly, Category = "Ranged")
	float ShootInterval = 2.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Ranged")
	float ShootRange = 1500.f;

	FTimerHandle ShootTimerHandle;
};
