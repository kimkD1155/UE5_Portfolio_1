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
    if (!CanReload()) return;

    SetGunState(EGunState::Reloading);

    /*if (ReloadSound)
        UGameplayStatics::PlaySoundAtLocation(this, ReloadSound, GetActorLocation());*/

    GetWorldTimerManager().SetTimer(
        ReloadTimerHandle,
        this,
        &ARangedWeapon::OnReloadFinished,
        GunData.ReloadTime,
        false
    );
    /*GetWorldTimerManager()는 현재 월드의 타이머 매니저를 가져온다.
    SetTimer()의 각 인자는 :

    ReloadTimerHandle: 이 타이머를 식별 / 제어하기 위한 핸들(FTimerHandle 타입 변수).나중에 ClearTimer()로 취소할 때도 사용됨
    this : 타이머가 실행될 때 함수를 호출할 대상 객체(현재 ARangedWeapon 인스턴스)
    & ARangedWeapon::OnReloadFinished : 시간이 지나면 호출될 콜백 함수 포인터
    GunData.ReloadTime : 몇 초 후에 실행할지(재장전 시간, float 값)
    false : 반복 여부(false면 한 번만 실행, true면 계속 반복)*/
}

void ARangedWeapon::OnReloadFinished()
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