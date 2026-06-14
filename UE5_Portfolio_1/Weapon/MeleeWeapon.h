// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WeaponBase.h"
#include "MeleeWeapon.generated.h"

/**
 * 
 */
UCLASS()
class UE5_PORTFOLIO_1_API AMeleeWeapon : public AWeaponBase
{
	GENERATED_BODY()
	
public:
	AMeleeWeapon();

    // 캐릭터 입력에서 호출
    virtual void StartFire() override;
    virtual void StopFire()  override;
    virtual void StartAim()  override;
    virtual void StopAim()   override;

protected:
    // 칼날 히트박스
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UBoxComponent* HitBox;

    // 데미지
    UPROPERTY(EditDefaultsOnly, Category = "Melee")
    float Damage = 30.f;

    // 히트박스 활성화 / 비활성화
    void EnableHitBox();
    void DisableHitBox();

    // 한 번의 공격에 중복 히트 방지
    TArray<AActor*> HitActors;

    UFUNCTION()
    void OnHitBoxOverlap(UPrimitiveComponent* OverlappedComp,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex,
        bool bFromSweep,
        const FHitResult& SweepResult);
};
