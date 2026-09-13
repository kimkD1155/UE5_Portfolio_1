// Fill out your copyright notice in the Description page of Project Settings.


#include "ZombieRanged.h"
#include "EnemyProjectile.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"
#include "../../Manager/ActorPoolSubsystem.h"

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
}

void AZombieRanged::SpawnProjectile()
{
	if (!ProjectileClass) return;

	UActorPoolSubsystem* Pool = GetWorld()->GetSubsystem<UActorPoolSubsystem>();
	if (!Pool) return;

	const FTransform SocketTransform = GetMesh()->GetSocketTransform(ProjectileSpawnSocket);

	// Destroy/SpawnActor 대신 풀에서 재사용 — 원거리 좀비가 많을수록 스폰/파괴 비용을 아낀다.
	AEnemyProjectile* Projectile = Pool->Acquire<AEnemyProjectile>(
		ProjectileClass, SocketTransform.GetLocation(), SocketTransform.Rotator());

	if (Projectile)
	{
		Projectile->SetOwner(this);
		Projectile->SetInstigator(this);

		Projectile->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, ProjectileSpawnSocket);
		Projectile->CollisionComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Projectile->ProjectileMovement->Deactivate(); // 아직 안 날아감

		HeldProjectile = Projectile;
	}
}

void AZombieRanged::ShootProjectile()
{
	AEnemyProjectile* Projectile = HeldProjectile.Get();
	if (!Projectile) return;


	Projectile->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	Projectile->CollisionComp->IgnoreActorWhenMoving(this, true);
	Projectile->ProjectileMovement->ProjectileGravityScale = 0.2f; // 중력 없음
	Projectile->CollisionComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

	FVector SpawnLocation = Projectile->GetActorLocation();
	FVector TargetLocation;

	if (AActor* Target = /* Blackboard TargetActor 가져오는 방식대로 */ nullptr)
	{
		TargetLocation = Target->GetActorLocation();
	}
	else
	{
		TargetLocation = SpawnLocation + GetActorForwardVector() * 1000.f;
	}

	

	FRotator ShootRotation = (TargetLocation - SpawnLocation).Rotation();
	Projectile->SetActorRotation(ShootRotation);

	Projectile->ProjectileMovement->Activate();
	Projectile->ProjectileMovement->Velocity = ShootRotation.Vector() * Projectile->ProjectileMovement->InitialSpeed;

	HeldProjectile = nullptr;
}

