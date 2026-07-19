// Fill out your copyright notice in the Description page of Project Settings.


#include "Rifle.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"

ARifle::ARifle()
{
    WeaponType = EWeaponType::Rifle;
	WeaponName = FText::FromString(TEXT("Ak47"));

    GunData.MagazineSize = 30;
    GunData.MaxReserveAmmo = 90;
    GunData.Damage = 25.f;
    GunData.Range = 5000.f;
    GunData.FireRate = 0.1f;
    GunData.ReloadTime = 2.0f;
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
    CurrentAmmo--;
    if (CurrentAmmo <= 0)
        SetGunState(EGunState::Empty);

    // ── 이펙트 ────────────────────────────────────
    /*if (MuzzleFlashEffect)
        UGameplayStatics::SpawnEmitterAttached(MuzzleFlashEffect, WeaponMesh, MuzzleSocketName);

    if (FireSound)
        UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());*/

    // ── 히트스캔 ──────────────────────────────────
    if (!OwnerCharacter) return;

    AController* Controller = OwnerCharacter->GetController();
    if (!Controller) return;

    FVector CameraLocation;
    FRotator CameraRotation;
    Controller->GetPlayerViewPoint(CameraLocation, CameraRotation);

    FVector TraceStart = CameraLocation;
    FVector TraceEnd = CameraLocation + CameraRotation.Vector() * GunData.Range;

    FHitResult HitResult;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);
    Params.AddIgnoredActor(OwnerCharacter);

    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult, TraceStart, TraceEnd, ECC_Pawn, Params
    );

    // ── 데미지 ────────────────────────────────────
    if (bHit && HitResult.GetActor())
    {
        UGameplayStatics::ApplyPointDamage(
            HitResult.GetActor(),
            GunData.Damage,
            CameraRotation.Vector(),
            HitResult,
            Controller,
            this,
            nullptr
        );
    }

    // ── 디버그 ────────────────────────────────────
#if WITH_EDITOR
    DrawDebugLine(GetWorld(), TraceStart,
        bHit ? HitResult.ImpactPoint : TraceEnd,
        FColor::Red, false, 0.1f, 0, 1.f);
#endif
}