// Fill out your copyright notice in the Description page of Project Settings.


#include "AllyBase.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "Components/SphereComponent.h"
#include "Components/CapsuleComponent.h"
#include "DrawDebugHelpers.h"

// 커스텀
#include "../Character/EnemyCharacter.h"
#include "../Weapon/WeaponBase.h"
#include "../Weapon/RangedWeapon.h"
#include "../Component/HealthComponent.h"
#include "../Component/CombatComponent.h"

AAllyBase::AAllyBase()
{
	PrimaryActorTick.bCanEverTick = true;

	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
	CombatComponent = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));

	AttackRangeSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AttackRangeSphere"));
	AttackRangeSphere->SetupAttachment(GetCapsuleComponent());
	AttackRangeSphere->SetSphereRadius(AttackRange);

	// 물리 충돌은 없고 오버랩 감지만 필요하므로 QueryOnly
	AttackRangeSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	AttackRangeSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	AttackRangeSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	AttackRangeSphere->SetGenerateOverlapEvents(true);

	AttackRangeSphere->OnComponentBeginOverlap.AddDynamic(this, &AAllyBase::OnEnemyEnterRange);
	AttackRangeSphere->OnComponentEndOverlap.AddDynamic(this, &AAllyBase::OnEnemyExitRange);
}

void AAllyBase::BeginPlay()
{
	Super::BeginPlay();
	EquipDefaultWeapon();

	HealthComponent->OnDeath.AddDynamic(this, &AAllyBase::HandleDeath);
}

void AAllyBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

#if WITH_EDITOR
	if (bShowAttackRangeDebug)
	{
		const FColor RangeColor = (EnemiesInRange.Num() > 0) ? FColor::Red : FColor::Green;
		DrawDebugSphere(GetWorld(), GetActorLocation(), AttackRange, 32, RangeColor, false, -1.f, 0, 2.f);

		for (AActor* Enemy : EnemiesInRange)
		{
			if (IsValid(Enemy))
			{
				DrawDebugSphere(GetWorld(), Enemy->GetActorLocation(), 50.f, 12, FColor::Yellow, false, -1.f, 0, 1.5f);
			}
		}

		if (IsValid(CurrentTarget))
		{
			DrawDebugLine(GetWorld(), GetActorLocation(), CurrentTarget->GetActorLocation(),
				FColor::Orange, false, -1.f, 0, 2.f);
		}
	}
#endif
}

void AAllyBase::HandleDeath(AActor* /*DamageInstigator*/)
{
	GetWorldTimerManager().ClearTimer(AttackTimerHandle);
	CombatComponent->StopFire();
	EnemiesInRange.Empty();
	CurrentTarget = nullptr;

	if (EquippedWeapon)
	{
		EquippedWeapon->Unequip();
		EquippedWeapon = nullptr;
	}

	SetActorEnableCollision(false);
	if (AController* AC = GetController())
	{
		AC->UnPossess();
	}
	SetLifeSpan(3.f);
}

AActor* AAllyBase::FindTarget()
{
	// 죽었거나 파괴된 적을 목록에서 정리
	EnemiesInRange.RemoveAll([](AActor* A) { return !IsValid(A); });

	if (EnemiesInRange.Num() == 0) return nullptr;

	// 거리 제곱 비교로 가장 가까운 적 탐색
	AActor* Closest = nullptr;
	float MinDist = TNumericLimits<float>::Max();

	for (AActor* Enemy : EnemiesInRange)
	{
		const float Dist = FVector::DistSquared(GetActorLocation(), Enemy->GetActorLocation());
		if (Dist < MinDist)
		{
			MinDist = Dist;
			Closest = Enemy;
		}
	}
	return Closest;
}

void AAllyBase::OnEnemyEnterRange(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// 좀비인지 확인 후에만 리스트에 추가 (다른 Pawn 오버랩 방지)
	if (Cast<AEnemyCharacter>(OtherActor))
	{
		EnemiesInRange.AddUnique(OtherActor);

		// 첫 적 감지 시 공격 타이머 시작
		if (!GetWorldTimerManager().IsTimerActive(AttackTimerHandle))
		{
			GetWorldTimerManager().SetTimer(AttackTimerHandle, this, &AAllyBase::Attack, AttackInterval, true);
		}
	}
}

void AAllyBase::OnEnemyExitRange(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	EnemiesInRange.Remove(OtherActor);

	if (CurrentTarget == OtherActor)
	{
		CurrentTarget = nullptr;
	}

	// 적이 다 사라지면 타이머 정지
	if (EnemiesInRange.Num() == 0)
	{
		GetWorldTimerManager().ClearTimer(AttackTimerHandle);
	}
}

void AAllyBase::EquipDefaultWeapon()
{
	if (!DefaultWeaponClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("AllyBase: DefaultWeaponClass not set"));
		return;
	}

	// 무기 스폰 (위치는 Equip()에서 소켓 기준으로 재조정됨)
	AWeaponBase* Weapon = GetWorld()->SpawnActor<AWeaponBase>(DefaultWeaponClass);
	if (!Weapon)
	{
		UE_LOG(LogTemp, Warning, TEXT("AllyBase: Failed to spawn weapon"));
		return;
	}

	Weapon->Equip(this);
	EquippedWeapon = Weapon;
}

void AAllyBase::Attack()
{
	CurrentTarget = FindTarget();
	if (!IsValid(CurrentTarget) || !EquippedWeapon) return;

	if (CombatComponent->IsReloading()) return;

	ARangedWeapon* RangedWeapon = Cast<ARangedWeapon>(EquippedWeapon);
	if (RangedWeapon && RangedWeapon->GetCurrentAmmo() <= 0)
	{
		CombatComponent->Reload();
		return;
	}

	FVector Direction = CurrentTarget->GetActorLocation() - GetActorLocation();
	Direction.Z = 0.f;
	SetActorRotation(Direction.Rotation());

	CombatComponent->FireOnce();
}
