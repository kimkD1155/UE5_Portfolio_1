// Fill out your copyright notice in the Description page of Project Settings.


#include "Pistol.h"

APistol::APistol()
{
	// 권총 기본 데이터 (BP에서 덮어쓰기 가능)
	GunData.MagazineSize = 12;
	GunData.MaxReserveAmmo = 60;
	GunData.Damage = 20.f;
	GunData.Range = 5000.f;
	GunData.FireRate = 0.f;      // 반자동이라 미사용
	GunData.ReloadTime = 1.5f;
	GunData.bAutomatic = false;

	WeaponType = EWeaponType::Pistol;
	PreferredSlot = EWeaponSlot::Secondary;
	WeaponName = FText::FromString(TEXT("Eagle"));
}
