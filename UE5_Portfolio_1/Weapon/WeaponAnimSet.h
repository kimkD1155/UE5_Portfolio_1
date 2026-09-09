// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "WeaponAnimSet.generated.h"

class UAnimMontage;

/**
 * 특정 무기를 사용할 때 소유 캐릭터 스켈레톤에서 재생되는 몽타주 모음.
 * 무기 액터(AWeaponBase)는 이 DataAsset 을 참조만 하고, 실제 재생은 UCombatComponent 가 한다.
 *
 * - 캐릭터가 switch(EWeaponType) 로 고르던 로직을 데이터로 분리 (OCP)
 * - 무기 변종끼리 같은 세트를 공유 가능 (예: AK / M4 가 소총 세트 공유)
 * - 멀티 캐릭터(스켈레톤별) 지원이 필요해지면 TMap<USkeleton*, UWeaponAnimSet*> 로 확장할 지점
 */
UCLASS(BlueprintType)
class UE5_PORTFOLIO_1_API UWeaponAnimSet : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Montage")
	TObjectPtr<UAnimMontage> FireMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Montage")
	TObjectPtr<UAnimMontage> ReloadMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Montage")
	TObjectPtr<UAnimMontage> EquipMontage;
};
