// Fill out your copyright notice in the Description page of Project Settings.


#include "RangedWeapon.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/Controller.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

ARangedWeapon::ARangedWeapon()
{
    PrimaryActorTick.bCanEverTick = false;
}

void ARangedWeapon::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void ARangedWeapon::BeginPlay()
{
    Super::BeginPlay();
    CurrentAmmo = GunData.MagazineSize;
    ReserveAmmo = GunData.MaxReserveAmmo;
    BroadcastAmmo();
}

void ARangedWeapon::StartFire()
{
    if (!CanFire()) return;

    SetGunState(EGunState::Firing);

    if (GunData.bAutomatic)
    {
        // 첫 발은 즉시, 이후 FireRate 간격으로 반복
        GetWorldTimerManager().SetTimer(FireTimerHandle, this, &ARangedWeapon::FireOnce,
            FMath::Max(GunData.FireRate, 0.01f), true, 0.f);
    }
    else
    {
        FireOnce();
        SetGunState(CurrentAmmo > 0 ? EGunState::Idle : EGunState::Empty);
    }
}

void ARangedWeapon::StopFire()
{
    GetWorldTimerManager().ClearTimer(FireTimerHandle);
    if (GunState == EGunState::Firing)
    {
        SetGunState(CurrentAmmo > 0 ? EGunState::Idle : EGunState::Empty);
    }
}

void ARangedWeapon::FireSingle()
{
    if (!CanFire()) return;

    SetGunState(EGunState::Firing);
    FireOnce();
    if (GunState == EGunState::Firing)
    {
        SetGunState(CurrentAmmo > 0 ? EGunState::Idle : EGunState::Empty);
    }
}

void ARangedWeapon::FireOnce()
{
    if (!CanFire())
    {
        StopFire();
        return;
    }

    CurrentAmmo--;
    BroadcastAmmo();

    ACharacter* OwnerChar = Cast<ACharacter>(GetOwner());
    if (!OwnerChar || !WeaponMesh) return;

    const float Range = GunData.Range;
    const FVector MuzzleLocation = WeaponMesh->GetSocketLocation(TEXT("Muzzle"));

    // 1단계: 조준 목표 지점 결정
    //  - 플레이어: 카메라(크로스헤어) 기준 트레이스
    //  - AI / 컨트롤러 없음: 캐릭터 정면
    FVector AimPoint;
    AController* OwnerController = OwnerChar->GetController();
    if (OwnerController && OwnerController->IsPlayerController())
    {
        FVector CamLoc;
        FRotator CamRot;
        OwnerController->GetPlayerViewPoint(CamLoc, CamRot);
        const FVector CamEnd = CamLoc + ApplyAimSpread(CamRot.Vector()) * Range;

        FHitResult CamHit;
        FCollisionQueryParams CamParams;
        CamParams.AddIgnoredActor(this);
        CamParams.AddIgnoredActor(OwnerChar);

        const bool bCamHit = GetWorld()->LineTraceSingleByChannel(CamHit, CamLoc, CamEnd, ECC_Pawn, CamParams);
        AimPoint = bCamHit ? CamHit.ImpactPoint : CamEnd;
    }
    else
    {
        AimPoint = OwnerChar->GetActorLocation() + ApplyAimSpread(OwnerChar->GetActorForwardVector()) * Range;
    }

    // 2단계: 총구 → 조준 목표 방향으로 실제 데미지 판정 트레이스
    const FVector TraceStart = MuzzleLocation;
    const FVector AimDir = (AimPoint - MuzzleLocation).GetSafeNormal();
    const FVector TraceEnd = TraceStart + AimDir * Range;

    FHitResult Hit;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(this);
    QueryParams.AddIgnoredActor(OwnerChar);
    const bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Pawn, QueryParams);

    PlayFireSound();
    PlayFireMontage();
    OnWeaponFired.Broadcast(); // 캐릭터 사격 몽타주 트리거

    if (bHit && Hit.GetActor())
    {
        UGameplayStatics::ApplyPointDamage(Hit.GetActor(), GunData.Damage, AimDir, Hit,
            OwnerChar->GetController(), this, nullptr);
    }

#if WITH_EDITOR
    DrawDebugLine(GetWorld(), TraceStart, bHit ? Hit.ImpactPoint : TraceEnd, FColor::Red, false, 1.f);
#endif

    if (CurrentAmmo <= 0)
    {
        SetGunState(EGunState::Empty);
        StopFire();
    }
}

void ARangedWeapon::StartAim()
{
    
}

void ARangedWeapon::StopAim()
{
    
}

void ARangedWeapon::Reload()
{
    SetGunState(EGunState::Reloading);
    PlayReloadMontage();
}

void ARangedWeapon::ReloadFinished()
{
    const int32 Needed = GunData.MagazineSize - CurrentAmmo;
    const int32 ToAdd = FMath::Min(Needed, ReserveAmmo);
    CurrentAmmo += ToAdd;
    ReserveAmmo -= ToAdd;
    SetGunState(CurrentAmmo > 0 ? EGunState::Idle : EGunState::Empty);
    BroadcastAmmo();
}

void ARangedWeapon::SetGunState(EGunState NewState)
{
    GunState = NewState;
}

bool ARangedWeapon::CanFire() const
{
    return CurrentAmmo > 0 && GunState != EGunState::Reloading;
}

bool ARangedWeapon::CanReload() const
{
    return GunState != EGunState::Reloading
        && CurrentAmmo < GunData.MagazineSize
        && ReserveAmmo > 0;
}

FVector ARangedWeapon::ApplyAimSpread(const FVector& AimDir) const
{
    if (AimSpread <= 0.f) return AimDir;
    float SpreadRad = FMath::DegreesToRadians(AimSpread);
    return FMath::VRandCone(AimDir, SpreadRad);
}