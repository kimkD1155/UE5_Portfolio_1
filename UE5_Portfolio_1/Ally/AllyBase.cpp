// Fill out your copyright notice in the Description page of Project Settings.


#include "AllyBase.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SphereComponent.h"
#include "Components/CapsuleComponent.h"
#include "DrawDebugHelpers.h"

// 커스텀
#include "../Character/EnemyCharacter.h"
#include "../Weapon/WeaponBase.h"
#include "../Weapon/RangedWeapon.h"

// Sets default values
AAllyBase::AAllyBase()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	AttackRangeSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AttackRangeSphere"));
	AttackRangeSphere->SetupAttachment(GetCapsuleComponent()); // RootComponent → 캡슐
	AttackRangeSphere->SetSphereRadius(AttackRange);

	// 물리 충돌은 없고 오버랩 감지만 필요하므로 QueryOnly
	AttackRangeSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

	// 기본은 전부 무시, Pawn 채널만 오버랩 (좀비는 Pawn이므로)
	AttackRangeSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	AttackRangeSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    AttackRangeSphere->SetGenerateOverlapEvents(true);

	AttackRangeSphere->OnComponentBeginOverlap.AddDynamic(this, &AAllyBase::OnEnemyEnterRange);
	AttackRangeSphere->OnComponentEndOverlap.AddDynamic(this, &AAllyBase::OnEnemyExitRange);
}

// Called when the game starts or when spawned
void AAllyBase::BeginPlay()
{
	Super::BeginPlay();
    EquipDefaultWeapon();
}

// Called every frame
void AAllyBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

#if WITH_EDITOR
    if (bShowAttackRangeDebug)
    {
        // 범위 안에 적이 있으면 빨강, 없으면 초록
        FColor RangeColor = (EnemiesInRange.Num() > 0) ? FColor::Red : FColor::Green;

        DrawDebugSphere(
            GetWorld(),
            GetActorLocation(),
            AttackRange,
            32, // 세그먼트 수 (부드러움 정도)
            RangeColor,
            false, // 지속 표시 아님
            -1.f,  // 지속시간 -1 = 매 프레임 새로 그림 (Tick에서 계속 호출하므로)
            0,
            2.f    // 선 두께
        );

        // 감지된 적마다 노란 구체 표시
        for (AActor* Enemy : EnemiesInRange)
        {
            if (IsValid(Enemy))
            {
                DrawDebugSphere(
                    GetWorld(),
                    Enemy->GetActorLocation(),
                    50.f,
                    12,
                    FColor::Yellow,
                    false,
                    -1.f,
                    0,
                    1.5f
                );
            }
        }

        // 현재 타겟이 있으면 라인으로 연결해서 표시
        if (IsValid(CurrentTarget))
        {
            DrawDebugLine(
                GetWorld(),
                GetActorLocation(),
                CurrentTarget->GetActorLocation(),
                FColor::Orange,
                false,
                -1.f,
                0,
                2.f
            );
        }
    }
#endif
}



AActor* AAllyBase::FindTarget()
{
	// 죽었거나 파괴된 적(포인터는 남아있지만 IsValid 실패)을 목록에서 정리
	EnemiesInRange.RemoveAll([](AActor* A) { return !IsValid(A); });

	if (EnemiesInRange.Num() == 0) return nullptr;

	// 거리 제곱 비교로 가장 가까운 적 탐색 (제곱근 연산 생략해서 비용 절감)
	AActor* Closest = nullptr;
	float MinDist = TNumericLimits<float>::Max();

	for (AActor* Enemy : EnemiesInRange)
	{
		float Dist = FVector::DistSquared(GetActorLocation(), Enemy->GetActorLocation());
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
        UE_LOG(LogTemp, Warning, TEXT("EnemyEnter"));
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
	// 범위 벗어난 적은 목록에서 제거
	EnemiesInRange.Remove(OtherActor);

	// 벗어난 적이 현재 타겟이었다면 타겟도 비워줌 (다음 Attack에서 재탐색하도록)
	if (CurrentTarget == OtherActor)
	{
		CurrentTarget = nullptr;
	}

    // 적이 다 사라지면 타이머 정지 (불필요한 호출 방지)
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

    // 무기 스폰 (위치는 Equip()에서 소켓 기준으로 재조정되므로 임시 위치로 스폰)
    AWeaponBase* Weapon = GetWorld()->SpawnActor<AWeaponBase>(DefaultWeaponClass);
    if (!Weapon)
    {
        UE_LOG(LogTemp, Warning, TEXT("AllyBase: Failed to spawn weapon"));
        return;
    }

    // WeaponBase::Equip()이 오너 설정 + 소켓 Attach를 전부 처리함
    Weapon->Equip(this);
    EquippedWeapon = Weapon;
}

void AAllyBase::Attack()
{
    CurrentTarget = FindTarget();
    if (!IsValid(CurrentTarget) || !EquippedWeapon) return;

    ARangedWeapon* RangedWeapon = Cast<ARangedWeapon>(EquippedWeapon);

    if (RangedWeapon && RangedWeapon->GetGunState() == EGunState::Reloading) return;

    if (RangedWeapon && RangedWeapon->GetCurrentAmmo() <= 0)
    {
        Reload();
        return;
    }

    FVector Direction = CurrentTarget->GetActorLocation() - GetActorLocation();
    Direction.Z = 0.f;
    SetActorRotation(Direction.Rotation());

    EquippedWeapon->StartFire();
}

void AAllyBase::Reload()
{
    ARangedWeapon* RangedWeapon = Cast<ARangedWeapon>(EquippedWeapon);
    if (!RangedWeapon || !RangedWeapon->CanReload()) return;

    // 무기 타입에 맞는 몽타주 선택
    UAnimMontage* MontageToPlay = nullptr;
    switch (RangedWeapon->GetWeaponType())
    {
    case EWeaponType::Pistol: MontageToPlay = PistolReloadMontage; break;
    case EWeaponType::Rifle:  MontageToPlay = RifleReloadMontage; break;
    default:
        UE_LOG(LogTemp, Warning, TEXT("AllyBase: Reload not implemented for this weapon type"));
        return;
    }

    if (!MontageToPlay)
    {
        UE_LOG(LogTemp, Warning, TEXT("AllyBase: Reload montage not set"));
        return;
    }

    // 무기 쪽 상태를 Reloading으로 전환 (탄약 로직은 ReloadFinished에서 처리됨)
    RangedWeapon->Reload();
    ReloadingWeapon = RangedWeapon;

    PlayAnimMontage(MontageToPlay);

    // 몽타주 종료 시점에 콜백 연결 (AnimNotify 대신 몽타주 자체의 종료 델리게이트 사용)
    if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
    {
        FOnMontageEnded EndDelegate;
        EndDelegate.BindUObject(this, &AAllyBase::OnReloadMontageEnded);
        AnimInstance->Montage_SetEndDelegate(EndDelegate, MontageToPlay);
    }
}

void AAllyBase::OnReloadMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
    if (ReloadingWeapon)
    {
        // 정상 종료 시에만 탄약 채움, 중단되면 상태만 Idle로 복구
        if (!bInterrupted)
        {
            ReloadingWeapon->ReloadFinished();
        }
        else
        {
            ReloadingWeapon->SetGunState(EGunState::Idle);
        }
        ReloadingWeapon = nullptr;
    }
}