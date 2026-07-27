// Fill out your copyright notice in the Description page of Project Settings.


#include "ZombieRanged.h"
#include "EnemyProjectile.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/CharacterMovementComponent.h"

AZombieRanged::AZombieRanged()
{
	MaxHealth = 30.f;
	CoinReward = 20;
	AttackRange = 1000.f;
	GetCharacterMovement()->MaxWalkSpeed = 250.f;
}

void AZombieRanged::BeginPlay()
{
	Super::BeginPlay();
	GetWorldTimerManager().SetTimer(ShootTimerHandle, this, &AZombieRanged::ShootProjectile, ShootInterval, true);
}

void AZombieRanged::ShootProjectile()
{
	if (!ProjectileClass) return;

	// 바리케이드 또는 플레이어 방향으로 발사
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (!PC) return;

	APawn* Player = PC->GetPawn();
	if (!Player) return;

	float DistToPlayer = FVector::Dist(GetActorLocation(), Player->GetActorLocation());
	if (DistToPlayer > ShootRange) return;

	FVector Direction = (Player->GetActorLocation() - GetActorLocation()).GetSafeNormal();
	FRotator SpawnRotation = Direction.Rotation();
	FVector SpawnLocation = GetActorLocation() + Direction * 50.f;

	FActorSpawnParameters Params;
	Params.Instigator = this;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	GetWorld()->SpawnActor<AEnemyProjectile>(ProjectileClass, SpawnLocation, SpawnRotation, Params);
}
