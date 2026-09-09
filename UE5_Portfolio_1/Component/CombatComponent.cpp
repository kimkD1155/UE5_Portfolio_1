// Fill out your copyright notice in the Description page of Project Settings.

#include "CombatComponent.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "../Interface/WeaponHolder.h"
#include "../Weapon/WeaponBase.h"
#include "../Weapon/RangedWeapon.h"
#include "../Animation/MontageHelper.h"
#include "Engine/World.h"
#include "TimerManager.h"

UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();
	OwnerCharacter = Cast<ACharacter>(GetOwner());
}

AWeaponBase* UCombatComponent::GetActiveWeapon() const
{
	// 소유 액터가 IWeaponHolder 를 구현하고 있어야 무기를 알 수 있다.
	if (IWeaponHolder* Holder = Cast<IWeaponHolder>(GetOwner()))
	{
		return Holder->GetActiveWeapon();
	}
	return nullptr;
}

ARangedWeapon* UCombatComponent::GetActiveRangedWeapon() const
{
	return Cast<ARangedWeapon>(GetActiveWeapon());
}

USkeletalMeshComponent* UCombatComponent::GetCharacterMesh() const
{
	return OwnerCharacter ? OwnerCharacter->GetMesh() : nullptr;
}

void UCombatComponent::StartFire()
{
	ARangedWeapon* Weapon = GetActiveRangedWeapon();
	if (!Weapon || IsReloading()) return;

	// 정확히 한 번만 바인딩되도록 (무기 교체와 무관하게 안전)
	Weapon->OnWeaponFired.RemoveDynamic(this, &UCombatComponent::HandleWeaponFired);
	Weapon->OnWeaponFired.AddDynamic(this, &UCombatComponent::HandleWeaponFired);

	Weapon->StartFire();
}

void UCombatComponent::StopFire()
{
	if (AWeaponBase* Weapon = GetActiveWeapon())
	{
		Weapon->StopFire();
	}
}

void UCombatComponent::FireOnce()
{
	ARangedWeapon* Weapon = GetActiveRangedWeapon();
	if (!Weapon || IsReloading()) return;

	Weapon->OnWeaponFired.RemoveDynamic(this, &UCombatComponent::HandleWeaponFired);
	Weapon->OnWeaponFired.AddDynamic(this, &UCombatComponent::HandleWeaponFired);

	Weapon->FireSingle();
}

void UCombatComponent::StartAim()
{
	if (AWeaponBase* Weapon = GetActiveWeapon())
	{
		Weapon->StartAim();
	}
}

void UCombatComponent::StopAim()
{
	if (AWeaponBase* Weapon = GetActiveWeapon())
	{
		Weapon->StopAim();
	}
}

void UCombatComponent::Reload()
{
	ARangedWeapon* Weapon = GetActiveRangedWeapon();
	if (!Weapon)
	{
		UE_LOG(LogTemp, Warning, TEXT("CombatComponent::Reload - 장착된 원거리 무기가 없음"));
		return;
	}
	if (IsReloading())
	{
		return;
	}
	if (!Weapon->CanReload())
	{
		UE_LOG(LogTemp, Warning, TEXT("CombatComponent::Reload - CanReload() false (탄창 %d/%d, 예비탄 %d)"),
			Weapon->GetCurrentAmmo(), Weapon->GetGunData().MagazineSize, Weapon->GetReserveAmmo());
		return;
	}

	// 무기 상태를 Reloading 으로 전환 (탄약 충전은 완료 시점에)
	Weapon->Reload();
	ReloadingWeapon = Weapon;

	// 캐릭터 재장전 몽타주가 있으면 몽타주 종료로, 없으면 ReloadTime 타이머로 완료
	UAnimMontage* Montage = Weapon->GetCharacterReloadMontage();
	const float MontageLen = MontageHelper::PlayWithEndCallback(
		GetCharacterMesh(), Montage, this, &UCombatComponent::OnReloadMontageEnded);

	if (MontageLen <= 0.f)
	{
		const float ReloadTime = FMath::Max(Weapon->GetGunData().ReloadTime, 0.1f);
		UE_LOG(LogTemp, Warning, TEXT("CombatComponent::Reload - 무기 CharacterAnimSet 의 ReloadMontage 미지정, %.1fs 타이머로 진행"), ReloadTime);

		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().SetTimer(ReloadTimerHandle, this,
				&UCombatComponent::FinishReloadByTimer, ReloadTime, false);
		}
	}
}

void UCombatComponent::OnReloadMontageEnded(UAnimMontage* /*Montage*/, bool bInterrupted)
{
	CompleteReload(bInterrupted);
}

void UCombatComponent::FinishReloadByTimer()
{
	CompleteReload(false);
}

void UCombatComponent::CompleteReload(bool bInterrupted)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ReloadTimerHandle);
	}

	if (!ReloadingWeapon) return;

	if (!bInterrupted)
	{
		ReloadingWeapon->ReloadFinished();          // 정상 종료 시에만 탄약 채움
	}
	else
	{
		ReloadingWeapon->SetGunState(EGunState::Idle); // 중단되면 상태만 복구
	}
	ReloadingWeapon = nullptr;
}

void UCombatComponent::HandleWeaponFired()
{
	ARangedWeapon* Weapon = GetActiveRangedWeapon();
	if (!Weapon) return;

	MontageHelper::Play(GetCharacterMesh(), Weapon->GetCharacterFireMontage());
}
