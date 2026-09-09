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

// 탄약(현재/예비)이 바뀔 때마다 브로드캐스트. HUD 가 매 프레임 폴링하지 않도록.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAmmoChanged, int32, CurrentAmmo, int32, ReserveAmmo);

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

    // 자동 발사 간격(초). bAutomatic 이 false 면 사용되지 않음
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    float FireRate;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    float ReloadTime;

    // true = 버튼을 누르고 있는 동안 연사, false = 클릭당 1발
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    bool bAutomatic = false;
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

    // AI 등에서 자동/반자동과 무관하게 정확히 1발만 쏘고 싶을 때
    void FireSingle();

    UPROPERTY(BlueprintAssignable, Category = "Gun")
    FOnAmmoChanged OnAmmoChanged;

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

    // 1발 발사: 조준점 해석 → 히트스캔 → 데미지. 권총/소총 공통 로직.
    // 특수 무기(샷건 펠릿 등)는 이 함수를 override 한다.
    virtual void FireOnce();

    FTimerHandle FireTimerHandle;

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

    UPROPERTY(EditAnywhere, Category = "Weapon")
    float AimSpread = 1.5f; 

    FVector ApplyAimSpread(const FVector& AimDir) const;

    void BroadcastAmmo() { OnAmmoChanged.Broadcast(CurrentAmmo, ReserveAmmo); }

public:
    void ReloadFinished();
};


