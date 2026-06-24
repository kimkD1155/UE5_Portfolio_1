// Fill out your copyright notice in the Description page of Project Settings.


#include "Pistol.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "GameFramework/Controller.h"
#include "DrawDebugHelpers.h"

APistol::APistol()
{
	// 권총 기본 데이터 (BP에서 덮어쓰기 가능)
	GunData.MagazineSize = 12;
	GunData.MaxReserveAmmo = 60;
	GunData.Damage = 20.f;
	GunData.Range = 5000.f;
	GunData.FireRate = 0.f; // 단발이라 RPM 의미 없음, 필요하면 쿨다운으로 사용
	GunData.ReloadTime = 1.5f;

	WeaponType = EWeaponType::Pistol;
	WeaponName = FText::FromString(TEXT("Eagle"));
}

void APistol::StartFire()
{
	if (!CanFire())
	{
		return;
	}

	Fire();
	SetGunState(EGunState::Firing);

	// 단발 무기: 한 번 쏘고 바로 Idle로 복귀 (자동발사 아님)
	SetGunState(EGunState::Idle);
}

void APistol::StopFire()
{
	// 단발이라 별도 처리 불필요. 연발 무기와 인터페이스 맞추기 위해 비워둠.
}

void APistol::Fire()
{
	if (CurrentAmmo <= 0)
	{
		SetGunState(EGunState::Empty);
		return;
	}

	CurrentAmmo--;

	ACharacter* OwnerChar = Cast<ACharacter>(GetOwner());
	if (!OwnerChar)
	{
		return;
	}

	AController* OwnerController = OwnerChar->GetController();
	if (!OwnerController)
	{
		return;
	}

	FVector ViewLocation;
	FRotator ViewRotation;
	OwnerController->GetPlayerViewPoint(ViewLocation, ViewRotation);

	FVector TraceEnd = ViewLocation + (ViewRotation.Vector() * TraceRange);

	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);
	QueryParams.AddIgnoredActor(OwnerChar);

	bool bHit = GetWorld()->LineTraceSingleByChannel(
		HitResult,
		ViewLocation,
		TraceEnd,
		ECC_Visibility,
		QueryParams
	);

	if (bHit && HitResult.GetActor())
	{
		UGameplayStatics::ApplyPointDamage(
			HitResult.GetActor(),
			GunData.Damage,
			HitResult.ImpactNormal,
			HitResult,
			OwnerController,
			this,
			nullptr
		);
	}

#if WITH_EDITOR
	DrawDebugLine(GetWorld(), ViewLocation, bHit ? HitResult.ImpactPoint : TraceEnd, FColor::Red, false, 1.f);
#endif

	UE_LOG(LogTemp, Warning, TEXT("Pistol Fire: Ammo %d / %d"), CurrentAmmo, GunData.MagazineSize);
}