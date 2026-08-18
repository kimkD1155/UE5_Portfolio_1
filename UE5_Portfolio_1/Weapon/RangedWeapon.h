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
    int32 MagazineSize;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    int32 MaxReserveAmmo;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    float Damage;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    float Range;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    float FireRate;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    float ReloadTime;
};

UCLASS()
class UE5_PORTFOLIO_1_API ARangedWeapon : public AWeaponBase
{
	GENERATED_BODY()
	

public:
    ARangedWeapon();
    virtual void Tick(float DeltaTime) override;

    virtual void StartFire() override;
    virtual void StopFire() override;
    virtual void StartAim() override;
    virtual void StopAim() override;

    bool CanFire() const;
    bool CanReload() const;
    void Reload();

    UFUNCTION()
    void SetGunState(EGunState NewState);
	UFUNCTION(BlueprintPure)
	FGunData GetGunData() const { return GunData; }

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

    
protected:
    UPROPERTY(EditAnywhere, Category = "Recoil")
    float RecoilPitchKick = 0.5f;

    UPROPERTY(EditAnywhere, Category = "Recoil")
    float RecoilYawKickMin = -1.0f;

    UPROPERTY(EditAnywhere, Category = "Recoil")
    float RecoilYawKickMax = 1.0f;

    UPROPERTY(EditAnywhere, Category = "Recoil")
    float RecoilRecoverySpeed = 2.0f;

    UPROPERTY(EditAnywhere, Category = "Recoil")
    float MaxRecoilPitchOffset = 10.0f;

    float CurrentRecoilPitchOffset = 0.0f;
    float CurrentRecoilYawOffset = 0.0f;

    void ApplyRecoilKick();
    void TickRecoilRecovery(float DeltaTime);

public:
    void ReloadFinished();
};
