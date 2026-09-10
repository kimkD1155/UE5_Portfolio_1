// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../Weapon/WeaponBase.h"
#include "../Ally/AllyBase.h"
#include "../Core/UpgradeType.h"
#include "ShopItemData.generated.h"


UENUM(BlueprintType)
enum class EShopItemType : uint8
{
	Rifle,
	Pistol,
	Ally,
	Upgrade
};

USTRUCT(BlueprintType)
struct FShopItemData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText ItemName;

	// 무기/아군 가격. 업그레이드는 가격이 레벨에 따라 달라지므로 UUpgradeTable 에서 계산하고 이 값은 무시된다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 Price = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EShopItemType ItemType = EShopItemType::Rifle;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<AWeaponBase> WeaponClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<AAllyBase> AllyClass;

	// ItemType == Upgrade 일 때 어떤 업그레이드인지
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EUpgradeType UpgradeType = EUpgradeType::PlayerDamage;
};