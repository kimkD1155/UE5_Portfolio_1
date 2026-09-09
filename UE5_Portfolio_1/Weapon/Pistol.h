// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RangedWeapon.h"
#include "Pistol.generated.h"

/**
 * 반자동 권총. 발사 로직은 전부 ARangedWeapon 이 처리하고
 * 이 클래스는 기본 스탯(FGunData)만 설정한다.
 */
UCLASS()
class UE5_PORTFOLIO_1_API APistol : public ARangedWeapon
{
	GENERATED_BODY()

public:
	APistol();
};
