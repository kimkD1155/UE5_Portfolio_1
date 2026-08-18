// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RangedWeapon.h"
#include "Pistol.generated.h"

/**
 * 
 */
UCLASS()
class UE5_PORTFOLIO_1_API APistol : public ARangedWeapon
{
	GENERATED_BODY()
	
public:
	APistol();

	virtual void StartFire() override;
	virtual void StopFire() override;

protected:
	void Fire();

	UPROPERTY(EditDefaultsOnly, Category = "Pistol")
	float TraceRange = 5000.f;


};
