// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "WeaponHolder.generated.h"

class AWeaponBase;

UINTERFACE(MinimalAPI)
class UWeaponHolder : public UInterface
{
	GENERATED_BODY()
};

/**
 * "무기를 들고 있는 액터" 를 나타내는 인터페이스.
 * UCombatComponent 가 소유자의 구체 타입(플레이어/아군)을 몰라도
 * 현재 제어할 무기를 가져올 수 있게 해준다. (DIP)
 */
class IWeaponHolder
{
	GENERATED_BODY()

public:
	// 현재 손에 든(제어 대상) 무기. 없으면 nullptr.
	virtual AWeaponBase* GetActiveWeapon() const = 0;

	// 이 소유자가 무기로 가하는 데미지 배율 (업그레이드 등). 기본 1.0.
	// 무기가 소유자 타입을 몰라도 "얼마나 버프됐는지" 를 물을 수 있게 한다.
	virtual float GetOutgoingDamageMultiplier() const { return 1.f; }

	// 이 소유자의 연사 속도 배율. 2.0 이면 발사 간격이 절반. 기본 1.0.
	virtual float GetFireRateMultiplier() const { return 1.f; }
};
