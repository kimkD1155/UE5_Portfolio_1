// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../Weapon/WeaponBase.h"
#include "../Ally/AllyBase.h"
#include "ShopItemData.generated.h"

UENUM(BlueprintType)
enum class EShopItemType : uint8
{
	Rifle,
	Pistol,
	Ally
};

USTRUCT(BlueprintType)
struct FShopItemData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText ItemName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 Price = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EShopItemType ItemType = EShopItemType::Rifle;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<AWeaponBase> WeaponClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<AAllyBase> AllyClass;
};