// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WeaponBase.h"
#include "RangedWeapon.generated.h"

UENUM(BlueprintType)
enum class EGunState : uint8
{
    Idle,
    Firing,
    Reloading,
    Empty
};

USTRUCT(BlueprintType)
struct FGunData
{
    GENERATED_BODY()

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    int32 MagazineSize = 30;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    int32 MaxReserveAmmo = 90;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    float Damage = 25.f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    float Range = 10000.f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    float FireRate = 0.1f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    float ReloadTime = 2.f;
};

UCLASS()
class UE5_PORTFOLIO_1_API ARangedWeapon : public AWeaponBase
{
	GENERATED_BODY()
	

public:
    ARangedWeapon();

    virtual void StartFire() override;
    virtual void StopFire() override;
    virtual void StartAim() override;
    virtual void StopAim() override;

    bool CanFire() const;
    bool CanReload() const;
    void Reload();


    UFUNCTION(BlueprintPure)
    EGunState GetGunState() const { return GunState; }

    UFUNCTION(BlueprintPure)
    int32 GetCurrentAmmo() const { return CurrentAmmo; }

    UFUNCTION(BlueprintPure)
    int32 GetReserveAmmo() const { return ReserveAmmo; }

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gun")
    FGunData GunData;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gun")
    EGunState GunState = EGunState::Idle;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gun")
    int32 CurrentAmmo;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gun")
    int32 ReserveAmmo;

    UPROPERTY(EditDefaultsOnly, Category = "Gun")
    FName MuzzleSocketName = TEXT("MuzzleFlash");

    FTimerHandle ReloadTimerHandle;

    void SetGunState(EGunState NewState);
    void OnReloadFinished();
};
