// Fill out your copyright notice in the Description page of Project Settings.


#include "ZombieNormal.h"
#include "Kismet/GameplayStatics.h"

AZombieNormal::AZombieNormal()
{
	MaxHealth = 40.f;
	CoinReward = 10;
	AttackRange = 200.f;
}

void AZombieNormal::Attack()
{
	AActor* Target = GetTargetLocation();
	if (!Target) return;
	UGameplayStatics::ApplyDamage(Target, 30.f, GetController(), this, nullptr);
}