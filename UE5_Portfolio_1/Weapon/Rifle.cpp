// Fill out your copyright notice in the Description page of Project Settings.


#include "Rifle.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "GameFramework/Controller.h"

ARifle::ARifle()
{
    PrimaryActorTick.bCanEverTick = true;

    WeaponType = EWeaponType::Rifle;
	WeaponName = FText::FromString(TEXT("Ak47"));

    GunData.MagazineSize = 30;
    GunData.MaxReserveAmmo = 90;
    GunData.Damage = 25.f;
    GunData.Range = 5000.f;
    GunData.FireRate = 0.12f;
    GunData.ReloadTime = 2.0f;
}

void ARifle::BeginPlay()
{
    Super::BeginPlay();
}

void ARifle::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    TickRecoilRecovery(DeltaTime);
}

void ARifle::StartFire()
{
    if (!CanFire())
    {
        //if (CurrentAmmo <= 0 && EmptySound)
        //    UGameplayStatics::PlaySoundAtLocation(this, EmptySound, GetActorLocation());
        return;
    }
    

    if (bIsAutomatic)
    {
        
        GetWorldTimerManager().SetTimer(
            FireTimerHandle,
            this,
            &ARifle::FireOnce,
            GunData.FireRate,
            true,   // 반복
            0.0f    // 즉시 첫 발
        );
    }
    else
    {
        FireOnce();
    }
}

void ARifle::StopFire()
{
    GetWorldTimerManager().ClearTimer(FireTimerHandle);
}

void ARifle::FireOnce()
{
    if (!CanFire())
    {
        StopFire();
        return;
    }

    // ── 탄약 소비 ─────────────────────────────────
    
    if (CurrentAmmo <= 0)
    {
        SetGunState(EGunState::Empty);
        return;
    }
    CurrentAmmo--;

    // ── 이펙트 ────────────────────────────────────
    /*if (MuzzleFlashEffect)
        UGameplayStatics::SpawnEmitterAttached(MuzzleFlashEffect, WeaponMesh, MuzzleSocketName);

    if (FireSound)
        UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());*/

	
    // ── 히트스캔 ──────────────────────────────────


    ACharacter* OwnerChar = Cast<ACharacter>(GetOwner());
    if (!OwnerChar)
    {
        return;
    }

    AController* Controller = OwnerChar->GetController();
    if (!Controller)
    {
        return;
    }

    FVector MuzzleLocation = WeaponMesh->GetSocketLocation(TEXT("Muzzle"));
    FVector AimPoint;

    if (AController* OwnerController = OwnerChar->GetController())
    {
        if (OwnerController->IsPlayerController())
        {
            // 1단계: 카메라 기준으로 크로스헤어가 가리키는 지점 탐색
            FVector CameraLocation;
            FRotator CameraRotation;
            OwnerController->GetPlayerViewPoint(CameraLocation, CameraRotation);
            FVector CameraTraceEnd = CameraLocation + CameraRotation.Vector() * GunData.Range;

            FHitResult CameraHit;
            FCollisionQueryParams CameraParams;
            CameraParams.AddIgnoredActor(this);
            CameraParams.AddIgnoredActor(OwnerChar);

            bool bCameraHit = GetWorld()->LineTraceSingleByChannel(
                CameraHit, CameraLocation, CameraTraceEnd, ECC_Pawn, CameraParams
            );

            // 맞았으면 충돌 지점, 안 맞았으면 사거리 끝점을 조준 목표로
            AimPoint = bCameraHit ? CameraHit.ImpactPoint : CameraTraceEnd;
        }
        else
        {
            // AI(Ally 등): 캐릭터 정면 기준
            AimPoint = OwnerChar->GetActorLocation() + OwnerChar->GetActorForwardVector() * GunData.Range;
        }
    }
    else
    {
        AimPoint = OwnerChar->GetActorLocation() + OwnerChar->GetActorForwardVector() * GunData.Range;
    }

    // 2단계: 총구에서 조준 목표를 향해 실제 데미지 판정 트레이스
    FVector TraceStart = MuzzleLocation;
    FVector AimDirection = (AimPoint - MuzzleLocation).GetSafeNormal();
    FVector TraceEnd = TraceStart + AimDirection * GunData.Range;

    FHitResult HitResult;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);
    Params.AddIgnoredActor(OwnerChar);

    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult, TraceStart, TraceEnd, ECC_Pawn, Params
    );

    PlayFireSound();
    PlayFireMontage();
    OnWeaponFired.Broadcast(); // 캐릭터 몽타주 트리거

    if (bHit && HitResult.GetActor())
    {
        UGameplayStatics::ApplyPointDamage(
            HitResult.GetActor(),
            GunData.Damage,
            AimDirection,
            HitResult,
            OwnerChar->GetController(),
            this,
            nullptr
        );
    }

#if WITH_EDITOR
    DrawDebugLine(GetWorld(), TraceStart, bHit ? HitResult.ImpactPoint : TraceEnd, FColor::Red, false, 1.f);
#endif
}