// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RangedWeapon.h"
#include "Rifle.generated.h"

/**
 * 
 */
UCLASS()
class UE5_PORTFOLIO_1_API ARifle : public ARangedWeapon
{
	GENERATED_BODY()
	

public:
    ARifle();
    virtual void Tick(float DeltaTime) override;

    virtual void StartFire() override;
    virtual void StopFire() override;

protected:
    virtual void BeginPlay() override;
    // 자동/반자동
    UPROPERTY(EditDefaultsOnly, Category = "Rifle")
    bool bIsAutomatic = true;

    FTimerHandle FireTimerHandle;
    

    void FireOnce();

};
