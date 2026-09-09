// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WeaponSlot.generated.h"

// 인벤토리 슬롯. 무기가 자기 선호 슬롯을 알고 있어서
// 인벤토리는 무기 타입을 분기하지 않는다 (OCP).
UENUM(BlueprintType)
enum class EWeaponSlot : uint8
{
	Primary,
	Secondary,
	Throwable
};
