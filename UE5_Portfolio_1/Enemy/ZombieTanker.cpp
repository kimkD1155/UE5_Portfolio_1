// Fill out your copyright notice in the Description page of Project Settings.


#include "ZombieTanker.h"
#include "GameFramework/CharacterMovementComponent.h"

AZombieTanker::AZombieTanker()
{
	MaxHealth = 200.f;
	CoinReward = 30;
	AttackRange = 250.f;

	// 느리고 크게
	GetCharacterMovement()->MaxWalkSpeed = 200.f;
}