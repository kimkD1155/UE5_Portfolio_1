// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UpgradeType.generated.h"

// 수색으로 모은 코인을 상점에서 소비해 올리는 영구 업그레이드 종류.
// 레벨은 AKangPlayerState 가 보유하고, 수치는 UUpgradeTable DataAsset 이 정의한다.
UENUM(BlueprintType)
enum class EUpgradeType : uint8
{
	PlayerDamage    UMETA(DisplayName = "플레이어 공격력"),
	PlayerFireRate  UMETA(DisplayName = "플레이어 공격속도"),
	BarricadeHealth UMETA(DisplayName = "바리케이드 최대체력"),
	HealthRegen     UMETA(DisplayName = "체력 재생량"),
	AllyDamage      UMETA(DisplayName = "아군 공격력")
};
