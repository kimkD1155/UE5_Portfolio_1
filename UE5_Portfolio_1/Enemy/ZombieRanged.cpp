// Fill out your copyright notice in the Description page of Project Settings.


#include "ZombieRanged.h"
#include "EnemyProjectile.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"

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

	FActorSpawnParameters Params;
	Params.Owner = this;
	Params.Instigator = this;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AEnemyProjectile* Projectile = GetWorld()->SpawnActor<AEnemyProjectile>(
		ProjectileClass,
		GetMesh()->GetSocketTransform(ProjectileSpawnSocket),
		Params);

	if (Projectile)
	{
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

	UE_LOG(LogTemp, Warning, TEXT("test"));

	FRotator ShootRotation = (TargetLocation - SpawnLocation).Rotation();
	Projectile->SetActorRotation(ShootRotation);

	Projectile->ProjectileMovement->Activate();
	Projectile->ProjectileMovement->Velocity = ShootRotation.Vector() * Projectile->ProjectileMovement->InitialSpeed;

	HeldProjectile = nullptr;

	UE_LOG(LogTemp, Warning, TEXT("IsActive=%d, TickEnabled=%d, Velocity=%s, UpdatedComp=%s"),
		Projectile->ProjectileMovement->IsActive(),
		Projectile->ProjectileMovement->IsComponentTickEnabled(),
		*Projectile->ProjectileMovement->Velocity.ToString(),
		Projectile->ProjectileMovement->UpdatedComponent ? *Projectile->ProjectileMovement->UpdatedComponent->GetName() : TEXT("null"));
}

