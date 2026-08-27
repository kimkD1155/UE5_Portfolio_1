// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../Character/EnemyCharacter.h"
#include "ZombieTanker.generated.h"

/**
 * 
 */
UCLASS()
class UE5_PORTFOLIO_1_API AZombieTanker : public AEnemyCharacter
{
	GENERATED_BODY()
	
public:
	AZombieTanker();

	virtual void Attack() override;
};
