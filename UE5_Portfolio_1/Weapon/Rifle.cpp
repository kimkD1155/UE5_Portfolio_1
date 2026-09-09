// Fill out your copyright notice in the Description page of Project Settings.


#include "Rifle.h"

ARifle::ARifle()
{
    WeaponType = EWeaponType::Rifle;
    PreferredSlot = EWeaponSlot::Primary;
    WeaponName = FText::FromString(TEXT("Ak47"));

    GunData.MagazineSize = 30;
    GunData.MaxReserveAmmo = 90;
    GunData.Damage = 25.f;
    GunData.Range = 5000.f;
    GunData.FireRate = 0.12f;
    GunData.ReloadTime = 2.0f;
    GunData.bAutomatic = true;
}
