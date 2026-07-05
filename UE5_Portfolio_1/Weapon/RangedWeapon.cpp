// Fill out your copyright notice in the Description page of Project Settings.


#include "RangedWeapon.h"
#include "Kismet/GameplayStatics.h"

ARangedWeapon::ARangedWeapon()
{

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