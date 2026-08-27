// Fill out your copyright notice in the Description page of Project Settings.


#include "ZombieTanker.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"

AZombieTanker::AZombieTanker()
{
	MaxHealth = 200.f;
	CoinReward = 30;
	AttackRange = 80.f;

}

void AZombieTanker::Attack()
{
	AActor* Target = GetTargetLocation();
	if (!Target) return;

	UE_LOG(LogTemp, Warning, TEXT("ZombieNormal Attack!"));
	UGameplayStatics::ApplyDamage(Target, 30.f, GetController(), this, nullptr);
}