// Fill out your copyright notice in the Description page of Project Settings.


#include "RangedWeapon.h"
#include "Kismet/GameplayStatics.h"

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
}



void ARangedWeapon::StartFire()
{
    UE_LOG(LogTemp, Warning, TEXT("RangedWeapon: StartFire"));
}

void ARangedWeapon::StopFire()
{
    UE_LOG(LogTemp, Warning, TEXT("RangedWeapon: StopFire"));
}

void ARangedWeapon::StartAim()
{
    UE_LOG(LogTemp, Warning, TEXT("RangedWeapon: StartAim"));
}

void ARangedWeapon::StopAim()
{
    UE_LOG(LogTemp, Warning, TEXT("RangedWeapon: StopAim"));
}

void ARangedWeapon::Reload()
{
    SetGunState(EGunState::Reloading);
    PlayReloadMontage();
}

void ARangedWeapon::ReloadFinished()
{
    int32 Needed = GunData.MagazineSize - CurrentAmmo;
    int32 ToAdd = FMath::Min(Needed, ReserveAmmo);
    CurrentAmmo += ToAdd;
    ReserveAmmo -= ToAdd;
    SetGunState(CurrentAmmo > 0 ? EGunState::Idle : EGunState::Empty);
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

void ARangedWeapon::ApplyRecoilKick()
{
    if (!OwnerCharacter) return;

    AController* PC = OwnerCharacter->GetController();
    if (!PC) return;

    float YawKick = FMath::FRandRange(RecoilYawKickMin, RecoilYawKickMax);

    CurrentRecoilPitchOffset = FMath::Clamp(CurrentRecoilPitchOffset + RecoilPitchKick, 0.0f, MaxRecoilPitchOffset);
    CurrentRecoilYawOffset += YawKick;

    FRotator Rot = PC->GetControlRotation();
    Rot.Pitch += RecoilPitchKick;
    Rot.Yaw += YawKick;
    PC->SetControlRotation(Rot);
}

void ARangedWeapon::TickRecoilRecovery(float DeltaTime)
{
    if (!OwnerCharacter) return;

    AController* PC = OwnerCharacter->GetController();
    if (!PC) return;

    FRotator Rot = PC->GetControlRotation();
    bool bDirty = false;

    if (!FMath::IsNearlyZero(CurrentRecoilPitchOffset, 0.01f))
    {
        float Prev = CurrentRecoilPitchOffset;
        CurrentRecoilPitchOffset = FMath::FInterpTo(CurrentRecoilPitchOffset, 0.0f, DeltaTime, RecoilRecoverySpeed);
        Rot.Pitch -= (Prev - CurrentRecoilPitchOffset);
        bDirty = true;
    }
    else
    {
        CurrentRecoilPitchOffset = 0.0f;
    }

    if (!FMath::IsNearlyZero(CurrentRecoilYawOffset, 0.01f))
    {
        float Prev = CurrentRecoilYawOffset;
        CurrentRecoilYawOffset = FMath::FInterpTo(CurrentRecoilYawOffset, 0.0f, DeltaTime, RecoilRecoverySpeed);
        Rot.Yaw -= (Prev - CurrentRecoilYawOffset);
        bDirty = true;
    }
    else
    {
        CurrentRecoilYawOffset = 0.0f;
    }

    if (bDirty)
    {
        PC->SetControlRotation(Rot);
    }
}