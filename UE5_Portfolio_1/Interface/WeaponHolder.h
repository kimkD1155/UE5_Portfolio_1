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
};
